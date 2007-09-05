/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <sstream>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/engine.h>

#include "db/util/Base64Codec.h"
#include "db/rt/Object.h"
#include "db/rt/Runnable.h"
#include "db/rt/Thread.h"
#include "db/rt/Semaphore.h"
#include "db/rt/System.h"
#include "db/rt/JobDispatcher.h"
#include "db/util/StringTools.h"
#include "db/net/TcpSocket.h"
#include "db/net/UdpSocket.h"
#include "db/net/DatagramSocket.h"
#include "db/net/Internet6Address.h"
#include "db/net/SslSocket.h"
#include "db/crypto/MessageDigest.h"
#include "db/util/Crc16.h"
#include "db/crypto/AsymmetricKeyFactory.h"
#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/crypto/DigitalEnvelope.h"
#include "db/crypto/DigitalSignatureInputStream.h"
#include "db/crypto/DigitalSignatureOutputStream.h"
#include "db/crypto/DefaultBlockCipher.h"
#include "db/util/Convert.h"
#include "db/net/Url.h"
#include "db/util/regex/Pattern.h"
#include "db/util/Date.h"
#include "db/net/http/HttpHeader.h"
#include "db/net/http/HttpRequest.h"
#include "db/net/http/HttpResponse.h"
#include "db/net/http/HttpConnectionServicer.h"
#include "db/net/http/HttpRequestServicer.h"
#include "db/net/http/HttpClient.h"
#include "db/modest/Kernel.h"
#include "db/net/Server.h"
#include "db/net/NullSocketDataPresenter.h"
#include "db/net/SslSocketDataPresenter.h"
#include "db/net/SocketDataPresenterList.h"
#include "db/util/StringTokenizer.h"
#include "db/io/FilterOutputStream.h"
#include "db/io/ByteArrayInputStream.h"
#include "db/data/xml/XmlReader.h"
#include "db/data/xml/XmlWriter.h"
#include "db/data/xml/XmlBindingOutputStream.h"
#include "db/data/DataMappingFunctor.h"
#include "db/io/OStreamOutputStream.h"
#include "db/crypto/BigDecimal.h"
#include "db/io/ByteArrayOutputStream.h"

using namespace std;
using namespace db::crypto;
using namespace db::io;
using namespace db::modest;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;
using namespace db::util;
using namespace db::util::regex;
using namespace db::data;
using namespace db::data::xml;

// WTF? this is required to get static library building for unknown reason
#include "db/io/PeekInputStream.h"
PeekInputStream g_junk1(NULL, false);
OperationList g_junk2(false);
NullSocketDataPresenter g_junk3;
StringTokenizer g_junk4;
FilterOutputStream g_junk5(NULL, false);
ByteArrayInputStream g_junk6(NULL, 0);

void runBase64Test()
{
	cout << "Running Base64 Test" << endl << endl;
	
	char data[] = {'a', 'b', 'c', 'd', 'e'};
	string encoded = Base64Codec::encode(data + 1, 4);
	cout << "encoded=" << encoded << endl;
	
   char* decoded;
	unsigned int length;
   Base64Codec::decode(encoded, &decoded, length);
	
	cout << "decoded bytes=" << length << endl;
   for(unsigned int i = 0; i < length; i++)
   {
      cout << "decoded[" << i << "]=" << decoded[i] << endl;
   }
	
	string encoded2 = Base64Codec::encode(decoded, 4);
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
public:
   bool mustWait;
   
   TestRunnable()
   {
      mustWait = true;
   }
   
   virtual ~TestRunnable()
   {
   }
   
   virtual void run()
   {
      Thread* t = Thread::currentThread();
      string name = t->getName();
      cout << name << ": This is a TestRunnable thread,addr=" << t << endl;
      
      if(name == "Thread 1")
      {
         cout << "Thread 1 Waiting for interruption..." << endl;
         InterruptedException* e = NULL;
         
         lock();
         {
            lock();
            lock();
            lock();
            e = wait();
            unlock();
            unlock();
            unlock();
         }
         unlock();
         
         if(Thread::interrupted())
         {
            cout << "Thread 1 Interrupted. Exception message="
                 << e->getMessage() << endl;
         }
         else
         {
            cout << "Thread 1 Finished." << endl;
         }
      }
      else if(name == "Thread 2")
      {
         cout << "Thread 2 Finished." << endl;
      }
      else if(name == "Thread 3")
      {
         cout << "Thread 3 Waiting for Thread 5..." << endl;
         
         lock();
         lock();
         lock();
         {
            cout << "Thread 3 starting wait..." << endl;
            while(mustWait && wait(5000) != NULL);
            cout << "Thread 3 Awake!" << endl;
         }
         unlock();
         unlock();
         unlock();
         
         if(Thread::interrupted())
         {
            cout << "Thread 3 Interrupted." << endl;
         }
         else
         {
            cout << "Thread 3 Finished." << endl;
         }         
      }
      else if(name == "Thread 4")
      {
         cout << "Thread 4 Finished." << endl;
      }
      else if(name == "Thread 5")
      {
         cout << "Thread 5 waking up a thread..." << endl;
         
         lock();
         lock();
         lock();
         lock();
         {
            mustWait = false;
            cout << "Thread 5 notifying a thread..." << endl;
            notify();
            cout << "Thread 5 notified another thread." << endl;
         }
         unlock();
         unlock();
         unlock();
         unlock();
         
         cout << "Thread 5 Finished." << endl;
      }
   }
};

void runThreadTest()
{
   cout << "Running Thread Test" << endl << endl;
   
   TestRunnable r1;
   Thread t1(&r1, "Thread 1");
   Thread t2(&r1, "Thread 2");
   Thread t3(&r1, "Thread 3");
   Thread t4(&r1, "Thread 4");
   Thread t5(&r1, "Thread 5");
   
   cout << "Threads starting..." << endl;
   
   t1.start();
   t2.start();
   t3.start();
   t4.start();
   t5.start();
   
   t1.interrupt();
   
   t2.join();
   t3.join();
   t1.join();
   t4.join();
   t5.join();
   
   cout << endl << "Thread Test complete." << endl;
}

class TestJob : public Runnable
{
public:
   string mName;
   
   TestJob(const string& name)
   {
      mName = name;
   }
   
   virtual ~TestJob()
   {
   }
   
   virtual void run()
   {
      cout << endl << "TestJob: Running a job,name=" << mName << endl;
      
      if(mName == "1")
      {
         Thread::sleep(3000);
      }
      else if(mName == "2")
      {
         Thread::sleep(1000);
      }
      else
      {
         Thread::sleep(1000);
      }
      
      cout << endl << "TestJob: Finished a job,name=" << mName << endl;
   }
};

void runJobThreadPoolTest()
{
   cout << "Running JobThreadPool Test" << endl << endl;
   
   // create a job thread pool
   JobThreadPool pool(3);
   
   // create jobs
   TestJob job1("1");
   TestJob job2("2");
   TestJob job3("3");
   TestJob job4("4");
   TestJob job5("5");
   
   // run jobs
   pool.runJob(&job1);
   pool.runJob(&job2);
   pool.runJob(&job3);
   pool.runJob(&job4);
   pool.runJob(&job5);
   
   // wait
   cout << "Waiting for jobs to complete..." << endl;
   Thread::sleep(10000);
   cout << "Finished waiting for jobs to complete." << endl;
   
   // terminate all jobs
   pool.terminateAllThreads();
   
   cout << endl << "JobThreadPool Test complete." << endl << endl;
}

void runJobDispatcherTest()
{
   cout << "Running JobDispatcher Test" << endl << endl;
   
   // create a job dispatcher
   //JobDispatcher jd;
   JobThreadPool pool(3);
   JobDispatcher jd(&pool, false);
   
   // create jobs
   TestJob job1("1");
   TestJob job2("2");
   TestJob job3("3");
   TestJob job4("4");
   TestJob job5("5");
   TestJob job6("6");
   
   // queue jobs
   jd.queueJob(&job1);
   jd.queueJob(&job2);
   jd.queueJob(&job3);
   jd.queueJob(&job4);
   jd.queueJob(&job5);
   jd.queueJob(&job6);
   
   // start dispatching
   jd.startDispatching();
   
   // wait
   cout << "Waiting 10 seconds for jobs to complete..." << endl;
   Thread::sleep(10000);
   cout << "Finished waiting for jobs to complete." << endl;
   
   // stop dispatching
   jd.stopDispatching();      
   
   cout << endl << "JobDispatcher Test complete." << endl << endl;
}

