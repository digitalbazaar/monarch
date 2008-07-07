/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>
#include <sstream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/rt/System.h"
#include "db/rt/DynamicObject.h"
#include "db/rt/DynamicObjectIterator.h"
#include "db/net/TcpSocket.h"
#include "db/net/UdpSocket.h"
#include "db/net/DatagramSocket.h"
#include "db/net/Internet6Address.h"
#include "db/net/SslSocket.h"
#include "db/io/File.h"
#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/io/FileList.h"
#include "db/net/Url.h"
#include "db/util/Date.h"
#include "db/util/StringTools.h"
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
#include "db/io/ByteArrayInputStream.h"

using namespace std;
using namespace db::test;
using namespace db::io;
using namespace db::modest;
using namespace db::net;
using namespace db::net::http;
using namespace db::rt;
using namespace db::util;

void runAddressResolveTest(TestRunner& tr)
{
   tr.test("Address Resolution");
   
   //cout << "Running Address Resolve Test" << endl << endl;
   
   Exception::clearLast();
   
   // create IPv4 address
   InternetAddress ip4;
   
   //cout << "Testing IPv4..." << endl << endl;
   
   ip4.setHost("www.bitmunk.com");
   ip4.getAddress();
   assertNoException();
   //cout << "www.bitmunk.com = " << ip4.getAddress() << endl;
   
   ip4.setHost("www.google.com");
   ip4.getAddress();
   assertNoException();
   //cout << "www.google.com = " << ip4.getAddress() << endl;
   
   ip4.setHost("www.yahoo.com");
   ip4.getAddress();
   assertNoException();
   //cout << "www.yahoo.com = " << ip4.getAddress() << endl;
   
   ip4.setHost("www.microsoft.com");
   ip4.getAddress();
   assertNoException();
   //cout << "www.microsoft.com = " << ip4.getAddress() << endl;
   
   //cout << endl;
   
   ip4.setAddress("192.168.0.1");
   ip4.getAddress();
   ip4.getHost();
   assertNoException();
   //cout << ip4.getAddress() << " = " << ip4.getHost() << endl;
   
   ip4.setAddress("192.168.0.8");
   ip4.getAddress();
   ip4.getHost();
   assertNoException();
   //cout << ip4.getAddress() << " = " << ip4.getHost() << endl;
   
   ip4.setAddress("216.239.51.99");
   ip4.getAddress();
   ip4.getHost();
   assertNoException();
   //cout << ip4.getAddress() << " = " << ip4.getHost() << endl;
   
//   // create IPv6 address
//   Internet6Address ip6;
//   
//   cout << endl << "Testing IPv6..." << endl << endl;
//   
//   ip6.setHost("ip6-localhost");
//   cout << "ip6-localhost = " << ip6.getAddress() << endl;
//   
//   ip6.setHost("yuna.digitalbazaar.com");
//   cout << "yuna.digitalbazaar.com = " << ip6.getAddress() << endl;
//   
//   ip6.setHost("www.google.com");
//   cout << "www.google.com = " << ip6.getAddress() << endl;
//   
//   ip6.setHost("www.yahoo.com");
//   cout << "www.yahoo.com = " << ip6.getAddress() << endl;
//   
//   ip6.setHost("www.microsoft.com");
//   cout << "www.microsoft.com = " << ip6.getAddress() << endl;
//   
//   cout << endl;
//   
//   ip6.setAddress("fc00:840:db:bb:d::8");
//   cout << ip6.getAddress() << " = " << ip6.getHost() << endl;
   
   tr.passIfNoException();
   
   //cout << endl << "Address Resolve Test complete." << endl << endl;
}

