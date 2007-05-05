/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <openssl/ssl.h>

#include "Base64Coder.h"
#include "Object.h"
#include "Runnable.h"
#include "Thread.h"
#include "System.h"
#include "TcpSocket.h"
#include "InternetAddress.h"
#include "SslSocket.h"
#include "MessageDigest.h"

using namespace std;
using namespace db::crypto;
using namespace db::net;
using namespace db::rt;
using namespace db::util;

void runBase64Test()
{
	cout << "Running Base64 Test" << endl << endl;
	
	char data[] = {'a', 'b', 'c', 'd'};
	string encoded = Base64Coder::encode(data, 0, 4);
	cout << "encoded=" << encoded << endl;
	
   char* decoded;
	int length = Base64Coder::decode(encoded, &decoded);
	
	cout << "decoded bytes=" << length << endl;
   for(int i = 0; i < length; i++)
   {
      cout << "decoded[" << i << "]=" << decoded[i] << endl;
   }
	
	string encoded2 = Base64Coder::encode(decoded, 0, 4);
	cout << "encoded again=" << encoded2 << endl;
	
   if(decoded != NULL)
   {
	   delete [] decoded;
   }
}

void runTimeTest()
{
   cout << "Running Time Test" << endl << endl;
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   cout << "Time start=" << start << endl;
   
   unsigned long long end = System::getCurrentMilliseconds();
   
   cout << "Time end=" << end << endl;
}

class TestRunnable : public virtual Object, public Runnable
{
   virtual void run()
   {
      string name = Thread::currentThread()->getName();
      cout << name << ": This is a TestRunnable thread." << endl;
      
      if(name == "Thread 1")
      {
         cout << "Thread 1 Waiting for Thread 5..." << endl;
         
         lock();
         {
            lock();
            lock();
            lock();
            wait();
         }
         unlock();
         
         cout << "Thread 1 Finished." << endl;
      }
      else if(name == "Thread 3")
      {
         cout << "Thread 3 Waiting for Thread 5..." << endl;
         
         lock();
         lock();
         lock();
         lock();
         {
            wait();
         }
         unlock();
         
         cout << "Thread 3 Finished." << endl;
      }
      else if(name == "Thread 5")
      {
         cout << "Thread 5 waking up threads..." << endl;
         
         lock();
         lock();
         lock();
         lock();
         {
            notifyAll();
         }
         unlock();
      }
   }
};

void runThreadTest()
{
   cout << "Running Thread Test" << endl << endl;
   
   TestRunnable r1;
   Thread t1(&r1, "Thread 1");
   
   //TestRunnable r2;
   Thread t2(&r1, "Thread 2");
   
   //TestRunnable r3;
   Thread t3(&r1, "Thread 3");
   
   //TestRunnable r4;
   Thread t4(&r1, "Thread 4");
   
   //TestRunnable r5;
   Thread t5(&r1, "Thread 5");
   
   t1.start();
   t2.start();
   t3.start();
   t4.start();
   t5.start();
   
   t1.join();
   t2.join();
   t3.join();
   t4.join();
   t5.join();
}

void runLinuxSocketTest()
{
   cout << "Running Socket Test" << endl << endl;
   
   // create tcp socket
   TcpSocket socket;
   
   // create address
   //InternetAddress address("127.0.0.1", 80);
   InternetAddress address("www.google.com", 80);
   cout << address.getAddress() << endl;
   
   // connect
   socket.connect(&address);
   
   char request[] =
      "GET / HTTP/1.0\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
   socket.send(request, 0, sizeof(request));
   
   // set receive timeout (10 seconds = 10000 milliseconds)
   socket.setReceiveTimeout(10000);
   
   char response[2048];
   int numBytes = 0;
   string str = "";
   
   cout << endl << "DOING A PEEK!" << endl;
   
   numBytes = socket.getInputStream()->peek(response, 0, 2048);
   if(numBytes != -1)
   {
      cout << "Peeked " << numBytes << " bytes." << endl;
      string peek = "";
      peek.append(response, numBytes);
      cout << "Peek bytes=" << peek << endl;
   }
   
   cout << endl << "DOING ACTUAL READ NOW!" << endl;
   
   while((numBytes = socket.getInputStream()->read(response, 0, 2048)) != -1)
   {
      cout << "numBytes received: " << numBytes << endl;
      str.append(response, numBytes);
   }
   
//   char response[2048];
//   int numBytes = 0;
//   string str = "";
//   while((numBytes = socket.receive(response, 0, 2048)) != -1)
//   {
//      cout << "numBytes received: " << numBytes << endl;
//      str.append(response, numBytes);
//   }
   
   // close
   socket.close();
   
   cout << "Socket connection closed." << endl;
   cout << "Response:" << endl << str << endl;
   
   cout << endl << "Socket test complete." << endl;
}