void runAddressResolveTest()
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

void runSocketTest()
{
   cout << "Running Socket Test" << endl << endl;
   
   // create address
   //InternetAddress address("127.0.0.1", 80);
   InternetAddress address("www.google.com", 80);
   
   // ensure host was known
   if(!Thread::hasException())
   {
      cout << "Connecting to: " << address.getAddress() << endl;
      
      // create tcp socket
      TcpSocket socket;
      
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
      if(numBytes > 0)
      {
         cout << "Peeked " << numBytes << " bytes." << endl;
         string peek = "";
         peek.append(response, numBytes);
         cout << "Peek bytes=" << peek << endl;
      }
      
      cout << endl << "DOING ACTUAL READ NOW!" << endl;
      
      while((numBytes = socket.getInputStream()->read(response, 2048)) > 0)
      {
         cout << "numBytes received: " << numBytes << endl;
         str.append(response, numBytes);
      }
      
   //   char response[2048];
   //   int numBytes = 0;
   //   string str = "";
   //   while((numBytes = socket.receive(response, 0, 2048)) > 0)
   //   {
   //      cout << "numBytes received: " << numBytes << endl;
   //      str.append(response, numBytes);
   //   }
      
      cout << "Response:" << endl << str << endl;
      
      // close
      socket.close();
      
      cout << "Socket connection closed." << endl;
   }
   
   cout << endl << "Socket test complete." << endl;
}

void runSslSocketTest()
{
   cout << "Running SSL Socket Test" << endl << endl;
   
   // openssl initialization code
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();
   
   // FIXME:
   // seed PRNG
   
   // create address
   InternetAddress address("127.0.0.1", 443);
   //InternetAddress address("127.0.0.1", 19020);
   //InternetAddress address("www.google.com", 80);
   cout << address.getAddress() << endl;
   
   // ensure host was known
   if(!Thread::hasException())
   {
      // create tcp socket
      TcpSocket socket;
      
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
      if(numBytes > 0)
      {
         cout << "Peeked " << numBytes << " bytes." << endl;
         string peek = "";
         peek.append(response, numBytes);
         cout << "Peek bytes=" << peek << endl;
      }
      
      cout << endl << "DOING ACTUAL READ NOW!" << endl;
      
      while((numBytes = sslSocket.getInputStream()->read(response, 2048)) > 0)
      {
         cout << "numBytes received: " << numBytes << endl;
         str.append(response, numBytes);
      }
      
      cout << "Response:" << endl << str << endl;
      
      // close
      sslSocket.close();
      
      cout << "SSL Socket connection closed." << endl;
   }
   
   cout << endl << "SSL Socket test complete." << endl;
   
   // clean up SSL
   EVP_cleanup();
}

void runServerSocketTest()
{
   cout << "Running Server Socket Test" << endl << endl;
   
   // bind and listen
   InternetAddress address("127.0.0.1", 19100);
   
   // ensure host was known
   if(!Thread::hasException())
   {
      // create tcp socket
      TcpSocket socket;
      
      if(socket.bind(&address))
      {
         cout << "Server socket bound..." << endl;
      }
      else
      {
         cout << "Could not bind server socket!" << endl;
      }
      
      if(socket.listen())
      {
         cout << "Listening for a connection..." << endl;
      }
      else
      {
         cout << "Could not listen with server socket!" << endl;
      }
      
      string str = "HTTP/1.0 200 OK\r\nContent-Length: 0\r\n\r\n";
      while(!Thread::interrupted(false))
      {
         // accept a connection
         Socket* worker = socket.accept(1);
         if(worker != NULL)
         {
            char request[100];
            int numBytes = 0;
            
            numBytes = worker->getInputStream()->peek(request, 100);
            worker->getOutputStream()->write(str.c_str(), str.length());
            
            // close worker socket
            worker->close();
            delete worker;
         }
      }
      
      // close server socket
      socket.close();
      
      cout << "Server Socket connection closed." << endl;
   }
   
   cout << endl << "Server Socket test complete." << endl;
}

void runSslServerSocketTest()
{
   cout << "Running SSL Server Socket Test" << endl << endl;
   
   // openssl initialization code
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();   
   
   // bind and listen
   InternetAddress address("127.0.0.1", 1024);
   
   // ensure host was known
   if(!Thread::hasException())
   {
      // create tcp socket
      TcpSocket socket;
      
      if(socket.bind(&address))
      {
         cout << "Server socket bound..." << endl;
      }
      else
      {
         cout << "Could not bind server socket!" << endl;
      }
      
      if(socket.listen())
      {
         cout << "Listening for a connection..." << endl;
      }
      else
      {
         cout << "Could not listen with server socket!" << endl;
      }
      
      // accept a connection
      TcpSocket* worker = (TcpSocket*)socket.accept(10);
      if(worker != NULL)
      {
         cout << "Accepted a connection!" << endl;
         
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
         if(numBytes > 0)
         {
            cout << "Peeked " << numBytes << " bytes." << endl;
            string peek = "";
            peek.append(request, numBytes);
            cout << "Peek bytes=" << peek << endl;
         }
         
         cout << endl << "DOING ACTUAL READ NOW!" << endl;
         
         while((numBytes = sslSocket.getInputStream()->read(request, 2048)) > 0)
         {
            cout << "numBytes received: " << numBytes << endl;
            str.append(request, numBytes);
         }
         
         cout << "Request:" << endl << str << endl;
         
         // close ssl socket socket
         sslSocket.close();
         delete worker;
      }
      else
      {
         cout << "Could not accept a connection!" << endl;
      }
      
      // close server socket
      socket.close();
      
      cout << "SSL Server Socket connection closed." << endl;
   }
   
   cout << endl << "SSL Server Socket test complete." << endl;
   
   // clean up SSL
   EVP_cleanup();
}

void runTcpClientServerTest()
{
   cout << "Running TCP Client/Server Test" << endl << endl;
   
   InternetAddress* address;
   InternetAddress ia("127.0.0.1", 9999);
   //Internet6Address ia("::0", 9999);
   address = &ia;
   
   // ensure host was known
   if(!Thread::hasException())
   {
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
   }
   
   cout << endl << "TCP Client/Server test complete." << endl;
}

void runUdpClientServerTest()
{
   cout << "Running UDP Client/Server Test" << endl << endl;
   
   InternetAddress* sa;
   InternetAddress* ca;
   InternetAddress serverAddress("127.0.0.1", 9999);
   InternetAddress clientAddress("127.0.0.1", 0);
   //Internet6Address serverAddress("::1", 9999);
   //Internet6Address clientAddress("::1", 0);
   sa = &serverAddress;
   ca = &clientAddress;
   
   // ensure host was known
   if(!Thread::hasException())
   {
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
   }
   
   cout << endl << "UDP Client/Server test complete." << endl;
}