void runSocketTest(TestRunner& tr)
{
   tr.test("Socket");
   
   //cout << "Running Socket Test" << endl << endl;
   Exception::clearLast();
   
   // create address
   //InternetAddress address("127.0.0.1", 80);
   InternetAddress address("www.google.com", 80);
   
   // ensure host was known
   assertNoException();
   
   address.getAddress();
   assertNoException();
   //cout << "Connecting to: " << address.getAddress() << endl;
   
   // create tcp socket
   TcpSocket socket;
   
   // connect
   socket.connect(&address);
   assertNoException();
   
   char request[] =
      "GET / HTTP/1.0\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
   socket.send(request, sizeof(request));
   assertNoException();
   
   // set receive timeout (10 seconds = 10000 milliseconds)
   socket.setReceiveTimeout(10000);
   assertNoException();
   
   char response[2048];
   int numBytes = 0;
   string str;
   
   //cout << endl << "DOING A PEEK!" << endl;
   
   string peek;
   numBytes = socket.getInputStream()->peek(response, 2048);
   if(numBytes > 0)
   {
      //cout << "Peeked " << numBytes << " bytes." << endl;
      peek.append(response, numBytes);
      //cout << "Peek bytes=" << peek << endl;
   }
   assertNoException();
   
   //cout << endl << "DOING ACTUAL READ NOW!" << endl;
   int peekBytes = numBytes;
   while((numBytes = socket.getInputStream()->read(response, 2048)) > 0)
   {
      //cout << "numBytes received: " << numBytes << endl;
      str.append(response, numBytes);
   }
   
   // confirm peek bytes check out
   assert(strncmp(peek.c_str(), str.c_str(), peekBytes) == 0);
   
//   char response[2048];
//   int numBytes = 0;
//   string str = "";
//   while((numBytes = socket.receive(response, 0, 2048)) > 0)
//   {
//      cout << "numBytes received: " << numBytes << endl;
//      str.append(response, numBytes);
//   }
   
   //cout << "Response:" << endl << str << endl;
   
   // close
   socket.close();
   
   tr.passIfNoException();
   
   //cout << "Socket connection closed." << endl;
   
   //cout << endl << "Socket test complete." << endl;
}