void runWindowsSocketTest()
{
   // initialize winsock
#ifdef WIN32
   WSADATA wsaData;
   if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
   {
      cout << "ERROR! Could not initialize winsock!" << endl;
   }
#endif
   
   // run linux socket test
   runLinuxSocketTest();
   
   // cleanup winsock
#ifdef WIN32
   WSACleanup();
#endif
}

void runLinuxSslSocketTest()
{
   cout << "Running SSL Socket Test" << endl << endl;
   
   // openssl initialization code
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();
   
   // FIXME:
   // seed PRNG
   
   // create an SSL context
   SslContext context;
   
   // create tcp socket
   TcpSocket socket;
   
   // create address
   InternetAddress address("127.0.0.1", 443);
   //InternetAddress address("127.0.0.1", 19020);
   //InternetAddress address("www.google.com", 80);
   cout << address.getAddress() << endl;
   
   // connect
   socket.connect(&address);
   
   // create an SSL socket
   SslSocket sslSocket(&context, &socket, true, false);
   
   // set receive timeout (10 seconds = 10000 milliseconds)
   sslSocket.setReceiveTimeout(10000);
   
   // perform handshake (automatically happens, this call isn't necessary)
   //sslSocket.performHandshake();
   
   char request[] =
      "GET / HTTP/1.0\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
   sslSocket.send(request, 0, sizeof(request));
   
   char response[2048];
   int numBytes = 0;
   string str = "";
   
   cout << endl << "DOING A PEEK!" << endl;
   
   numBytes = sslSocket.getInputStream()->peek(response, 0, 2048);
   if(numBytes != -1)
   {
      cout << "Peeked " << numBytes << " bytes." << endl;
      string peek = "";
      peek.append(response, numBytes);
      cout << "Peek bytes=" << peek << endl;
   }
   
   cout << endl << "DOING ACTUAL READ NOW!" << endl;
   
   while((numBytes = sslSocket.getInputStream()->read(response, 0, 2048)) != -1)
   {
      cout << "numBytes received: " << numBytes << endl;
      str.append(response, numBytes);
   }
   
   // close
   sslSocket.close();
   
   cout << "SSL Socket connection closed." << endl;
   cout << "Response:" << endl << str << endl;
   
   cout << endl << "SSL Socket test complete." << endl;
   
   // clean up SSL
   EVP_cleanup();
}

void runWindowsSslSocketTest()
{
// initialize winsock
#ifdef WIN32
   WSADATA wsaData;
   if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
   {
      cout << "ERROR! Could not initialize winsock!" << endl;
   }
#endif
   
   // run linux ssl socket test
   runLinuxSslSocketTest();
   
// cleanup winsock
#ifdef WIN32
   WSACleanup();
#endif   
}

void runMessageDigestTest()
{
   cout << "Running MessageDigest Test" << endl << endl;
   
   // correct values
   string correctMd5 = "78eebfd9d42958e3f31244f116ab7bbe";
   string correctSha1 = "5f24f4d6499fd2d44df6c6e94be8b14a796c071d";   
   
   MessageDigest testMd5("MD5");
   testMd5.updateMessage("THIS IS A MESSAGE");
   string digestMd5 = testMd5.getDigest();
   
   cout << "MD5 Digest=" << digestMd5 << endl;
   if(digestMd5 == correctMd5)
   {
      cout << "MD5 is correct!" << endl;
   }
   else
   {
      cout << "MD5 is incorrect!" << endl;
   }
   
   MessageDigest testSha1("SHA1");
   testSha1.updateMessage("THIS IS A MESSAGE");
   string digestSha1 = testSha1.getDigest();
   
   cout << "SHA-1 Digest=" << digestSha1 << endl;
   if(digestSha1 == correctSha1)
   {
      cout << "SHA-1 is correct!" << endl;
   }
   else
   {
      cout << "SHA-1 is incorrect!" << endl;
   }
   
   cout << "MessageDigest test complete." << endl << endl;
}

int main()
{
   cout << "Tests starting..." << endl << endl;
   
   try
   {
      //runBase64Test();
      //runTimeTest();
      //runThreadTest();
      //runWindowsSocketTest();
      //runLinuxSocketTest();
      //runWindowsSslSocketTest();
      //runLinuxSslSocketTest();
      runMessageDigestTest();
   }
   catch(SocketException& e)
   {
      cout << "SocketException caught!" << endl;
      cout << "message: " << e.getMessage() << endl;
      cout << "code: " << e.getCode() << endl;
   }
   catch(UnsupportedAlgorithmException& e)
   {
      cout << "UnsupportedAlgorithmException caught!" << endl;
      cout << "message: " << e.getMessage() << endl;
   }
   catch(...)
   {
      cout << "Exception caught!" << endl;
   }
   
   cout << endl << "Tests finished." << endl;
}