void runDatagramTest()
{
   cout << "Running Datagram Test" << endl << endl;
   
   InternetAddress* sa;
   InternetAddress* ca;
   InternetAddress serverAddress("127.0.0.1", 9999);
   InternetAddress clientAddress("127.0.0.1", 0);
   //Internet6Address serverAddress("::1", 9999);
   //Internet6Address clientAddress("::1", 0);
   sa = &serverAddress;
   ca = &clientAddress;
   
   // ensure host was known
   if(!Thread::hasException())
   {
      // create datagram server and client sockets
      DatagramSocket server;
      DatagramSocket client;
      
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
      
      // create a datagram
      Datagram d1(sa);
      d1.assignString("Hello there, Server.");
      
      // send the datagram with the client
      client.send(&d1);
      
      cout << "Client sent: " << d1.getString() << endl;
      
      // create a datagram
      char externalData[2048];
      Datagram d2(ca);
      d2.setData(externalData, 2048, false);
      
      // receive a datagram
      server.receive(&d2);
      
      cout << "Server received: " << d2.getString() << endl;
      cout << "Data from: " << d2.getAddress()->getAddress();
      cout << ":" << d2.getAddress()->getPort() << endl;
      
      // send a datagram with the server
      d2.assignString("G'day, Client.");
      server.send(&d2);
      
      cout << "Server sent: " << d2.getString() << endl;
      
      // receive the server datagram
      Datagram d3(sa, 2048);
      client.receive(&d3);
      
      cout << "Client received: " << d3.getString() << endl;
      cout << "Data from: " << d3.getAddress()->getAddress();
      cout << ":" << d3.getAddress()->getPort() << endl;
      
      // close sockets
      client.close();
      server.close();
      
      cout << "Sockets closed." << endl;
   }
   
   cout << endl << "Datagram test complete." << endl;
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
   
   // read in PEM private key
   File file1("/work/src/dbcpp/dbcore/trunk/Debug/private.pem");
   FileInputStream fis1(&file1);
   
   string privatePem = "";
   
   char b[2048];
   int numBytes;
   while((numBytes = fis1.read(b, 2048)) > 0)
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
   
   while((numBytes = fis2.read(b, 2048)) > 0)
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
   
   cout << endl << "RSA Asymmetric Key Creation test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runDigitalSignatureInputStreamTest()
{
   cout << "Running DigitalSignatureInputStream Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
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
      
      char dummy[8];
      ByteArrayInputStream bais(data, 8);
      DigitalSignatureInputStream dsos1(ds1, &bais, false);
      dsos1.read(dummy, 8);
      
      // get the signature
      char sig[ds1->getValueLength()];
      unsigned int length;
      ds1->getValue(sig, length);
      delete ds1;
      
      // verify the signature
      DigitalSignature* ds2 = publicKey->createSignature();
      bais.setByteArray(data, 8);
      DigitalSignatureInputStream dsos2(ds2, &bais, false);
      dsos2.read(dummy, 8);
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
   
   cout << endl << "DigitalSignatureInputStream test complete." << endl;
   
   // clean up crypto strings
   EVP_cleanup();
}

void runDigitalSignatureOutputStreamTest()
{
   cout << "Running DigitalSignatureOutputStream Test" << endl << endl;
   
   // include crypto error strings
   ERR_load_crypto_strings();
   
   // add all algorithms
   OpenSSL_add_all_algorithms();
   
   // seed PRNG
   //RAND_load_file("/dev/urandom", 1024);
   
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
      
      ostringstream oss;
      OStreamOutputStream osos(&oss);
      DigitalSignatureOutputStream dsos1(ds1, &osos, false);
      dsos1.write(data, 8);
      
      // get the signature
      char sig[ds1->getValueLength()];
      unsigned int length;
      ds1->getValue(sig, length);
      delete ds1;
      
      // verify the signature
      DigitalSignature* ds2 = publicKey->createSignature();
      DigitalSignatureOutputStream dsos2(ds2, &osos, false);
      dsos2.write(data, 8);
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
   
   cout << endl << "DigitalSignatureOutputStream test complete." << endl;
   
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
   
   // get an asymmetric key factory
   AsymmetricKeyFactory factory;
   
   // create a new key pair
   PrivateKey* privateKey;
   PublicKey* publicKey;
   factory.createKeyPair("RSA", &privateKey, &publicKey);
   
   if(privateKey != NULL && publicKey != NULL)
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
   cipher.startEncrypting(algorithm.c_str(), &key);
   
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
   
   cout << "10 to lower-case hex=" << Convert::intToHex(10) << endl;
   cout << "33 to lower-case hex=" << Convert::intToHex(33) << endl;
   cout << "100 to lower-case hex=" << Convert::intToHex(100) << endl;
   cout << "10 to upper-case hex=" << Convert::intToUpperHex(10) << endl;
   cout << "33 to upper-case hex=" << Convert::intToUpperHex(33) << endl;
   cout << "100 to upper-case hex=" << Convert::intToUpperHex(100) << endl;
   cout << "8975 to lower-case hex=" << Convert::intToHex(8975) << endl;
   cout << "8975 to upper-case hex=" << Convert::intToUpperHex(8975) << endl;
   cout << "65537 to lower-case hex=" << Convert::intToHex(65537) << endl;
   cout << "65537 to upper-case hex=" << Convert::intToUpperHex(65537) << endl;
   
   string hex = "230f";
   cout << "0x230f to integer=" <<
      Convert::hexToInt(hex.c_str(), hex.length()) << endl;
   hex = "230F";
   cout << "0x230F to integer=" <<
      Convert::hexToInt(hex.c_str(), hex.length()) << endl;
   hex = "230FABCD";
   cout << "0x230FABCD to integer=" <<
      Convert::hexToInt(hex.c_str(), hex.length()) << endl;
   hex = "0";
   cout << "0x0 to integer=" <<
      Convert::hexToInt(hex.c_str(), hex.length()) << endl;
   
   cout << endl << "Convert test complete." << endl;
}

void runUrlEncodeTest()
{
   cout << "Starting Url Encode/Decode test." << endl << endl;
   
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
   
   cout << endl << "Url Encode/Decode test complete." << endl;
}

void runUrlTest()
{
   cout << "Starting Url test." << endl << endl;
   
   Url url("http://www.bitmunk.com/mypath?variable1=test");
   
   // ensure URL was valid
   if(!Thread::hasException())
   {
      string str;
      url.toString(str);
      
      cout << "url=" << str << endl;
      cout << "scheme=" << url.getScheme() << endl;
      cout << "scheme specific part=" << url.getSchemeSpecificPart() << endl;
      cout << "authority=" << url.getAuthority() << endl;
      cout << "path=" << url.getPath() << endl;
      cout << "query=" << url.getQuery() << endl;
   }
   
   cout << endl << "Url test complete." << endl;
}

void runRegexTest()
{
   cout << "Starting Regex test." << endl << endl;
   
   string regex = "[a-z]{3}";
   string str = "abc";
   
   if(Pattern::match(regex.c_str(), str.c_str()))
   {
      cout << "Simple pattern matches!" << endl;
   }
   else
   {
      cout << "Simple pattern DOES NOT MATCH!" << endl;
   }
   
   cout << endl << "Doing sub-match test..." << endl << endl;
   
   string submatches = "Look for green globs of green matter in green goo.";
   Pattern* p = Pattern::compile("green");
   
   unsigned int start, end;
   unsigned int index = 0;
   while(p->match(submatches.c_str(), index, start, end))
   {
      cout << "Found match at (" << start << ", " << end << ")" << endl;
      cout << "Match=" << submatches.substr(start, end - start) << endl;
      index = end;
   }
   
   delete p;
   
   cout << endl << "Doing replace all test..." << endl << endl;
   
   cout << "change 'green' to 'blue'" << endl;
   cout << submatches << endl;
   StringTools::regexReplaceAll(submatches, "green", "blue");
   cout << submatches << endl;
   
   cout << endl << "Regex test complete." << endl;
}

void runDateTest()
{
   cout << "Starting Date test." << endl << endl;
   
   TimeZone gmt = TimeZone::getTimeZone("GMT");
   TimeZone local = TimeZone::getTimeZone();
   
   Date d;
   string str;
   //d.format(str);
   //d.format(str, "E EEEE d dd M MMMM MM yy w ww yyyy a", "java");
   //d.format(str, "EEEE, MMMM dd yyyy hh:mm:ss a", "java");
   //d.format(str, "EEE, MMMM dd yyyy hh:mm:ss a", "java", &local);
   //d.format(str, "EEE, d MMM yyyy HH:mm:ss", "java", &gmt);
   //d.format(str, "%a, %d %b %Y %H:%M:%S");
   d.format(str, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   //d.format(str, "%a, %d %b %Y %H:%M:%S", "c", &local);
   
   cout << "Current Date: " << str << endl;
   
   // parse date
   Date d2;
   d2.parse(str, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   //d2.parse(str, "%a, %d %b %Y %H:%M:%S", "c", &local);
   string str2;
   d2.format(str2, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   //d2.format(str2, "%a, %d %b %Y %H:%M:%S", "c", &local);
   
   cout << "Parsed Date 1: " << str2 << endl;
   
//   // FIXME: parser may have a problem with AM/PM
   // parse date again
   Date d3;
   str = "Thu, 02 Aug 2007 10:30:00";
   d3.parse(str, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   string str3;
   //d3.format(str3, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   d3.format(str3, "%a, %d %b %Y %H:%M:%S", "c", &local);
   
   cout << "Parsed Date 2: " << str3 << endl;
   
   cout << endl << "Date test complete." << endl;
}

class InterruptTest : public virtual Object, public Runnable
{
public:
   /**
    * Runs the unit tests.
    */
   virtual void run()
   {
      runServerSocketTest();
      
      if(Thread::hasException())
      {
         Exception* e = Exception::getLast();
         cout << "Exception occurred!" << endl;
         cout << "message: " << e->getMessage() << endl;
         cout << "code: " << e->getCode() << endl;
      }      
   }
};

void runInterruptTest()
{
   InterruptTest runnable;
   Thread t(&runnable);
   t.start();
   
   cout << "Waiting for thread..." << endl;
   Thread::sleep(2000);
   cout << "Finished waiting for thread." << endl;
   
   cout << "Interrupting thread..." << endl;
   t.interrupt();
   
   cout << "Joining thread..." << endl;
   t.join();
   cout << "Thread joined." << endl;
}

class TestGuard : public OperationGuard
{
public:
   virtual bool canExecuteOperation(ImmutableState* s)
   {
      bool rval = false;
      
      int ops = 0;
      s->getInteger("number.of.ops", ops);
      
      bool loggingOut = false;
      s->getBoolean("logging.out", loggingOut);
      
      rval = !loggingOut && ops < 3;
      if(!rval)
      {
         cout << "Operation must wait or cancel." << endl;
      }
      else
      {
         cout << "Operation can run." << endl;
      }
      
      return rval;
   }
   
   virtual bool mustCancelOperation(ImmutableState* s)
   {
      bool loggedOut = false;
      s->getBoolean("logged.out", loggedOut);
      
      if(loggedOut)
      {
         cout << "Operation must cancel, user logged out." << endl;
      }
      else
      {
         cout << "Operation can wait, user is not logged out yet." << endl;
      }
      
      return loggedOut;
   }
};

class TestStateMutator : public StateMutator
{
protected:
   bool mLogout;
public:
   TestStateMutator(bool logout)
   {
      mLogout = logout;
   }
   
   virtual void mutatePreExecutionState(State* s, Operation* op)
   {
      int ops = 0;
      s->getInteger("number.of.ops", ops);
      s->setInteger("number.of.ops", ++ops);
      
      if(mLogout)
      {
         s->setBoolean("logging.out", true);
         cout << "Logging out..." << endl;
      }
   }
   
   virtual void mutatePostExecutionState(State* s, Operation* op)
   {
      int ops = 0;
      s->getInteger("number.of.ops", ops);
      s->setInteger("number.of.ops", --ops);
      
      if(mLogout)
      {
         s->setBoolean("logged.out", true);
         cout << "Logged out." << endl;
      }
   }
};

class RunOp : public virtual Object, public Runnable
{
protected:
   string mName;
   unsigned long mTime;
   
public:
   RunOp(string name, unsigned long time)
   {
      mName = name;
      mTime = time;
   }
   
   virtual void run()
   {
      cout << "Operation running: " << mName << endl;
      
      lock();
      {
         wait(mTime);
      }
      unlock();
      
      cout << "Operation finished: " << mName << endl;
   }
   
   virtual string& toString(string& str)
   {
      str = mName;
      return mName;
   }
};

void runModestTest()
{
   cout << "Starting Modest test." << endl << endl;
   
   Kernel k;
   
   cout << "Modest engine started." << endl;
   k.getEngine()->start();
   
   RunOp r1("Number 1", 2000);
   RunOp r2("Number 2", 2000);
   RunOp r3("Number 3", 2000);
   RunOp r4("Number 4", 2000);
   RunOp r5("Number 5", 2000);
   RunOp rLogout("Logout", 1000);
   
   TestStateMutator sm(false);
   TestStateMutator smLogout(true);
   TestGuard g;
   
   Operation op1(&r1, &g, &sm);
   Operation op2(&r2, &g, &sm);
   Operation op3(&r3, &g, &sm);
   Operation op4(&r4, &g, &sm);
   Operation op5(&r5, &g, &sm);
   Operation opLogout(&rLogout, &g, &smLogout);
   
   k.getEngine()->queue(&op1);
   k.getEngine()->queue(&op2);
   k.getEngine()->queue(&op3);
   k.getEngine()->queue(&op4);
   k.getEngine()->queue(&opLogout);
   k.getEngine()->queue(&op5);
   
   op1.waitFor();
   op2.waitFor();
   op3.waitFor();
   op4.waitFor();
   op5.waitFor();
   opLogout.waitFor();
   
   cout << "Operations complete." << endl;
   
   k.getEngine()->stop();
   cout << "Modest engine stopped." << endl;
   
   cout << endl << "Modest test complete." << endl;
}

class TestConnectionServicer1 : public ConnectionServicer
{
public:
   unsigned int serviced;
   string reply;
   
   TestConnectionServicer1()
   {
      serviced = 0;
      reply = "HTTP/1.0 200 OK\r\nContent-Length: 0\r\n\r\n";
      //reply = "HTTP/1.0 404 Not Found\r\n";
   }
   
   virtual ~TestConnectionServicer1() {}
   
   void serviceConnection(Connection* c)
   {
      //cout << "1: Servicing connection!" << endl;
      
      char b[100];
      int numBytes = 0;
      
      //cout << endl << "Reading HTTP..." << endl;
      
      InputStream* is = c->getInputStream();
      numBytes = is->peek(b, 100);
      if(numBytes > 0)
      {
//         cout << "Read " << numBytes << " bytes." << endl;
//         string str = "";
//         str.append(b, numBytes);
//         cout << "HTTP=" << endl << str << endl;
      }
      
      OutputStream* os = c->getOutputStream();
      os->write(reply.c_str(), reply.length());
      
      //cout << "1: Finished servicing connection." << endl;
      
      serviced++;
      //cout << "Connections serviced=" << serviced << endl;
   }
};

class TestConnectionServicer2 : public ConnectionServicer
{
   void serviceConnection(Connection* c)
   {
      cout << "2: Servicing connection!" << endl;
      cout << "2: Finished servicing connection." << endl;
   }
};

class TestConnectionServicer3 : public ConnectionServicer
{
   void serviceConnection(Connection* c)
   {
      cout << "3: Servicing connection!" << endl;
      cout << "3: Finished servicing connection." << endl;
   }
};

void runServerConnectionTest()
{
   cout << "Starting Server Connection test." << endl << endl;
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("0.0.0.0", 19100);
   
   // create generic service
   TestConnectionServicer1 tcs1;
   server.addConnectionService(&address, &tcs1);
   
//   // create generic service (stomp on other service)
//   TestConnectionServicer2 tcs2;
//   server.addConnectionService(&address, &tcs2);
   
   if(server.start())
   {
      cout << "Server started." << endl;
   }
   else if(Exception::getLast() != NULL)
   {
      cout << "Server started with errors=" <<
         Exception::getLast()->getMessage() << endl;
   }
   
//   // create generic service (stomp on second service, dynamically stop/start)
//   TestConnectionServicer3 tcs3;
//   if(!server.addConnectionService(&address, &tcs3))
//   {
//      cout << "Could not start service 3!, exception=" <<
//         Exception::getLast()->getMessage() << endl;
//   }
//   
//   Thread::sleep(5000);
//   
//   // create generic service (stomp on third service, dynamically stop/start)
//   if(!server.addConnectionService(&address, &tcs2))
//   {
//      cout << "Could not start service 2!, exception=" <<
//         Exception::getLast()->getMessage() << endl;
//   }
   
   Object lock;
   lock.lock();
   {
      lock.wait();//lock.wait(120000);
      //lock.wait(30000);
   }
   lock.unlock();
   //Thread::sleep(60000);
   
   server.stop();
   cout << "Server stopped." << endl;
   
   // stop kernel engine
   k.getEngine()->stop();
   
   cout << endl << "Server Connection test complete." << endl;
}
unsigned int gConnections = 0;

class BlastConnections : public Runnable
{
public:
   InternetAddress* address;
   
   BlastConnections(InternetAddress* a)
   {
      address = a;
   }
   
   virtual ~BlastConnections()
   {
   }
   
   void run()
   {
      //Thread::sleep(20000);
      
      TcpSocket socket;
      socket.setReceiveTimeout(1000);
      
      //InternetAddress address2("mojo.bitmunk.com", 9120);
      
      // blast connections
      int connections = 50;
      char b[1024];
      string request =
         "GET / HTTP/1.0\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
      for(int i = 0; i < connections; i++)
      {
         // connect
         if(socket.connect(address))
         {
            //cout << "connected" << endl;
            
            // send request
            if(socket.send(request.c_str(), request.length()))
            {
               // receive response
               socket.receive(b, 1024);
            }
            else
            {
               cout << "Exception=" <<
                  Exception::getLast()->getMessage() << endl;
            }
         }
         else
         {
            cout << "Exception=" <<
               Exception::getLast()->getMessage() << endl;
         }
         
         // close socket
         socket.close();
         
         gConnections++;
      }      
   }
};

void runServerSslConnectionTest()
{
   cout << "Starting Server SSL Connection test." << endl << endl;
   
   // openssl initialization code
   SSL_library_init();
   SSL_load_error_strings();
   OpenSSL_add_all_algorithms();
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("localhost", 19100);
   
//   // create SSL-only service
//   TestConnectionServicer1 tcs1;
//   SslContext context;
//   SslSocketDataPresenter presenter(&context);
//   server.addConnectionService(&address, &tcs1, &presenter);
   
   // create SSL/generic service
   TestConnectionServicer1 tcs1;
   SslContext context;
   SslSocketDataPresenter presenter1(&context);
   NullSocketDataPresenter presenter2;
   SocketDataPresenterList list(false);
   list.add(&presenter1);
   list.add(&presenter2);
   server.addConnectionService(&address, &tcs1, &list);
   
   if(server.start())
   {
      cout << "Server started." << endl;
   }
   else if(Exception::getLast() != NULL)
   {
      cout << "Server started with errors=" <<
         Exception::getLast()->getMessage() << endl;
   }
   
   BlastConnections bc(&address);
   Thread t1(&bc);
   Thread t2(&bc);
   Thread t3(&bc);
   Thread t4(&bc);
   Thread t5(&bc);
   Thread t6(&bc);
   Thread t7(&bc);
   Thread t8(&bc);
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   t1.start();
   t2.start();
//   t3.start();
//   t4.start();
//   t5.start();
//   t6.start();
//   t7.start();
//   t8.start();
   
   t1.join();
   t2.join();
//   t3.join();
//   t4.join();
//   t5.join();
//   t6.join();
//   t7.join();
//   t8.join();
   cout << "all client threads joined." << endl;
   
   unsigned long long end = System::getCurrentMilliseconds();
   long double time = end - start;
   long double secs = time / 1000.0;
   unsigned int connections = gConnections;//tcs1.serviced
   double rate = (double)connections / secs;
   
   cout << "Connections=" << tcs1.serviced << endl;
   cout << "Time=" << time << " ms = " << secs << " secs" << endl;
   cout << "Connections/second=" << rate << endl;
   
   server.stop();
   cout << "Server stopped." << endl;
   
   // stop kernel engine
   k.getEngine()->stop();
   
   // clean up SSL
   EVP_cleanup();
   
   cout << endl << "Server SSL Connection test complete." << endl;
}

class TestDatagramServicer : public DatagramServicer
{
   void serviceDatagrams(DatagramSocket* s)
   {
      cout << "Servicing datagrams!" << endl;
      cout << "Finished servicing datagrams." << endl;
   }
};

void runServerDatagramTest()
{
   cout << "Starting Server Datagram test." << endl << endl;
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("localhost", 10080);
   
   // create datagram service
   TestDatagramServicer tds;
   server.addDatagramService(&address, &tds);
   
   if(server.start())
   {
      cout << "Server started." << endl;
   }
   else if(Exception::getLast() != NULL)
   {
      cout << "Server started with errors=" <<
         Exception::getLast()->getMessage() << endl;
   }
   
   Thread::sleep(10000);
   
   server.stop();
   cout << "Server stopped." << endl;
   
   // stop kernel engine
   k.getEngine()->stop();
   
   cout << endl << "Server Datagram test complete." << endl;
}

void runByteArrayInputStreamTest()
{
   cout << "Starting ByteArrayInputStream test." << endl << endl;
   
   char html[] = "<html>505 HTTP Version Not Supported</html>";
   ByteArrayInputStream is(html, 43);
   
   char b[10];
   int numBytes;
   string str;
   while((numBytes = is.read(b, 9)) > 0)
   {
      memset(b + numBytes, 0, 1);
      str.append(b);
   }
   
   cout << "read data='" << str << "'" << endl;
   
   cout << endl << "ByteArrayInputStream test complete." << endl;
}

void runStringTokenizerTest()
{
   cout << "Starting StringTokenizer test." << endl << endl;
   
   char* str = "This is a test of the StringTokenizer class.";
   
   StringTokenizer st(str, ' ');
   while(st.hasNextToken())
   {
      cout << "token='" << st.nextToken() << "'" << endl;
   }
   
   cout << endl << "StringTokenizer test complete." << endl;
}

void runStringEqualityTest()
{
   cout << "Starting string equality test." << endl << endl;
   
   // Note: string length doesn't appear to matter
   string str = "blah";
   unsigned long long start, end;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str == "");
   }
   end = System::getCurrentMilliseconds();
   cout << "String == \"\" time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str.length() == 0);
   }
   end = System::getCurrentMilliseconds();
   cout << "String.length() == 0 time: " << (end - start) << " ms" << endl;
   
   // Note: test demonstrates that comparing to length is about 6 times faster
   
   cout << endl << "String equality test complete." << endl;
}

void runStringAppendCharTest()
{
   cout << "Starting string append char test." << endl << endl;
   
   // Note: string length doesn't appear to matter
   string str = "blah";
   unsigned long long start, end;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str.length() == 1 && str[0] == '/');
   }
   end = System::getCurrentMilliseconds();
   cout << "String.length() == 1 && str[0] == '/' time: " <<
      (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str == "/");
   }
   end = System::getCurrentMilliseconds();
   cout << "String == \"/\" time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(strcmp(str.c_str(), "/") == 0);
   }
   end = System::getCurrentMilliseconds();
   cout << "strcmp(String.c_str(), \"/\") == 0 time: " <<
      (end - start) << " ms" << endl;
   
   string version = "HTTP/1.0";
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(version == "HTTP/1.0");
   }
   end = System::getCurrentMilliseconds();
   cout << "String == \"HTTP/1.0\" time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(strcmp(version.c_str(), "HTTP/1.0") == 0);
   }
   end = System::getCurrentMilliseconds();
   cout << "strcmp(String.c_str(), \"HTTP/1.0\") == 0 time: " <<
      (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; i++)
   {
      str.append(1, '/');
   }
   end = System::getCurrentMilliseconds();
   cout << "String.append(1, '/') time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; i++)
   {
      str.append("/");
   }
   end = System::getCurrentMilliseconds();
   cout << "String.append(\"/\") time: " << (end - start) << " ms" << endl;
   
   string space = " ";
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; i++)
   {
      str.append("this" + space + "is a sentence");
   }
   end = System::getCurrentMilliseconds();
   cout << "String inline append time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 10000; i++)
   {
      str.append("this");
      str.append(space);
      str.append("is a sentence");
   }
   end = System::getCurrentMilliseconds();
   cout << "String multiline append time: " << (end - start) << " ms" << endl;
   
   cout << endl << "String append char test complete." << endl;
}