void runSslSocketTest()
{
   cout << "Running SSL Socket Test" << endl << endl;
   
   // FIXME:
   // seed PRNG
   
   // create address
   InternetAddress address("127.0.0.1", 443);
   //InternetAddress address("127.0.0.1", 19020);
   //InternetAddress address("www.google.com", 80);
   cout << address.getAddress() << endl;
   
   // ensure host was known
   if(!Exception::hasLast())
   {
      // create tcp socket
      TcpSocket socket;
      
      // connect
      socket.connect(&address);
      
      // create an SSL context
      SslContext context(NULL, true);
      
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
}

void runServerSocketTest()
{
   //cout << "Running Server Socket Test" << endl << endl;
   
   Exception::clearLast();
   
   // bind and listen
   InternetAddress address("127.0.0.1", 19100);
   
   // ensure host was known
   if(!Exception::hasLast())
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
   
   // bind and listen
   InternetAddress address("127.0.0.1", 1024);
   
   // ensure host was known
   if(!Exception::hasLast())
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
         SslContext context(NULL, false);
         
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
}

void runTcpClientServerTest()
{
   cout << "Running TCP Client/Server Test" << endl << endl;
   
   InternetAddress* address;
   InternetAddress ia("127.0.0.1", 9999);
   //Internet6Address ia("::0", 9999);
   address = &ia;
   
   // ensure host was known
   if(!Exception::hasLast())
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
   if(!Exception::hasLast())
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
   if(!Exception::hasLast())
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

void runUrlEncodeTest(TestRunner& tr)
{
   tr.test("Url Encode/Decode");
   
   string str = "billy bob & \"jane\" +^%2{13.";
   
   string encoded = Url::encode(str.c_str(), str.length());
   string decoded = Url::decode(encoded.c_str(), encoded.length());
   
   //cout << "test data=" << str << endl;
   
   //cout << "url encoded=" << encoded << endl;
   //cout << "url decoded=" << decoded << endl;
   
   assert(decoded == str);
   
   tr.pass();
}

void dumpUrl(Url url)
{
   if(Exception::hasLast())
   {
      cout << "url=[exception]" << endl;
   }
   else
   {
      string str = url.toString();
      
      cout << "url=" << str << endl;
      cout << " scheme=" << url.getScheme() << endl;
      cout << " scheme specific part=" << url.getSchemeSpecificPart() << endl;
      cout << " authority=" << url.getAuthority() << endl;
      cout << " userinfo=" << url.getUserInfo() << endl;
      cout << " user=" << url.getUser() << endl;
      cout << " password=" << url.getPassword() << endl;
      cout << " host=" << url.getHost() << endl;
      cout << " port=" << url.getPort() << endl;
      cout << " path=" << url.getPath() << endl;
      cout << " query=" << url.getQuery() << endl;
   }
}

void runUrlTest(TestRunner& tr)
{
   tr.test("Url");

   {
      Url url("http:");
      
      //dumpUrl(url);
      assert(url.getScheme() == "http");
      assert(url.getSchemeSpecificPart() == "");
   }
   
   {
      Url url("http://");
      
      //dumpUrl(url);
      assert(url.getScheme() == "http");
      assert(url.getSchemeSpecificPart() == "//");
   }
   
   {
      Url url("http://www.bitmunk.com");
      
      //dumpUrl(url);
      assert(url.getScheme() == "http");
      assert(url.getSchemeSpecificPart() == "//www.bitmunk.com");
      assert(url.getHost() == "www.bitmunk.com");
      assert(url.getPath() == "/");
   }
   
   {
      Url url("http://www.bitmunk.com/mypath?variable1=test");
      
      //dumpUrl(url);
      assert(url.getScheme() == "http");
      assert(url.getUserInfo() == "");
      assert(url.getUser() == "");
      assert(url.getPassword() == "");
      assert(url.getHost() == "www.bitmunk.com");
      assert(url.getPort() == 80);
      assert(url.getPath() == "/mypath");
      assert(url.getQuery() == "variable1=test");
   }
   
   {
      Url url("mysql://username:password@host:3306/mydatabase");
      
      //dumpUrl(url);
      assert(url.getScheme() == "mysql");
      assert(url.getUser() == "username");
      assert(url.getPassword() == "password");
      assert(url.getHost() == "host");
      assert(url.getPort() == 3306);
      assert(url.getPath() == "/mydatabase");
   }
   
   {
      Url url("http://example.com:8080/path");

      //dumpUrl(url);
      assert(!Exception::hasLast());
      assert(url.getScheme() == "http");
      assert(url.getUserInfo() == "");
      assert(url.getUser() == "");
      assert(url.getPassword() == "");
      assert(url.getHost() == "example.com");
      assert(url.getPort() == 8080);
      assert(url.getPath() == "/path");
      assert(url.getQuery() == "");
   }
   
   {   
      Url url("scheme:schemespecific");

      //dumpUrl(url);
      assert(!Exception::hasLast());
      assert(url.getScheme() == "scheme");
      assert(url.getSchemeSpecificPart() == "schemespecific");
   }
   
   {
      Url url(
         "scheme://user:password@host:1234/path?key1=value1&key2=value2"
         "&key3=two%20words%3D2");

      //dumpUrl(url);
      assert(!Exception::hasLast());
      assert(url.getScheme() == "scheme");
      assert(url.getUserInfo() == "user:password");
      assert(url.getUser() == "user");
      assert(url.getPassword() == "password");
      assert(url.getHost() == "host");
      assert(url.getPort() == 1234);
      assert(url.getPath() == "/path");
      assert(url.getQuery() == "key1=value1&key2=value2&key3=two%20words%3D2");
      
      DynamicObject vars;
      assert(url.getQueryVariables(vars));
      assert(strcmp(vars["key1"]->getString(), "value1") == 0);
      assert(strcmp(vars["key2"]->getString(), "value2") == 0);
      assert(strcmp(vars["key3"]->getString(), "two words=2") == 0);
   }
   
   {
      Url url(
         "/path/param1/10001?key1=value1&key2=value2&key3=two%20words%3D2",
         true);
      
      //dumpUrl(url);
      assert(!Exception::hasLast());
      assert(url.getPath() == "/path/param1/10001");
      assert(url.getQuery() == "key1=value1&key2=value2&key3=two%20words%3D2");
      
      DynamicObject tokens;
      assert(url.getTokenizedPath(tokens, "/path/"));
      assert(strcmp(tokens[0]->getString(), "param1") == 0);
      assert(tokens[1]->getInt32() == 10001);
      
      DynamicObject vars;
      assert(url.getQueryVariables(vars));
      assert(strcmp(vars["key1"]->getString(), "value1") == 0);
      assert(strcmp(vars["key2"]->getString(), "value2") == 0);
      assert(strcmp(vars["key3"]->getString(), "two words=2") == 0);
   }
   
   {
      Url url("http://bitmunk.com/path?email=wa-hoo.test_user%40bitmunk.com");
      
      //dumpUrl(url);
      assert(!Exception::hasLast());
      assert(url.getPath() == "/path");
      assert(url.getQuery() == "email=wa-hoo.test_user%40bitmunk.com");
      
      DynamicObject vars;
      assert(url.getQueryVariables(vars));
      assertStrCmp(vars["email"]->getString(), "wa-hoo.test_user@bitmunk.com");
   }
   
   tr.pass();
}

class InterruptServerSocketTest : public virtual Object, public Runnable
{
public:
   /**
    * Runs the unit tests.
    */
   virtual void run()
   {
      runServerSocketTest();
      
      if(Exception::hasLast())
      {
         ExceptionRef e = Exception::getLast();
         cout << "Exception occurred!" << endl;
         cout << "message: " << e->getMessage() << endl;
         cout << "type: " << e->getType() << endl;
         cout << "code: " << e->getCode() << endl;
      }      
   }
};

void runInterruptServerSocketTest(TestRunner& tr)
{
   tr.test("Thread Interrupt");
   
   InterruptServerSocketTest runnable;
   Thread t(&runnable);
   t.start(131072);
   
   //cout << "Waiting for thread..." << endl;
   Thread::sleep(2000);
   //cout << "Finished waiting for thread." << endl;
   
   //cout << "Interrupting thread..." << endl;
   t.interrupt();
   
   //cout << "Joining thread..." << endl;
   t.join();
   //cout << "Thread joined." << endl;
   
   tr.pass();
}

class TestConnectionServicer1 : public ConnectionServicer
{
public:
   unsigned int serviced;
   string reply;
   
   TestConnectionServicer1()
   {
      serviced = 0;
      reply =
         "HTTP/1.0 200 OK\r\n"
         "Content-Length: 5\r\n"
         "Content-Type: text/plain\r\n"
         "\r\n"
         "Hello";
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
//         cout << "Server read " << numBytes << " bytes." << endl;
//         string str;
//         str.append(b, numBytes);
//         cout << "DATA=" << endl << str << endl;
      }
      else if(numBytes == -1)
      {
         cout << "Server Exception=" <<
            Exception::getLast()->getMessage() << endl <<
            Exception::getLast()->getType() << endl;
      }
      
      OutputStream* os = c->getOutputStream();
      if(os->write(reply.c_str(), reply.length()))
      {
//         cout << "Server sent=" << endl << reply << endl;
      }
      else
      {
         cout << "Server Exception=" <<
            Exception::getLast()->getMessage() << endl <<
            Exception::getLast()->getType() << endl;
      }
      
//      cout << "1: Finished servicing connection." << endl;
      
      serviced++;
//      cout << "Connections serviced=" << serviced << endl;
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

class BlastConnections : public Runnable
{
public:
   InternetAddress* address;
   bool ssl;
   
   BlastConnections(InternetAddress* a, bool ssl)
   {
      this->address = a;
      this->ssl = ssl;
   }
   
   virtual ~BlastConnections()
   {
   }
   
   void run()
   {
      //Thread::sleep(20000);
      
      // create ssl context
      SslContext context("TLS", true);
      //File certFile("/we/need/to/add/this/here.ca");
      //context.setVerifyCAs(&certFile, NULL);
      assertNoException();
      
      // for storing SSL session
      SslSession session;
      
      // blast connections
      int connections = 50;
      char b[1024];
      string request =
         "GET / HTTP/1.0\r\n"
         "Content-Length: 0\r\n"
         "Connection: close\r\n"
         "\r\n";
      for(int i = 0; i < connections; i++)
      {
         // create socket
         Socket* socket = new TcpSocket();
         socket->setReceiveTimeout(1000);
         
         // connect
         if(socket->connect(address))
         {
            //cout << "connected" << endl;
            
            // do ssl if appropriate
            if(ssl)
            {
               // reuse session
               socket = new SslSocket(&context, (TcpSocket*)socket, true, true);
               ((SslSocket*)socket)->setSession(&session);
               
               // start session
               ((SslSocket*)socket)->performHandshake();
               
               // store session
               session = ((SslSocket*)socket)->getSession();
            }
            
            // send request
            if(socket->send(request.c_str(), request.length()))
            {
               // receive response
               socket->receive(b, 1024);
//               int numBytes = socket->receive(b, 1024);
//               cout << "Client read " << numBytes << "bytes." << endl;
//               cout << "DATA=";
//               cout.write(b, numBytes);
//               cout << endl;
            }
            else
            {
               cout << "Client Exception=" <<
                  Exception::getLast()->getMessage() << endl <<
                  Exception::getLast()->getType() << endl;
            }
         }
         else
         {
            cout << "Client Exception=" <<
               Exception::getLast()->getMessage() << endl <<
               Exception::getLast()->getType() << endl;
         }
         
         // close socket
         socket->close();
         
         // clean up socket
         delete socket;
      }
   }
};

void runServerSslConnectionTest()
{
   cout << "Starting Server SSL Connection test." << endl << endl;
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("localhost", 19100);
   
   // set up SSL context
   SslContext context("TLS", false);
   File certFile("/etc/apache2/ssl/www.bitmunk.com.crt");
   File pkeyFile("/etc/apache2/ssl/www.bitmunk.com.key");
   context.setCertificate(certFile);
   context.setPrivateKey(pkeyFile);
   
   // create SSL-only service
   TestConnectionServicer1 tcs1;
   SslSocketDataPresenter presenter(&context);
   server.addConnectionService(&address, &tcs1, &presenter);
   
//   // create SSL/generic service
//   TestConnectionServicer1 tcs1;
//   SslSocketDataPresenter presenter1(&context);
//   NullSocketDataPresenter presenter2;
//   SocketDataPresenterList list(false);
//   list.add(&presenter1);
//   list.add(&presenter2);
//   server.addConnectionService(&address, &tcs1, &list);
   
   if(server.start())
   {
      cout << "Server started." << endl;
   }
   else if(Exception::getLast() != NULL)
   {
      cout << "Server started with errors=" <<
         Exception::getLast()->getMessage() << endl;
   }
   
   BlastConnections bc(&address, true);
   Thread t1(&bc);
   Thread t2(&bc);
   Thread t3(&bc);
   Thread t4(&bc);
   Thread t5(&bc);
   Thread t6(&bc);
   Thread t7(&bc);
   Thread t8(&bc);
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   size_t stackSize = 131072;
   t1.start(stackSize);
   //t2.start(stackSize);
//   t3.start(stackSize);
//   t4.start(stackSize);
//   t5.start(stackSize);
//   t6.start(stackSize);
//   t7.start(stackSize);
//   t8.start(stackSize);
   
   t1.join();
   //t2.join();
//   t3.join();
//   t4.join();
//   t5.join();
//   t6.join();
//   t7.join();
//   t8.join();
//   cout << "all client threads joined." << endl;
   
   unsigned long long end = System::getCurrentMilliseconds();
   
   // stop server
   server.stop();
   cout << "Server stopped." << endl;
   
   // stop kernel engine
   k.getEngine()->stop();
   
   long double time = end - start;
   long double secs = time / 1000.0;
   double rate = (double)tcs1.serviced / secs;
   
   cout << "Connections=" << tcs1.serviced << endl;
   cout << "Time=" << time << " ms = " << secs << " secs" << endl;
   cout << "Time/Connection=" << (double)time / tcs1.serviced << " ms" << endl;
   cout << "Connections/second=" << rate << endl;
   
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

void runHttpHeaderTest(TestRunner& tr)
{
   tr.test("HttpHeader");
   
   // test bicapitalization of http headers
   char test[] = "ThIs-a-BICaPitAlized-hEADer";
   HttpHeader::biCapitalize(test);
   
   //cout << "BiCapitalized Header=" << test << endl;
   assertStrCmp(test, "This-A-Bicapitalized-Header");
   
//   string t = "   d  f  ";
//   StringTools::trim(t);
//   cout << "t='" << t << "'" << endl;
   
   //cout << endl << "Request Header:" << endl;
   
   HttpRequestHeader reqHeader;
   reqHeader.setDate();
   reqHeader.setMethod("GET");
   reqHeader.setPath("/");
   reqHeader.setVersion("HTTP/1.1");
   reqHeader.setField("host", "localhost:80");
   reqHeader.setField("Content-Type", "text/html");
   reqHeader.setField("Connection", "close");
   
   //const char* expect =
   //   "GET / HTTP/1.1\r\n"
   //   "Connection: close\r\n"
   //   "Content-Type: text/html\r\n"
   //   "Host: localhost:80\r\n"
   //   "\r\n";
   
   string str = reqHeader.toString();
   //assertStrCmp(str.c_str(), expect);
   //cout << str;
   tr.warning("fix http request parse test");
   
   //cout << "End of Request Header." << endl;
   
   //cout << endl << "Parsed Request Header:" << endl;
   
   HttpRequestHeader reqHeader2;
   reqHeader2.parse(str);
   
   string str2 = reqHeader2.toString();
   //assertStrCmp(str2.c_str(), expect);
   tr.warning("fix http request parse test");
   //cout << str2;
   
   //cout << "End of Parsed Request Header." << endl;

   //cout << endl << "Response Header:" << endl;
   
   HttpResponseHeader resHeader;
   resHeader.setDate();
   resHeader.setVersion("HTTP/1.1");
   resHeader.setStatus(404, "Not Found");
   resHeader.setField("host", "localhost:80");
   resHeader.setField("Content-Type", "text/html");
   resHeader.setField("Connection", "close");
   
   str = resHeader.toString();
   tr.warning("fix http response parse test");
   //cout << str;
   
   //cout << "End of Response Header." << endl;
   
   //cout << endl << "Parsed Response Header:" << endl;
   
   HttpResponseHeader resHeader2;
   resHeader2.parse(str);
   tr.warning("fix http response parse test");
   
   str2 = resHeader2.toString();
   tr.warning("fix http response parse test");
   //cout << str2;
   
   //cout << "End of Parsed Response Header." << endl;
   
   tr.passIfNoException();
}

void runHttpNormalizePath(TestRunner& tr)
{
   tr.test("Http normalize path");
   
   char temp[100];
   
   // no starting slash
   {
      HttpRequestServicer::normalizePath("a/b/c", temp);
      assertStrCmp(temp, "/a/b/c");
   }
   
   // extra ending slash
   {
      HttpRequestServicer::normalizePath("/a/b/c/", temp);
      assertStrCmp(temp, "/a/b/c");
   }
   
   // no starting slash, extra ending slash
   {
      HttpRequestServicer::normalizePath("a/b/c/", temp);
      assertStrCmp(temp, "/a/b/c");
   }
   
   // extra middle slashes
   {
      HttpRequestServicer::normalizePath("/a//b//c/", temp);
      assertStrCmp(temp, "/a/b/c");
   }
   
   // crazy
   {
      HttpRequestServicer::normalizePath("a///b///////c////", temp);
      assertStrCmp(temp, "/a/b/c");
   }
   
   // crazy
   {
      HttpRequestServicer::normalizePath("////a///b///////c////", temp);
      assertStrCmp(temp, "/a/b/c");
   }
   
   // crazy
   {
      HttpRequestServicer::normalizePath("/a///b///////c////", temp);
      assertStrCmp(temp, "/a/b/c");
   }
   
   // crazy
   {
      HttpRequestServicer::normalizePath("woof///moo///////meow////", temp);
      assertStrCmp(temp, "/woof/moo/meow");
   }
   
   tr.pass();
}

class TestHttpRequestServicer : public HttpRequestServicer
{
public:
   const char* content;
   
   TestHttpRequestServicer(const char* path) : HttpRequestServicer(path)
   {
      content = "Bob Loblaw's Law Blog";
   }
   
   virtual ~TestHttpRequestServicer()
   {
   }
   
   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
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
   
   // create kernel
   Kernel k;
   
   // set thread stack size in engine (128k)
   k.getEngine()->getThreadPool()->setThreadStackSize(131072);
   
   // optional for testing --
   // limit threads to 2: one for accepting, 1 for handling
   //k.getEngine()->getThreadPool()->setPoolSize(2);
   
   // start engine
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
      cout << "Connected to: " << url.toString() << endl;
      InternetAddress address(url.getHost().c_str(), url.getPort());
      cout << address.toString() << endl;
      
      // do get
      const char* headers[] = {"Test-Header: bacon", NULL};
      HttpResponse* response = client.get(&url, headers);
      if(response != NULL)
      {
         cout << "Response=" << endl <<
            response->getHeader()->toString() << endl;
         if(response->getHeader()->getStatusCode() == 200)
         {
            // receive content
            HttpTrailer trailer;
            File file("/tmp/index.html");
            FileOutputStream fos(file);
            if(client.receiveContent(&fos, &trailer))
            {
               cout << "Content downloaded to '" <<
                  file->getName() << "'" << endl;
               
               cout << "HTTP trailers=\n" << trailer.toString() << endl;
            }
            assertNoException();
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
      cout << "Connected to: " << url.toString() << endl;
      InternetAddress address(url.getHost().c_str(), url.getPort());
      cout << address.toString() << endl;
      
      char someData[] = "Just some post data.";
      ByteArrayInputStream baos(someData, strlen(someData));
      
      // do post
      const char* headers[] = {
         "Content-Type: text/plain",
         "Transfer-Encoding: chunked",
         NULL};
      
      HttpTrailer trailer;
      HttpResponse* response = client.post(&url, headers, &baos, &trailer);
      if(response != NULL)
      {
         cout << "Response=" << endl <<
            response->getHeader()->toString() << endl;
         if(response->getHeader()->getStatusCode() == 200)
         {
            // receive content
            trailer.clearFields();
            File file("/tmp/postresponse.txt");
            FileOutputStream fos(file);
            if(client.receiveContent(&fos, &trailer))
            {
               cout << "Content downloaded to '" <<
                  file->getName() << "'" << endl;
               
               cout << "HTTP trailers=\n" << trailer.toString() << endl;
            }
            assertNoException();
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

class PingHttpRequestServicer : public HttpRequestServicer
{
public:
   const char* content;
   
   PingHttpRequestServicer(const char* path) : HttpRequestServicer(path)
   {
      content = "Bob Loblaw's Law Blog";
   }
   
   virtual ~PingHttpRequestServicer()
   {
   }
   
   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
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

class PingConnectionServicer : public ConnectionServicer
{
public:
   unsigned long long start;
   unsigned long long end;
   unsigned int serviced;
   
   PingConnectionServicer()
   {
      serviced = 0;
      start = 0;
      end = 0;
   }
   
   virtual ~PingConnectionServicer() {}
   
   void serviceConnection(Connection* c)
   {
      if(start == 0)
      {
         start = System::getCurrentMilliseconds();
      }
      
      serviced++;
      end = System::getCurrentMilliseconds();
   }
};

void runPingTest()
{
   cout << "Starting Ping test." << endl << endl;
   
   // create kernel
   Kernel k;
   k.getEngine()->start();
   
   // create server
   Server server(&k);
   InternetAddress address("localhost", 19100);
   
//   // create SSL/generic ping connection servicer
//   PingConnectionServicer pcs;
////   SslContext context;
////   SslSocketDataPresenter presenter1(&context);
////   NullSocketDataPresenter presenter2;
////   SocketDataPresenterList list(false);
////   list.add(&presenter1);
////   list.add(&presenter2);
//   server.addConnectionService(&address, &pcs);//, &list);

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
   PingHttpRequestServicer test1("/test");
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
   
   // connect
   Url url("http://localhost:19100");
   HttpTrailer trailer;
   File file("/tmp/index.html");
   FileOutputStream fos(file);
   HttpClient client;
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   client.connect(&url);
   client.get(&url, NULL);
   client.receiveContent(&fos, &trailer);
   
   unsigned long long end = System::getCurrentMilliseconds();
   
   client.disconnect();
   
   // sleep
   //Thread::sleep(10000);
   
   server.stop();
   cout << "Server stopped." << endl;
   
   // stop kernel engine
   k.getEngine()->stop();
   
   unsigned long long millis = end - start;
   cout << "Connection Time: " << millis << endl;
   
//   unsigned long long millis = test1.end - test1.start;
//   long double cps = ((long double)pcs.serviced) / millis * 1000.0;
//   cout << "Connections serviced: " << pcs.serviced << endl;
//   cout << "Time: " << millis << endl;
//   cout << "Connections/Second: " << cps << endl;
   
   cout << endl << "Ping test complete." << endl;
}

class DbNetTester : public db::test::Tester
{
public:
   DbNetTester()
   {
      setName("net");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      /*
      runAddressResolveTest(tr);
      runSocketTest(tr);
      runUrlEncodeTest(tr);
      runUrlTest(tr);
      runHttpHeaderTest(tr);
      runHttpNormalizePath(tr);
      */
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
//      runInterruptServerSocketTest(tr);
//      runSslSocketTest();
//      runServerSocketTest();
//      runSslServerSocketTest();
//      runTcpClientServerTest();
//      runUdpClientServerTest();
//      runDatagramTest();
//      runServerConnectionTest();
//      runServerSslConnectionTest();
//      runServerDatagramTest();
//      runHttpNormalizePath(tr);
      runHttpServerTest();
//      runHttpClientGetTest();
//      runHttpClientPostTest();
//      runPingTest();
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbNetTester)
#endif
