/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include "Base64Coder.h"
#include "Object.h"
#include "Runnable.h"
#include "Thread.h"
#include "System.h"
#include "TcpSocket.h"
#include "UdpSocket.h"
#include "Internet6Address.h"
#include "SslSocket.h"
#include "MessageDigest.h"
#include "Crc16.h"
#include "AsymmetricKeyFactory.h"
#include "FileInputStream.h"
#include "DigitalEnvelope.h"
#include "DefaultBlockCipher.h"
#include "Convert.h"
#include "Url.h"
#include "http/HttpHeader.h"

using namespace std;
using namespace db::crypto;
using namespace db::io;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;
using namespace db::util;

void runBase64Test()
{
	cout << "Running Base64 Test" << endl << endl;
	
	char data[] = {'a', 'b', 'c', 'd', 'e'};
	string encoded = Base64Coder::encode(data + 1, 4);
	cout << "encoded=" << encoded << endl;
	
   char* decoded;
	unsigned int length;
   Base64Coder::decode(encoded, &decoded, length);
	
	cout << "decoded bytes=" << length << endl;
   for(unsigned int i = 0; i < length; i++)
   {
      cout << "decoded[" << i << "]=" << decoded[i] << endl;
   }
	
	string encoded2 = Base64Coder::encode(decoded, 4);
	cout << "encoded again=" << encoded2 << endl;
	
   if(decoded != NULL)
   {
	   delete [] decoded;
   }
   
   cout << endl << "Base64 Test complete." << endl;
}

void runTimeTest()
{
   cout << "Running Time Test" << endl << endl;
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   cout << "Time start=" << start << endl;
   
   unsigned long long end = System::getCurrentMilliseconds();
   
   cout << "Time end=" << end << endl;
   
   cout << endl << "Time Test complete." << endl;
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
   
   cout << endl << "Thread Test complete." << endl;
}

void runLinuxAddressResolveTest()
{
   cout << "Running Address Resolve Test" << endl << endl;
   
   // create IPv4 address
   InternetAddress ip4;
   
   cout << "Testing IPv4..." << endl << endl;
   
   ip4.setHost("www.bitmunk.com");
   cout << "www.bitmunk.com = " << ip4.getAddress() << endl;
   
   ip4.setHost("www.google.com");
   cout << "www.google.com = " << ip4.getAddress() << endl;
   
   ip4.setHost("www.yahoo.com");
   cout << "www.yahoo.com = " << ip4.getAddress() << endl;
   
   ip4.setHost("www.microsoft.com");
   cout << "www.microsoft.com = " << ip4.getAddress() << endl;
   
   cout << endl;
   
   ip4.setAddress("192.168.0.1");
   cout << ip4.getAddress() << " = " << ip4.getHost() << endl;
   
   ip4.setAddress("192.168.0.8");
   cout << ip4.getAddress() << " = " << ip4.getHost() << endl;
   
   ip4.setAddress("216.239.51.99");
   cout << ip4.getAddress() << " = " << ip4.getHost() << endl;
   
   // create IPv6 address
   Internet6Address ip6;
   
   cout << endl << "Testing IPv6..." << endl << endl;
   
   //ip6.setHost("ip6-localhost");
   //cout << "ip6-localhost = " << ip6.getAddress() << endl;
   
   //ip6.setHost("yuna.digitalbazaar.com");
   //cout << "yuna.digitalbazaar.com = " << ip6.getAddress() << endl;
   
   /*
   ip6.setHost("www.google.com");
   cout << "www.google.com = " << ip6.getAddress() << endl;
   
   ip6.setHost("www.yahoo.com");
   cout << "www.yahoo.com = " << ip6.getAddress() << endl;
   
   ip6.setHost("www.microsoft.com");
   cout << "www.microsoft.com = " << ip6.getAddress() << endl;
   */
   
   cout << endl;
   
   ip6.setAddress("fc00:840:db:bb:d::8");
   cout << ip6.getAddress() << " = " << ip6.getHost() << endl;
   
   cout << endl << "Address Resolve Test complete." << endl << endl;
}

void runWindowsAddressResolveTest()
{
   // initialize winsock
#ifdef WIN32
   WSADATA wsaData;
   if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
   {
      cout << "ERROR! Could not initialize winsock!" << endl;
   }
#endif
   
   // run linux address resolve test
   runLinuxAddressResolveTest();
   
   // cleanup winsock
#ifdef WIN32
   WSACleanup();
#endif
}