void runStringCompareTest()
{
   cout << "Starting string compare test." << endl << endl;
   
   string str1 = "blah";
   char str2[] = "blah";
   unsigned long long start, end;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(str1 == "blah");
   }
   end = System::getCurrentMilliseconds();
   cout << "std::string compare time: " << (end - start) << " ms" << endl;
   
   start = System::getCurrentMilliseconds();
   for(int i = 0; i < 1000000; i++)
   {
      if(strcmp(str2, "blah") == 0);
   }
   end = System::getCurrentMilliseconds();
   cout << "char* compare time: " << (end - start) << " ms" << endl;
   
   cout << endl << "String compare test complete." << endl;
}

void runHttpHeaderTest()
{
   cout << "Starting HttpHeader test." << endl << endl;
   
   // test bicapitalization of http headers
   char test[] = "ThIs-a-BICaPitAlized-hEADer";
   HttpHeader::biCapitalize(test);
   
   cout << "BiCapitalized Header=" << test << endl;
   
//   string t = "   d  f  ";
//   StringTools::trim(t);
//   cout << "t='" << t << "'" << endl;
   
   cout << endl << "Request Header:" << endl;
   
   HttpRequestHeader reqHeader;
   reqHeader.setDate();
   reqHeader.setMethod("GET");
   reqHeader.setPath("/");
   reqHeader.setVersion("HTTP/1.1");
   reqHeader.setField("host", "localhost:80");
   reqHeader.setField("Content-Type", "text/html");
   reqHeader.setField("Connection", "close");
   
   string str;
   reqHeader.toString(str);
   cout << str;
   
   cout << "End of Request Header." << endl;
   
   cout << endl << "Parsed Request Header:" << endl;
   
   HttpRequestHeader reqHeader2;
   reqHeader2.parse(str);
   
   string str2;
   reqHeader2.toString(str2);
   cout << str2;
   
   cout << "End of Parsed Request Header." << endl;
   
   cout << endl << "Response Header:" << endl;
   
   HttpResponseHeader resHeader;
   resHeader.setDate();
   resHeader.setVersion("HTTP/1.1");
   resHeader.setStatus(404, "Not Found");
   resHeader.setField("host", "localhost:80");
   resHeader.setField("Content-Type", "text/html");
   resHeader.setField("Connection", "close");
   
   resHeader.toString(str);
   cout << str;
   
   cout << "End of Response Header." << endl;
   
   cout << endl << "Parsed Response Header:" << endl;
   
   HttpResponseHeader resHeader2;
   resHeader2.parse(str);
   
   resHeader2.toString(str2);
   cout << str2;
   
   cout << "End of Parsed Response Header." << endl;
   
   cout << endl << "HttpHeader test complete." << endl;
}

class TestHttpRequestServicer : public HttpRequestServicer
{
public:
   TestHttpRequestServicer(const char* path) : HttpRequestServicer(path)
   {
   }
   
   virtual ~TestHttpRequestServicer()
   {
   }
   
   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
      char content[] = "Bob Loblaw's Law Blog";
      
      // send 200 OK
      response->getHeader()->setStatus(200, "OK");
      //response->getHeader()->setField("Content-Length", 0);
      response->getHeader()->setField("Transfer-Encoding", "chunked");
      response->getHeader()->setField("Connection", "close");
      response->sendHeader();
      
      HttpTrailer trailer;
      ByteArrayInputStream bais(content, strlen(content));
      response->sendBody(&bais, &trailer);
   }
};

void runHttpServerTest()
{
   cout << "Starting Http Server test." << endl << endl;
   
//   // openssl initialization code
//   SSL_library_init();
//   SSL_load_error_strings();
//   OpenSSL_add_all_algorithms();
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("localhost", 19100);
   
   // create SSL/generic http connection servicer
   HttpConnectionServicer hcs;
//   SslContext context;
//   SslSocketDataPresenter presenter1(&context);
//   NullSocketDataPresenter presenter2;
//   SocketDataPresenterList list(false);
//   list.add(&presenter1);
//   list.add(&presenter2);
   server.addConnectionService(&address, &hcs);//, &list);
   
   // create test http request servicer
   TestHttpRequestServicer test1("/test");
   hcs.addRequestServicer(&test1, false);
   
   if(server.start())
   {
      cout << "Server started." << endl;
   }
   else if(Exception::getLast() != NULL)
   {
      cout << "Server started with errors=" <<
         Exception::getLast()->getMessage() << endl;
   }
   
   // sleep
   Thread::sleep(30000);
   
   server.stop();
   cout << "Server stopped." << endl;
   
   // stop kernel engine
   k.getEngine()->stop();
   
//   // clean up SSL
//   ERR_remove_state(0);
//   ENGINE_cleanup();
//   ERR_free_strings();
//   EVP_cleanup();
//   CRYPTO_cleanup_all_ex_data();
   
   cout << endl << "Http Server test complete." << endl;
}