void runLinuxSocketTest()
{
   cout << "Running Socket Test" << endl << endl;
   
   // create tcp socket
   TcpSocket socket;
   
   // create address
   //InternetAddress address("127.0.0.1", 80);
   InternetAddress address("www.google.com", 80);
   cout << "Connecting to: " << address.getAddress() << endl;
   
   // connect
   socket.connect(&address);
   
   char request[] =
      "GET / HTTP/1.0\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
   socket.send(request, sizeof(request));
   
   // set receive timeout (10 seconds = 10000 milliseconds)
   socket.setReceiveTimeout(10000);
   
   char response[2048];
   int numBytes = 0;
   string str = "";
   
   cout << endl << "DOING A PEEK!" << endl;
   
   numBytes = socket.getInputStream()->peek(response, 2048);
   if(numBytes != -1)
   {
      cout << "Peeked " << numBytes << " bytes." << endl;
      string peek = "";
      peek.append(response, numBytes);
      cout << "Peek bytes=" << peek << endl;
   }
   
   cout << endl << "DOING ACTUAL READ NOW!" << endl;
   
   while((numBytes = socket.getInputStream()->read(response, 2048)) != -1)
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
   //cout << "Response:" << endl << str << endl;
   
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
   
   // create tcp socket
   TcpSocket socket;
   
   // create address
   InternetAddress address("127.0.0.1", 443);
   //InternetAddress address("127.0.0.1", 19020);
   //InternetAddress address("www.google.com", 80);
   cout << address.getAddress() << endl;
   
   // connect
   socket.connect(&address);
   
   // create an SSL context
   SslContext context;
   
   // create an SSL socket
   SslSocket sslSocket(&context, &socket, true, false);
   
   // set receive timeout (10 seconds = 10000 milliseconds)
   sslSocket.setReceiveTimeout(10000);
   
   // perform handshake (automatically happens, this call isn't necessary)
   //sslSocket.performHandshake();
   
   char request[] =
      "GET / HTTP/1.0\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
   sslSocket.send(request, sizeof(request));
   
   char response[2048];
   int numBytes = 0;
   string str = "";
   
   cout << endl << "DOING A PEEK!" << endl;
   
   numBytes = sslSocket.getInputStream()->peek(response, 2048);
   if(numBytes != -1)
   {
      cout << "Peeked " << numBytes << " bytes." << endl;
      string peek = "";
      peek.append(response, numBytes);
      cout << "Peek bytes=" << peek << endl;
   }
   
   cout << endl << "DOING ACTUAL READ NOW!" << endl;
   
   while((numBytes = sslSocket.getInputStream()->read(response, 2048)) != -1)
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

void runLinuxServerSocketTest()
{
   cout << "Running Server Socket Test" << endl << endl;
   
   // create tcp socket
   TcpSocket socket;
   
   // bind and listen
   InternetAddress address("127.0.0.1", 1024);
   socket.bind(&address);
   socket.listen();
   
   // accept a connection
   Socket* worker = socket.accept(10);
   
   // set receive timeout (10 seconds = 10000 milliseconds)
   worker->setReceiveTimeout(10000);
   
   char request[2048];
   int numBytes = 0;
   string str = "";
   
   cout << endl << "DOING A PEEK!" << endl;
   
   numBytes = worker->getInputStream()->peek(request, 2048);
   if(numBytes != -1)
   {
      cout << "Peeked " << numBytes << " bytes." << endl;
      string peek = "";
      peek.append(request, numBytes);
      cout << "Peek bytes=" << peek << endl;
   }
   
   cout << endl << "DOING ACTUAL READ NOW!" << endl;
   
   while((numBytes = worker->getInputStream()->read(request, 2048)) != -1)
   {
      cout << "numBytes received: " << numBytes << endl;
      str.append(request, numBytes);
   }
   
   // close worker socket
   worker->close();
   delete worker;
   
   // close server socket
   socket.close();
   
   cout << "Server Socket connection closed." << endl;
   cout << "Request:" << endl << str << endl;
   
   cout << endl << "Server Socket test complete." << endl;
}

void runWindowsServerSocketTest()
{
// initialize winsock
#ifdef WIN32
   WSADATA wsaData;
   if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
   {
      cout << "ERROR! Could not initialize winsock!" << endl;
   }
#endif
   
   // run linux server socket test
   runLinuxServerSocketTest();
   
// cleanup winsock
#ifdef WIN32
   WSACleanup();
#endif   
}

void runLinuxSslServerSocketTest()
{
   cout << "Running SSL Server Socket Test" << endl << endl;
   
   // openssl initialization code
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();   
   
   // create tcp socket
   TcpSocket socket;
   
   // bind and listen
   InternetAddress address("127.0.0.1", 1024);
   socket.bind(&address);
   socket.listen();
   
   // accept a connection
   TcpSocket* worker = (TcpSocket*)socket.accept(10);
   
   // create an SSL context
   SslContext context;
   
   // create an SSL socket
   SslSocket sslSocket(&context, worker, false, false);
   
   // set receive timeout (10 seconds = 10000 milliseconds)
   sslSocket.setReceiveTimeout(10000);
   
   char request[2048];
   int numBytes = 0;
   string str = "";
   
   cout << endl << "DOING A PEEK!" << endl;
   
   numBytes = worker->getInputStream()->peek(request, 2048);
   if(numBytes != -1)
   {
      cout << "Peeked " << numBytes << " bytes." << endl;
      string peek = "";
      peek.append(request, numBytes);
      cout << "Peek bytes=" << peek << endl;
   }
   
   cout << endl << "DOING ACTUAL READ NOW!" << endl;
   
   while((numBytes = sslSocket.getInputStream()->read(request, 2048)) != -1)
   {
      cout << "numBytes received: " << numBytes << endl;
      str.append(request, numBytes);
   }
   
   // close ssl socket socket
   sslSocket.close();
   delete worker;
   
   // close server socket
   socket.close();
   
   cout << "SSL Server Socket connection closed." << endl;
   cout << "Request:" << endl << str << endl;
   
   cout << endl << "SSL Server Socket test complete." << endl;
   
   // clean up SSL
   EVP_cleanup();
}

void runWindowsSslServerSocketTest()
{
// initialize winsock
#ifdef WIN32
   WSADATA wsaData;
   if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
   {
      cout << "ERROR! Could not initialize winsock!" << endl;
   }
#endif
   
   // run linux SSL server socket test
   runLinuxSslServerSocketTest();
   
// cleanup winsock
#ifdef WIN32
   WSACleanup();
#endif   
}

void runLinuxTcpClientServerTest()
{
   cout << "Running TCP Client/Server Test" << endl << endl;
   
   InternetAddress* address;
   InternetAddress ia("127.0.0.1", 9999);
   //Internet6Address ia("::0", 9999);
   address = &ia;
   
   // create tcp server and client sockets
   TcpSocket server;
   TcpSocket client;
   
   // set receive timeouts to 10 seconds
   server.setReceiveTimeout(10000);
   client.setReceiveTimeout(10000);
   
   // bind and listen with server
   server.bind(address);
   server.listen();
   
   cout << "Server listening at host: " << address->getHost() << endl;
   cout << "Server listening at address: " << address->getAddress() << endl;
   cout << "Server listening on port: " << address->getPort() << endl;
   
   // connect with client
   client.connect(address);
   
   cout << "Client connected." << endl;
   
   // accept a connection
   TcpSocket* worker = (TcpSocket*)server.accept(10);
   
   cout << "Client connection accepted by Server." << endl;
   
   // send some data with client
   string clientData = "Hello there, Server.";
   client.getOutputStream()->write(clientData.c_str(), clientData.length());
   
   cout << "Client sent: " << clientData << endl;
   
   // receive the client data
   char read[2048];
   int numBytes = worker->getInputStream()->read(read, 2048);
   string serverReceived(read, numBytes);
   
   cout << "Server received: " << serverReceived << endl;
   
   // send some data with server
   string serverData = "G'day, Client.";
   worker->getOutputStream()->write(serverData.c_str(), serverData.length());
   
   cout << "Server sent: " << serverData << endl;
   
   // receive the server data
   numBytes = client.getInputStream()->read(read, 2048);
   string clientReceived(read, numBytes);
   
   cout << "Client received: " << clientReceived << endl;
   
   // close sockets
   client.close();
   server.close();
   
   // delete worker
   if(worker != NULL)
   {
      worker->close();
      delete worker;
   }
   
   cout << "Sockets closed." << endl;
   
   cout << endl << "TCP Client/Server test complete." << endl;
}

void runWindowsTcpClientServerTest(InternetAddress* address)
{
// initialize winsock
#ifdef WIN32
   WSADATA wsaData;
   if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
   {
      cout << "ERROR! Could not initialize winsock!" << endl;
   }
#endif
   
   // run linux client/server test
   runLinuxTcpClientServerTest();
   
// cleanup winsock
#ifdef WIN32
   WSACleanup();
#endif   
}

void runLinuxUdpClientServerTest()
{
   cout << "Running UDP Client/Server Test" << endl << endl;
   
   InternetAddress* sa;
   InternetAddress* ca;
   InternetAddress serverAddress("127.0.0.1", 9999);
   InternetAddress clientAddress("127.0.0.1", 0);
   //Internet6Address serverAddress("::0", 9999);
   //Internet6Address clientAddress("::1", 0);
   sa = &serverAddress;
   ca = &clientAddress;
   
   // create udp server and client sockets
   UdpSocket server;
   UdpSocket client;
   
   // set receive timeouts to 10 seconds
   server.setReceiveTimeout(10000);
   client.setReceiveTimeout(10000);
   
   // bind with server
   server.bind(sa);
   
   cout << "Server bound at host: " << sa->getHost() << endl;
   cout << "Server bound at address: " << sa->getAddress() << endl;
   cout << "Server bound on port: " << sa->getPort() << endl;
   
   // bind with client
   client.bind(ca);
   client.getLocalAddress(ca);
   
   cout << "Client bound at host: " << ca->getHost() << endl;
   cout << "Client bound at address: " << ca->getAddress() << endl;
   cout << "Client bound on port: " << ca->getPort() << endl;
   
   // send some data with client
   string clientData = "Hello there, Server.";
   client.sendDatagram(clientData.c_str(), clientData.length(), sa);
   
   cout << "Client sent: " << clientData << endl;
   
   // receive the client data
   char read[2048];
   int numBytes = server.receiveDatagram(read, 2048, ca);
   string serverReceived(read, numBytes);
   
   cout << "Server received: " << serverReceived << endl;
   cout << "Data from: " << ca->getAddress();
   cout << ":" << ca->getPort() << endl;
   
   // send some data with server
   string serverData = "G'day, Client.";
   server.sendDatagram(serverData.c_str(), serverData.length(), ca);
   
   cout << "Server sent: " << serverData << endl;
   
   // receive the server data
   numBytes = client.receiveDatagram(read, 2048, sa);
   string clientReceived(read, numBytes);
   
   cout << "Client received: " << clientReceived << endl;
   cout << "Data from: " << sa->getAddress();
   cout << ":" << sa->getPort() << endl;
   
   // close sockets
   client.close();
   server.close();
   
   cout << "Sockets closed." << endl;
   
   cout << endl << "UDP Client/Server test complete." << endl;
}

void runWindowsUdpClientServerTest()
{
// initialize winsock
#ifdef WIN32
   WSADATA wsaData;
   if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
   {
      cout << "ERROR! Could not initialize winsock!" << endl;
   }
#endif
   
   // run linux client/server test
   runLinuxUdpClientServerTest();
   
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
   testMd5.update("THIS ");
   testMd5.update("IS A");
   testMd5.update(" MESSAGE");
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
   testSha1.update("THIS IS A MESSAGE");
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
   
   cout << endl << "MessageDigest test complete." << endl;
}

void runCrcTest()
{
   cout << "Running CRC Test" << endl << endl;
   
   unsigned int correctValue = 6013;
   
   Crc16 crc16;
   char b[] = {10, 20, 30, 40, 50, 60, 70, 80};
//   crc16.update(10);
//   crc16.update(20);
//   crc16.update(30);
//   crc16.update(40);
//   crc16.update(50);
//   crc16.update(60);
//   crc16.update(70);
//   crc16.update(80);
   crc16.update(b, 8);
   
   cout << "CRC-16=" << crc16.getChecksum() << endl;
   if(crc16.getChecksum() == correctValue)
   {
      cout << "CRC-16 is correct!" << endl;
   }
   else
   {
      cout << "CRC-16 is incorrect!" << endl;
   }
   
   cout << endl << "CRC test complete." << endl;
}

void runAsymmetricKeyLoadingTest()
{
   cout << "Running Asymmetric Key Loading Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   try
   {
      // read in PEM private key
      File file1("/work/src/dbcpp/dbcore/trunk/Debug/private.pem");
      FileInputStream fis1(&file1);
      
      string privatePem = "";
      
      char b[2048];
      int numBytes;
      while((numBytes = fis1.read(b, 2048)) != -1)
      {
         privatePem.append(b, numBytes);
      }
      
      // close stream
      fis1.close();
      
      cout << "Private Key PEM=" << endl << privatePem << endl;
      
      // read in PEM public key
      File file2("/work/src/dbcpp/dbcore/trunk/Debug/public.pem");
      FileInputStream fis2(&file2);
      
      string publicPem = "";
      
      while((numBytes = fis2.read(b, 2048)) != -1)
      {
         publicPem.append(b, numBytes);
      }
      
      // close stream
      fis2.close();
      
      cout << "Public Key PEM=" << endl << publicPem << endl;
           
      // get an asymmetric key factory
      AsymmetricKeyFactory factory;
      
      // load the private key
      PrivateKey* privateKey = factory.loadPrivateKeyFromPem(
         privatePem, "password");
      
      cout << "Private Key Algorithm=" << privateKey->getAlgorithm() << endl;
      
      // load the public key
      PublicKey* publicKey = factory.loadPublicKeyFromPem(publicPem);
      
      cout << "Public Key Algorithm=" << publicKey->getAlgorithm() << endl;
      
      // sign some data
      char data[] = {1,2,3,4,5,6,7,8};
      DigitalSignature* ds1 = privateKey->createSignature();
      ds1->update(data, 8);
      
      // get the signature
      char sig[ds1->getValueLength()];
      unsigned int length;
      ds1->getValue(sig, length);
      delete ds1;
      
      // verify the signature
      DigitalSignature* ds2 = publicKey->createSignature();
      ds2->update(data, 8);
      bool verified = ds2->verify(sig, length);
      delete ds2;
      
      if(verified)
      {
         cout << "Digital Signature Verified!" << endl;
      }
      else
      {
         cout << "Digital Signature NOT VERIFIED!" << endl;
      }
      
      string outPrivatePem =
         factory.writePrivateKeyToPem(privateKey, "password");
      string outPublicPem =
         factory.writePublicKeyToPem(publicKey);
      
      cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
      cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
      
      // delete the private key
      delete privateKey;
      
      // delete the public key
      delete publicKey;
   }
   catch(IOException &e)
   {
      cout << "IOException caught!" << endl;
      cout << e.getMessage() << endl;
      cout << e.getCode() << endl;
   }
   
   cout << endl << "Asymmetric Key Loading test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runDsaAsymmetricKeyCreationTest()
{
   cout << "Running DSA Asymmetric Key Creation Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   try
   {
      // get an asymmetric key factory
      AsymmetricKeyFactory factory;
      
      // create a new key pair
      PrivateKey* privateKey;
      PublicKey* publicKey;
      factory.createKeyPair("DSA", &privateKey, &publicKey);
      
      if(privateKey != NULL)
      {
         cout << "DSA Private Key created!" << endl;
      }
      else
      {
         cout << "DSA Private Key creation FAILED!" << endl;
      }
      
      if(publicKey != NULL)
      {
         cout << "DSA Public Key created!" << endl;
      }
      else
      {
         cout << "DSA Public Key creation FAILED!" << endl;
      }
      
      if(privateKey != NULL && publicKey != NULL)
      {
         cout << "Private Key Algorithm=" << privateKey->getAlgorithm() << endl;
         cout << "Public Key Algorithm=" << publicKey->getAlgorithm() << endl;
         
         // sign some data
         char data[] = {1,2,3,4,5,6,7,8};
         DigitalSignature* ds1 = privateKey->createSignature();
         ds1->update(data, 8);
         
         // get the signature
         char sig[ds1->getValueLength()];
         unsigned int length;
         ds1->getValue(sig, length);
         delete ds1;
         
         // verify the signature
         DigitalSignature* ds2 = publicKey->createSignature();
         ds2->update(data, 8);
         bool verified = ds2->verify(sig, length);
         delete ds2;
         
         if(verified)
         {
            cout << "Digital Signature Verified!" << endl;
         }
         else
         {
            cout << "Digital Signature NOT VERIFIED!" << endl;
         }
         
         string outPrivatePem =
            factory.writePrivateKeyToPem(privateKey, "password");
         string outPublicPem =
            factory.writePublicKeyToPem(publicKey);
         
         cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
         cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
      }
      
      // cleanup private key
      if(privateKey != NULL)
      {
         delete privateKey;
      }
      
      // cleanup public key
      if(publicKey != NULL)
      {
         delete publicKey;
      }
   }
   catch(Exception &e)
   {
      cout << "Exception caught!" << endl;
      cout << e.getMessage() << endl;
      cout << e.getCode() << endl;
   }
   
   cout << endl << "DSA Asymmetric Key Creation test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runRsaAsymmetricKeyCreationTest()
{
   cout << "Running RSA Asymmetric Key Creation Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   try
   {
      // get an asymmetric key factory
      AsymmetricKeyFactory factory;
      
      // create a new key pair
      PrivateKey* privateKey;
      PublicKey* publicKey;
      factory.createKeyPair("RSA", &privateKey, &publicKey);
      
      if(privateKey != NULL)
      {
         cout << "RSA Private Key created!" << endl;
      }
      else
      {
         cout << "RSA Private Key creation FAILED!" << endl;
      }
      
      if(publicKey != NULL)
      {
         cout << "RSA Public Key created!" << endl;
      }
      else
      {
         cout << "RSA Public Key creation FAILED!" << endl;
      }
      
      if(privateKey != NULL && publicKey != NULL)
      {
         cout << "Private Key Algorithm=" << privateKey->getAlgorithm() << endl;
         cout << "Public Key Algorithm=" << publicKey->getAlgorithm() << endl;
         
         // sign some data
         char data[] = {1,2,3,4,5,6,7,8};
         DigitalSignature* ds1 = privateKey->createSignature();
         ds1->update(data, 8);
         
         // get the signature
         char sig[ds1->getValueLength()];
         unsigned int length;
         ds1->getValue(sig, length);
         delete ds1;
         
         // verify the signature
         DigitalSignature* ds2 = publicKey->createSignature();
         ds2->update(data, 8);
         bool verified = ds2->verify(sig, length);
         delete ds2;
         
         if(verified)
         {
            cout << "Digital Signature Verified!" << endl;
         }
         else
         {
            cout << "Digital Signature NOT VERIFIED!" << endl;
         }
         
         string outPrivatePem =
            factory.writePrivateKeyToPem(privateKey, "password");
         string outPublicPem =
            factory.writePublicKeyToPem(publicKey);
         
         cout << "Written Private Key PEM=" << endl << outPrivatePem << endl;
         cout << "Written Public Key PEM=" << endl << outPublicPem << endl;
      }
      
      // cleanup private key
      if(privateKey != NULL)
      {
         delete privateKey;
      }
      
      // cleanup public key
      if(publicKey != NULL)
      {
         delete publicKey;
      }
   }
   catch(Exception &e)
   {
      cout << "Exception caught!" << endl;
      cout << e.getMessage() << endl;
      cout << e.getCode() << endl;
   }
   
   cout << endl << "RSA Asymmetric Key Creation test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runEnvelopeTest(const std::string& algorithm)
{
   cout << "Running " << algorithm << " Envelope Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   try
   {
      // get an asymmetric key factory
      AsymmetricKeyFactory factory;
      
      // create a new key pair
      PrivateKey* privateKey;
      PublicKey* publicKey;
      factory.createKeyPair("RSA", &privateKey, &publicKey);
      
      if(privateKey != NULL && publicKey != NULL)
      {
         try
         {
            // create a secret message
            char message[] =
               "This is a confidential message. For British Eyes Only.";
            int length = strlen(message);
            
            string display1 = "";
            display1.append(message, length);
            cout << "Sending message '" << display1 << "'" << endl;
            cout << "Message Length=" << length << endl;
            
            // create an outgoing envelope
            SymmetricKey* secretKey;
            DigitalEnvelope* outEnv = publicKey->createEnvelope(
               "AES256", &secretKey);
            cout << "Created outgoing envelope..." << endl;
            
            // update the envelope
            char output[2048];
            int outLength;
            int totalOut = 0;
            outEnv->update(message, length, output, outLength);
            cout << "Updated outgoing envelope..." << endl;
            totalOut += outLength;
            
            // finish the envelope
            cout << "Output Length=" << outLength << endl;
            outEnv->finish(output + outLength, outLength);
            cout << "Finished sealing outgoing envelope..." << endl;
            totalOut += outLength;
            
            cout << "Total Output Length=" << totalOut << endl;
            
            // create an incoming envelope
            DigitalEnvelope* inEnv = privateKey->createEnvelope(secretKey);
            cout << "Created incoming envelope..." << endl;
            
            // update the envelope
            char input[2048];
            int inLength;
            int totalIn = 0;
            inEnv->update(output, totalOut, input, inLength);
            cout << "Updated incoming envelope..." << endl;
            totalIn += inLength;
            
            // finish the envelope
            cout << "Input Length=" << inLength << endl;
            inEnv->finish(input + inLength, inLength);
            cout << "Finished opening incoming envelope..." << endl;
            totalIn += inLength;
            
            cout << "Total Input Length=" << totalIn << endl;
            
            // create a string to display the received message
            string display2 = "";
            display2.append(input, totalIn);
            
            cout << "Received message '" << display2 << "'" << endl;
            
            // delete envelopes and key
            delete secretKey;
            delete outEnv;
            delete inEnv;
         }
         catch(IOException &e)
         {
            cout << "IOException caught in envelope method!" << endl;
            cout << "message: " << e.getMessage() << endl;
            cout << "code: " << e.getCode() << endl;
         }
      }
      
      // cleanup private key
      if(privateKey != NULL)
      {
         delete privateKey;
      }
      
      // cleanup public key
      if(publicKey != NULL)
      {
         delete publicKey;
      }
   }
   catch(Exception &e)
   {
      cout << "Exception caught!" << endl;
      cout << e.getMessage() << endl;
      cout << e.getCode() << endl;
   }
   
   cout << endl << algorithm << " Envelope test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runCipherTest(const string& algorithm)
{
   cout << "Running " << algorithm << " Cipher Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
   try
   {
      // create a secret message
      char message[] = "I'll never teelllll!";
      int length = strlen(message);
      
      string display1 = "";
      display1.append(message, length);
      cout << "Encrypting message '" << display1 << "'" << endl;
      cout << "Message Length=" << length << endl;
      
      // get a default block cipher
      DefaultBlockCipher cipher;
      
      cout << "Starting encryption..." << endl;
      
      // generate a new key for the encryption
      SymmetricKey* key = NULL;
      cipher.startEncrypting(algorithm, &key);
      
      if(key != NULL)
      {
         // update encryption
         char output[2048];
         int outLength;
         int totalOut = 0;
         cipher.update(message, length, output, outLength);
         cout << "Updated encryption..." << endl;
         totalOut += outLength;
         
         // finish the envelope
         cout << "Output Length=" << outLength << endl;
         cipher.finish(output + outLength, outLength);
         cout << "Finished encryption..." << endl;
         totalOut += outLength;
         
         cout << "Total Output Length=" << totalOut << endl;
         
         cout << "Starting decryption..." << endl;
         cipher.startDecrypting(key);
         
         // update the decryption
         char input[2048];
         int inLength;
         int totalIn = 0;
         cipher.update(output, totalOut, input, inLength);
         cout << "Updated decryption..." << endl;
         totalIn += inLength;
         
         // finish the decryption
         cout << "Input Length=" << inLength << endl;
         cipher.finish(input + inLength, inLength);
         cout << "Finished decrypting..." << endl;
         totalIn += inLength;
         
         cout << "Total Input Length=" << totalIn << endl;
         
         // create a string to display the received message
         string display2 = "";
         display2.append(input, totalIn);
         
         cout << "Decrypted message '" << display2 << "'" << endl;
      }
      
      // cleanup key
      if(key != NULL)
      {
         delete key;
      }
   }
   catch(Exception &e)
   {
      cout << "Exception caught!" << endl;
      cout << e.getMessage() << endl;
      cout << e.getCode() << endl;
   }
   
   cout << endl << algorithm << " Cipher test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runConvertTest()
{
   cout << "Starting Convert test." << endl << endl;
   
   // convert to hex
   char data[] = "abcdefghiABCDEFGZXYW0123987{;}*%6,./.12`~";
   string original(data, strlen(data));
   
   cout << "test data=" << original << endl;
   
   string lowerHex = Convert::bytesToHex(data, strlen(data));
   string upperHex = Convert::bytesToHex(data, strlen(data));
   
   cout << "lower-case hex=" << lowerHex << endl;
   cout << "lower-case hex length=" << lowerHex.length() << endl;
   cout << "upper-case hex=" << upperHex << endl;
   cout << "upper-case hex length=" << upperHex.length() << endl;
   
   char decoded1[lowerHex.length() / 2];
   char decoded2[upperHex.length() / 2];
   
   unsigned int length1;
   unsigned int length2;
   Convert::hexToBytes(lowerHex.c_str(), lowerHex.length(), decoded1, length1);
   Convert::hexToBytes(upperHex.c_str(), upperHex.length(), decoded2, length2);
   
   string ascii1(decoded1, length1);
   
   string ascii2(decoded2, length2);
   
   cout << "lower-case hex to ascii=" << ascii1 << endl;
   cout << "lower-case hex length=" << length1 << endl;
   cout << "upper-case hex to ascii=" << ascii2 << endl;
   cout << "upper-case hex length=" << length2 << endl;
   
   if(ascii1 == ascii2 && ascii1 == original)
   {
      cout << "Test successful!" << endl;
   }
   else
   {
      cout << "Test FAILED! Strings do not match!" << endl;
   }
   
   cout << endl << "Convert test complete." << endl;
}

void runUrlTest()
{
   cout << "Starting Url test." << endl << endl;
   
   string str = "billy bob & \"jane\" +^%2{13.";
   
   string encoded = Url::encode(str.c_str(), str.length());
   string decoded = Url::decode(encoded.c_str(), encoded.length());
   
   cout << "test data=" << str << endl;
   
   cout << "url encoded=" << encoded << endl;
   cout << "url decoded=" << decoded << endl;
   
   if(decoded == str)
   {
      cout << "Test successful!" << endl;
   }
   else
   {
      cout << "Test FAILED! Strings do not match!" << endl;
   }
   
   cout << endl << "Url test complete." << endl;
}

void runHttpHeaderTest()
{
   cout << "Starting HttpHeader test." << endl << endl;
   
   // test bicapitalization of http headers
   string header = "ThIs-a-BICaPitAlized-hEADer";
   HttpHeader::biCapitalize(header);
   
   cout << "BiCapitalized Header=" << header << endl;
   
   cout << endl << "HttpHeader test complete." << endl;
}

int main()
{
   cout << "Tests starting..." << endl << endl;
   
   try
   {
      //runBase64Test();
      //runTimeTest();
      //runThreadTest();
      //runWindowsAddressResolveTest();
      //runLinuxAddressResolveTest();
      //runWindowsSocketTest();
      //runLinuxSocketTest();
      //runWindowsSslSocketTest();
      //runLinuxSslSocketTest();
      //runWindowsServerSocketTest();
      //runLinuxServerSocketTest();
      //runWindowsSslServerSocketTest();
      //runLinuxSslServerSocketTest();
      //runWindowsTcpClientServerTest();
      //runLinuxTcpClientServerTest();
      //runWindowsUdpClientServerTest();
      runLinuxUdpClientServerTest();
      //runMessageDigestTest();
      //runCrcTest();
      //runAsymmetricKeyLoadingTest();
      //runDsaAsymmetricKeyCreationTest();
      //runRsaAsymmetricKeyCreationTest();
      //runEnvelopeTest("DSA");
      //runEnvelopeTest("RSA");
      //runCipherTest("AES256");
      //runConvertTest();
      //runUrlTest();
      //runHttpHeaderTest();
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
      cout << "code: " << e.getCode() << endl;
   }
   catch(Exception &e)
   {
      cout << "Exception caught!" << endl;
      cout << "message: " << e.getMessage() << endl;
      cout << "code: " << e.getCode() << endl;
   }
   catch(...)
   {
      cout << "Exception caught!" << endl;
   }
   
   cout << endl << "Tests finished." << endl;
}