void runHttpClientGetTest()
{
   cout << "Starting Http Client GET test." << endl << endl;
   
   // create client
   HttpClient client;
   
   // connect
   Url url("http://www.bitmunk.com");
   if(client.connect(&url))
   {
      string str;
      cout << "Connected to: " << url.toString(str) << endl;
      InternetAddress address(url.getHost(), url.getPort());
      cout << address.toString(str) << endl;
      
      // do get
      char* headers[] = {"Test-Header: bacon", NULL};
      HttpResponse* response = client.get(&url, headers);
      if(response != NULL)
      {
         cout << "Response=" << endl <<
            response->getHeader()->toString(str) << endl;
         if(response->getHeader()->getStatusCode() == 200)
         {
            // receive content
            HttpTrailer trailer;
            File file("/tmp/index.html");
            FileOutputStream fos(&file);
            IOException* e = client.receiveContent(&fos, &trailer);
            if(e == NULL)
            {
               cout << "Content downloaded to '" <<
                  file.getName() << "'" << endl;
               
               cout << "HTTP trailers=\n" << trailer.toString(str) << endl;
            }
            else
            {
               cout << "IOException!,message=" << e->getMessage() << endl;
            }
         }
      }
      else
      {
         cout << "There was no response!" << endl;
      }
      
      cout << "Disconnecting..." << endl;
      client.disconnect();
      cout << "Disconnected." << endl;
   }
   
   cout << endl << "Http Client GET test complete." << endl;
}

void runHttpClientPostTest()
{
   cout << "Starting Http Client POST test." << endl << endl;
   
   // create client
   HttpClient client;
   
   // connect
   Url url("http://www.bitmunk.com");
   if(client.connect(&url))
   {
      string str;
      cout << "Connected to: " << url.toString(str) << endl;
      InternetAddress address(url.getHost(), url.getPort());
      cout << address.toString(str) << endl;
      
      char someData[] = "Just some post data.";
      ByteArrayInputStream baos(someData, strlen(someData));
      
      // do post
      char* headers[] = {
         "Content-Type: text/plain",
         "Transfer-Encoding: chunked",
         NULL};
      
      HttpTrailer trailer;
      HttpResponse* response = client.post(&url, headers, &baos, &trailer);
      if(response != NULL)
      {
         cout << "Response=" << endl <<
            response->getHeader()->toString(str) << endl;
         if(response->getHeader()->getStatusCode() == 200)
         {
            // receive content
            trailer.clearFields();
            File file("/tmp/postresponse.txt");
            FileOutputStream fos(&file);
            IOException* e = client.receiveContent(&fos, &trailer);
            if(e == NULL)
            {
               cout << "Content downloaded to '" <<
                  file.getName() << "'" << endl;
               
               cout << "HTTP trailers=\n" << trailer.toString(str) << endl;
            }
            else
            {
               cout << "IOException!,message=" << e->getMessage() << endl;
            }
         }
      }
      else
      {
         cout << "There was no response!" << endl;
      }
      
      cout << "Disconnecting..." << endl;
      client.disconnect();
      cout << "Disconnected." << endl;
   }
   
   cout << endl << "Http Client POST test complete." << endl;
}

void runDelegateTest()
{
   cout << "Starting Delegate test." << endl << endl;
   
   // FIXME:
   
   cout << endl << "Delegate test complete." << endl;
}

class TestContent
{
protected:
   char* mContent;
   
public:
   TestContent()
   {
      mContent = new char[1];
      memset(mContent, 0, 1);
   }
   
   virtual ~TestContent()
   {
      delete mContent;
   }
   
   virtual void setContent(const char* str)
   {
      delete mContent;
      mContent = new char[strlen(str) + 1];
      strcpy(mContent, str);
   }
   
   virtual const char* getContent()
   {
      return mContent;
   }
};

class TestChild : public TestContent
{
protected:
   int mId;
   
public:
   TestChild()
   {
      mId = 0;
   }
   
   virtual ~TestChild()
   {
   }
   
   virtual void setId(int id)
   {
      mId = id;
   }
   
   virtual int getId()
   {
      return mId;
   }
};

class TestParent : public TestContent
{
protected:
   TestChild* mChild;
   
public:
   TestParent()
   {
      mChild = NULL;
   }
   
   virtual ~TestParent()
   {
      if(mChild != NULL)
      {
         delete mChild;
      }
   }
   
   virtual TestChild* createChild()
   {
      return new TestChild();
   }
   
   virtual void addChild(TestChild* child)
   {
      if(mChild != NULL)
      {
         delete mChild;
      }
      
      mChild = child;
   }
   
   virtual TestChild* getChild()
   {
      return mChild;
   }
};

class TestChildDataBinding : public DataBinding
{
protected:
   DataMappingFunctor<TestChild> mChildContent;
   DataMappingFunctor<TestChild> mChildId;
   
public:
   TestChildDataBinding(TestChild* c = NULL) :
      DataBinding(c),
      mChildContent(&TestChild::setContent, &TestChild::getContent),
      mChildId(&TestChild::setId, &TestChild::getId)
   {
      // set root data name
      setDataName(NULL, "TestChild");
      
      // add mappings
      addDataMapping(NULL, "id", false, &mChildId);
      addDataMapping(NULL, "TestChild", true, &mChildContent);
   }
   
   virtual ~TestChildDataBinding()
   {
   }
};

class TestParentDataBinding : public DataBinding
{
protected:
   TestParent* mTestParent;
   TestChildDataBinding mChildBinding;
   DataMappingFunctor<TestParent> mTestContent;
   DataMappingFunctor<TestParent, TestChild> mCreateChild;
   
public:
   TestParentDataBinding(TestParent* p) :
      DataBinding(p),
      mTestContent(&TestParent::setContent, &TestParent::getContent),
      mCreateChild(&TestParent::createChild, &TestParent::addChild)
   {
      mTestParent = p;
      
      // set root data name
      setDataName(NULL, "TestContent");
      
      // add mappings
      addDataMapping(NULL, "TestContent", true, &mTestContent);
      addDataMapping(NULL, "TestChild", true, &mCreateChild);
      
      // add bindings
      addDataBinding(NULL, "TestChild", &mChildBinding);
   }
   
   virtual ~TestParentDataBinding()
   {
   }
   
   virtual void getChildren(DataName* dn, list<void*>& children)
   {
      if(mTestParent->getChild() != NULL)
      {
         children.push_back(mTestParent->getChild());
      }
   }
};

void runXmlReaderTest()
{
   cout << "Starting XmlReader test." << endl << endl;
   
   XmlReader reader;
   
//   string xml;
//   xml.append("<Book><Chapter number=\"1\"><Title>Test Chapter 1</Title>");
//   xml.append("<Content>This is the first chapter of the book.</Content>");
//   xml.append("</Chapter><Chapter number=\"2\"/></Book>");
   
   string xml;
   xml.append("<TestContent>This is my content.");
   xml.append("<TestChild id=\"12\">Blah</TestChild></TestContent>");
   
   // main object to populate
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   reader.start(&db);
   reader.read(&bais);
   reader.finish();
   
   cout << "TestContent data='" << p.getContent() << "'" << endl;
   if(p.getChild() != NULL)
   {
      cout << "TestChild data='" << p.getChild()->getContent() << "'" << endl;
      cout << "TestChild id='" << p.getChild()->getId() << "'" << endl;
   }
   else
   {
      cout << "TestChild does not exist!" << endl;
   }
   
   cout << endl << "XmlReader test complete." << endl;
}

void runXmlWriterTest()
{
   cout << "Starting XmlWriter test." << endl << endl;
   
   // main object to populate
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   XmlWriter writer;
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   // write out xml
   writer.write(&db, &os);
   cout << "XML empty=\n" << oss.str() << endl;
   
   // clear string stream
   oss.str("");
   
   // set some content
   p.setContent("Moooooooo");
   
   // add child to TestContent
   TestChild* c = new TestChild();
   c->setId(514);
   p.addChild(c);
   
   writer.write(&db, &os);
   cout << "XML full=\n" << oss.str() << endl;
   
   cout << endl << "XmlWriter test complete." << endl;
}

void runXmlReadWriteTest()
{
   cout << "Starting XmlReadWrite test." << endl << endl;
   
   XmlReader reader;
   
   string xml;
   xml.append("<TestContent>This is my content.");
   xml.append("<TestChild id=\"12\">Blah</TestChild></TestContent>");
   
   // main object to populate
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   ByteArrayInputStream bais(xml.c_str(), xml.length());
   reader.start(&db);
   reader.read(&bais);
   reader.finish();
   
   cout << "*****DOING XML READ*****" << endl;
   
   cout << "TestContent data='" << p.getContent() << "'" << endl;
   if(p.getChild() != NULL)
   {
      cout << "TestChild data='" << p.getChild()->getContent() << "'" << endl;
      cout << "TestChild id='" << p.getChild()->getId() << "'" << endl;
   }
   else
   {
      cout << "TestChild does not exist!" << endl;
   }
   
   cout << endl << "*****DOING XML WRITE*****" << endl;
   
   XmlWriter writer;
   ostringstream oss;
   OStreamOutputStream os(&oss);
   
   // write out xml
   writer.write(&db, &os);
   
   cout << "XML=\n" << oss.str() << endl;
   
   cout << endl << "XmlReadWrite test complete." << endl;
}

void runXmlBindingInputStreamTest()
{
   cout << "Starting XmlBindingInputStream test." << endl << endl;
   
//   string xml1;
//   string xml2;
//   xml1.append("<TestContent>This is the first.");
//   xml2.append("<TestChild id=\"64\">Blah</TestChild> Second.</TestContent>");
//   
//   // main object to populate
//   TestParent p;
//   
//   // data binding for object
//   TestParentDataBinding db(&p);
//   
//   // create output stream for reading from binding
//   XmlBindingInputStream xbos(&db);
//   
//   // write xml to output stream
//   xbos.write(xml1.c_str(), xml1.length());
//   xbos.write(xml2.c_str(), xml2.length());
//   //xbos.write((xml1 + xml2).c_str(), xml1.length() + xml2.length());
//   
//   cout << "TestContent data='" << p.getContent() << "'" << endl;
//   if(p.getChild() != NULL)
//   {
//      cout << "TestChild data='" << p.getChild()->getContent() << "'" << endl;
//      cout << "TestChild id='" << p.getChild()->getId() << "'" << endl;
//   }
//   else
//   {
//      cout << "TestChild does not exist!" << endl;
//   }
   
   cout << endl << "XmlBindingInputStream test complete." << endl;
}

void runXmlBindingOutputStreamTest()
{
   cout << "Starting XmlBindingOutputStream test." << endl << endl;
   
   string xml1;
   string xml2;
   xml1.append("<TestContent>This is the first.");
   xml2.append("<TestChild id=\"64\">Blah</TestChild> Second.</TestContent>");
   
   // main object to populate
   TestParent p;
   
   // data binding for object
   TestParentDataBinding db(&p);
   
   // create output stream for writing to binding
   XmlBindingOutputStream xbos(&db);
   
   // write xml to output stream
   xbos.write(xml1.c_str(), xml1.length());
   xbos.write(xml2.c_str(), xml2.length());
   //xbos.write((xml1 + xml2).c_str(), xml1.length() + xml2.length());
   
   cout << "TestContent data='" << p.getContent() << "'" << endl;
   if(p.getChild() != NULL)
   {
      cout << "TestChild data='" << p.getChild()->getContent() << "'" << endl;
      cout << "TestChild id='" << p.getChild()->getId() << "'" << endl;
   }
   else
   {
      cout << "TestChild does not exist!" << endl;
   }
   
   cout << endl << "XmlBindingOutputStream test complete." << endl;
}

void runBigIntegerTest()
{
   cout << "Starting BigInteger test." << endl << endl;
   
   BigInteger number1 = 2;
   BigInteger number2 = 123456789;
   
   cout << "number1=" << number1 << endl;
   cout << "number2=" << number2 << endl;
   cout << "number1 + number2=" << (number1 + number2) << endl;
   cout << "number1 - number2=" << (number1 - number2) << endl;
   cout << "number1 * number2=" << (number1 * number2) << endl;
   cout << "number2 / number1=" << (number2 / number1) << endl;
   cout << "number2 % number1=" << (number2 % number1) << endl;
   cout << "number2 ^ number1=" << (number2.pow(number1)) << endl;
   
   cout << endl << "BigInteger test complete." << endl;
}

void runBigDecimalTest()
{
   cout << "Starting BigDecimal test." << endl << endl;
   
   BigDecimal number1 = 3.0;
   //BigDecimal number2 = 123456789.5;
   BigDecimal number2 = "123456789.53";
   //BigDecimal number2 = 1.234;
   //BigDecimal number2 = "1.23e-04";
   //BigDecimal number2 = "1234";
      
   cout << "number1=" << number1 << endl;
   cout << "number2=" << number2 << endl;
   cout << "number1 + number2=" << (number1 + number2) << endl;
   cout << "number1 - number2=" << (number1 - number2) << endl;
   cout << "number1 * number2=" << (number1 * number2) << endl;
   cout << "number2 / number1=" << (number2 / number1) << endl;
   cout << "number2 % number1=" << (number2 % number1) << endl;
   
   BigDecimal number3 = "129.54678";
   cout << endl << "number3=" << number3 << endl;
   
   BigDecimal bd;
   
   cout << endl;
   
   for(int i = 7; i >= 0; i--)
   {
      bd = number3;
      bd.setPrecision(i, Up);
      bd.round();
      cout << "round " << i << " places, up=" << bd << endl;
   }
   
   cout << endl;
   
   for(int i = 7; i >= 0; i--)
   {
      bd = number3;
      bd.setPrecision(i, HalfUp);
      bd.round();
      cout << "round " << i << " places, half up=" << bd << endl;
   }
   
   cout << endl;
   
   for(int i = 7; i >= 0; i--)
   {
      bd = number3;
      bd.setPrecision(i, Down);
      bd.round();
      cout << "round " << i << " places, down=" << bd << endl;
   }
   
   cout << endl << "BigDecimal test complete." << endl;
}

void runByteBufferTest()
{
   cout << "Starting ByteBuffer test." << endl << endl;
   
   ByteBuffer b;
   
   char* chicken = "chicken";
   char* t = "T ";
   char* hate = "hate ";
   b.clear();
   b.put(t, strlen(t), true);
   b.put(hate, strlen(hate), true);
   b.put(chicken, strlen(chicken), true);
   b.put("", 1, true);
   
   // FIXME: this test should be more comprehensive
   
   cout << "Data=" << b.data() << endl;
   
   cout << endl << "ByteBuffer test complete." << endl;
}

void runByteArrayOutputStreamTest()
{
   cout << "Starting ByteArrayOutputStream test." << endl << endl;
   
   ByteBuffer b;
   
   ByteArrayOutputStream baos1(&b);
   char* sentence = "This is a sentence.";
   baos1.write(sentence, strlen(sentence) + 1);
   
   cout << "Data1=" << b.data() << endl;
   
   char* chicken = "chicken";
   char* t = "T ";
   char* hate = "hate ";
   b.clear();
   b.put(t, strlen(t), true);
   b.put(hate, strlen(hate), true);
   b.put(chicken, strlen(chicken), true);
   b.put("", 1, true);
   
   cout << "Prior Data2=" << b.data() << endl;
   
   // trim null-terminator
   b.trim(1);
   
   ByteArrayOutputStream baos2(&b);
   baos2.write(sentence, strlen(sentence) + 1);
   
   cout << "Data2=" << b.data() << endl;
   
   cout << endl << "ByteArrayOutputStream test complete." << endl;
}

void runOtherTest()
{
   cout << "Starting Other test." << endl << endl;
   
   // FIXME:
   
   cout << endl << "Other test complete." << endl;
}

class RunTests : public virtual Object, public Runnable
{
public:
   /**
    * Runs the unit tests.
    */
   virtual void run()
   {
      cout << "Tests starting..." << endl << endl;
      
//      runBase64Test();
//      runTimeTest();
//      runThreadTest();
//      runInterruptTest();
//      runJobThreadPoolTest();
//      runJobDispatcherTest();
//      runModestTest();
//      runAddressResolveTest();
//      runSocketTest();
//      runSslSocketTest();
//      runServerSocketTest();
//      runSslServerSocketTest();
//      runTcpClientServerTest();
//      runUdpClientServerTest();
//      runDatagramTest();
//      runMessageDigestTest();
//      runCrcTest();
//      runAsymmetricKeyLoadingTest();
//      runDsaAsymmetricKeyCreationTest();
//      runRsaAsymmetricKeyCreationTest();
//      runDigitalSignatureInputStreamTest();
//      runDigitalSignatureOutputStreamTest();
//      runEnvelopeTest("DSA");
//      runEnvelopeTest("RSA");
//      runCipherTest("AES256");
//      runConvertTest();
//      runUrlEncodeTest();
//      runUrlTest();
//      runRegexTest();
//      runDateTest();
//      runConfigTest();
//      runServerConnectionTest();
//      runServerSslConnectionTest();
//      runServerDatagramTest();
//      runByteBufferTest();
//      runByteArrayInputStreamTest();
//      runByteArrayOutputStreamTest();
//      runStringTokenizerTest();
//      runStringEqualityTest();
//      runStringAppendCharTest();
//      runStringCompareTest();
//      runHttpHeaderTest();
//      runHttpServerTest();
//      runHttpClientGetTest();
//      runHttpClientPostTest();
//      runXmlReaderTest();
//      runXmlWriterTest();
//      runXmlReadWriteTest();
//      runXmlBindingInputStreamTest();
      runXmlBindingOutputStreamTest();
//      runBigIntegerTest();
//      runBigDecimalTest();
      
      cout << endl << "Tests finished." << endl;
      
      if(Thread::hasException())
      {
         Exception* e = Exception::getLast();
         cout << "Exception occurred!" << endl;
         cout << "message: " << e->getMessage() << endl;
         cout << "code: " << e->getCode() << endl;
      }
   }
};

int main()
{
   // initialize winsock
   #ifdef WIN32
      WSADATA wsaData;
      if(WSAStartup(MAKEWORD(2, 0), &wsaData) < 0)
      {
         cout << "ERROR! Could not initialize winsock!" << endl;
      }
   #endif
   
   RunTests runnable;
   Thread t(&runnable);
   t.start();
   t.join();
   
   // cleanup winsock
   #ifdef WIN32
      WSACleanup();
   #endif
   
   Thread::setException(new Exception("Main thread exception leak test"));

   #ifndef WIN32
   // FIXME: calling Thread::exit() on windows causes a busy loop of
   // some sort (perhaps a deadlock spin lock)
   Thread::exit();
   #endif
   
   return 0;
}
