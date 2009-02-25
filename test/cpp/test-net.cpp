/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
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
#include "db/net/http/CookieJar.h"
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
   
   string strAddress = ip4.toString();
   InternetAddress another;
   another.fromString(strAddress.c_str());
   assertStrCmp(another.toString().c_str(), strAddress.c_str());
   
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

void runUdpClientServerTest(TestRunner& tr)
{
   tr.test("UDP Client/Server");
   {
      InternetAddress* sa;
      InternetAddress* ca;
      InternetAddress serverAddress("127.0.0.1", 0);
      InternetAddress clientAddress("127.0.0.1", 0);
      //Internet6Address serverAddress("::1", 0);
      //Internet6Address clientAddress("::1", 0);
      sa = &serverAddress;
      ca = &clientAddress;
      
      // create udp server and client sockets
      UdpSocket server;
      UdpSocket client;
      
      // set receive timeouts to 2 seconds
      server.setReceiveTimeout(2000);
      client.setReceiveTimeout(2000);
      
      // bind with server
      server.bind(sa);
      assertNoException();
      
      //printf("Server bound at host: %s\n", sa->getHost());
      //printf("Server bound at address: %s\n", sa->getAddress());
      //printf("Server bound on port: %i\n", sa->getPort());
      
      // bind with client
      client.bind(ca);
      assertNoException();
      
      //printf("Client bound at host: %s\n", ca->getHost());
      //printf("Client bound at address: %s\n", ca->getAddress());
      //printf("Client bound on port: %i\n", ca->getPort());
      
      // send some data with client
      string clientData = "Hello there, Server.";
      client.sendDatagram(clientData.c_str(), clientData.length(), sa);
      assertNoException();
      
      //printf("Client sent: %s\n", clientData.c_str());
      
      // receive the client data
      char read[2048];
      int numBytes = server.receiveDatagram(read, clientData.length(), ca);
      string serverReceived(read, numBytes);
      assertNoException();
      
      //printf("Server received: %s\n", serverReceived.c_str());
      //printf("Data from: %s:%i\n", ca->getAddress(), ca->getPort());
      assertStrCmp(clientData.c_str(), serverReceived.c_str());
      
      // send some data with server
      string serverData = "G'day, Client.";
      server.sendDatagram(serverData.c_str(), serverData.length(), ca);
      assertNoException();
      
      //printf("Server sent: %s\n", serverData.c_str());
      
      // receive the server data
      numBytes = client.receiveDatagram(read, serverData.length(), sa);
      string clientReceived(read, numBytes);
      
      //printf("Client received: %s\n", clientReceived.c_str());
      //printf("Data from: %s:%i\n", sa->getAddress(), sa->getPort());
      assertStrCmp(serverData.c_str(), clientReceived.c_str());
      
      // close sockets
      client.close();
      server.close();
      
      //printf("Sockets closed.\n");
   }
   tr.passIfNoException();
}

void runDatagramTest(TestRunner& tr)
{
   tr.group("Datagram");
   
   tr.test("unicast");
   {
      InternetAddressRef sa;
      InternetAddressRef ca;
      InternetAddressRef serverAddress = new InternetAddress("127.0.0.1", 0);
      InternetAddressRef clientAddress = new InternetAddress("127.0.0.1", 0);
      //InternetAddressRef serverAddress = new Internet6Address("::1", 0);
      //InternetAddressRef clientAddress = new Internet6Address("::1", 0);
      sa = serverAddress;
      ca = clientAddress;
      
      // create datagram server and client sockets
      DatagramSocket server;
      DatagramSocket client;
      
      // set receive timeouts to 2 seconds
      server.setReceiveTimeout(2000);
      client.setReceiveTimeout(2000);
      
      // bind with server
      server.bind(&(*sa));
      assertNoException();
      
      //printf("Server bound at host: %s\n", sa->getHost());
      //printf("Server bound at address: %s\n", sa->getAddress());
      //printf("Server bound on port: %i\n", sa->getPort());
      
      // bind with client
      client.bind(&(*ca));
      assertNoException();
      
      //printf("Client bound at host: %s\n", ca->getHost());
      //printf("Client bound at address: %s\n", ca->getAddress());
      //printf("Client bound on port: %i\n", ca->getPort());
      
      // create a datagram
      DatagramRef d1 = new Datagram(sa);
      d1->assignString("Hello there, Server.");
      
      // send the datagram with the client
      client.send(d1);
      assertNoException();
      
      //printf("Client sent: %s\n", d1->getString().c_str());
      
      // create a datagram
      InternetAddressRef ia = new InternetAddress();
      char externalData[2048];
      DatagramRef d2 = new Datagram(ia);
      d2->getBuffer()->setBytes(externalData, 0, 0, 2048, false);
      
      // receive a datagram
      server.receive(d2);
      assertNoException();
      
      //printf("Server received: %s\n", d2->getString().c_str());
      //printf("Data from: %s:%i\n",
      //   d2->getAddress()->getAddress(), d2->getAddress()->getPort());
      assertStrCmp(
         d2->getAddress()->toString().c_str(), ca->toString().c_str());
      assertStrCmp(d2->getString().c_str(), d1->getString().c_str());
      
      // send a datagram with the server
      d2->assignString("G'day, Client.");
      d2->setAddress(ca);
      server.send(d2);
      assertNoException();
      
      //printf("Server sent: %s\n", d2->getString().c_str());
      
      // receive the server datagram
      ia = new InternetAddress();
      DatagramRef d3 = new Datagram(ia, d2->getString().length());
      client.receive(d3);
      assertNoException();
      
      //printf("Client received: %s\n", d3->getString().c_str());
      //printf("Data from: %s:%i\n",
      //   d3->getAddress()->getAddress(), d3->getAddress()->getPort());
      assertStrCmp(
         d3->getAddress()->toString().c_str(), sa->toString().c_str());
      assertStrCmp(
         d3->getString().c_str(), d2->getString().c_str());
      
      // close sockets
      client.close();
      server.close();
      
      //printf("Sockets closed.\n");
   }
   
   tr.passIfNoException();
   
   tr.test("multicast");
   {
      InternetAddressRef sa;
      InternetAddressRef ca;
      InternetAddressRef ga;
      InternetAddressRef serverAddress = new InternetAddress("0.0.0.0", 12345);
      InternetAddressRef clientAddress = new InternetAddress("0.0.0.0", 0);
      InternetAddressRef groupAddress = new InternetAddress("225.0.0.1", 12345);
      //InternetAddressRef serverAddress = new Internet6Address("::1", 0);
      //InternetAddressRef clientAddress = new Internet6Address("::1", 0);
      //InternetAddressRef groupAddress = new Internet6Address("FIXME", 0);
      sa = serverAddress;
      ca = clientAddress;
      ga = groupAddress;
      
      // create datagram server and client sockets
      DatagramSocket server;
      DatagramSocket client;
      
      // set receive timeouts to 2 seconds
      server.setReceiveTimeout(2000);
      client.setReceiveTimeout(2000);
      
      // bind with server
      server.bind(&(*sa));
      assertNoException();
      
      //printf("Server bound at host: %s\n", sa->getHost());
      //printf("Server bound at address: %s\n", sa->getAddress());
      //printf("Server bound on port: %i\n", sa->getPort());
      
      // joing group with server
      server.joinGroup(&(*groupAddress), &(*sa));
      assertNoException();
      
      // bind with client
      client.bind(&(*ca));
      assertNoException();
      
      //printf("Client bound at host: %s\n", ca->getHost());
      //printf("Client bound at address: %s\n", ca->getAddress());
      //printf("Client bound on port: %i\n", ca->getPort());
      
      // create a datagram
      DatagramRef d1 = new Datagram(ga);
      d1->assignString("Hello there, everyone.");
      
      // send the datagram to the group with the client
      client.send(d1);
      assertNoException();
      
      //printf("Client sent: %s\n", d1->getString().c_str());
      
      // create a datagram
      InternetAddressRef ia = new InternetAddress();
      char externalData[2048];
      DatagramRef d2 = new Datagram(ia);
      d2->getBuffer()->setBytes(externalData, 0, 0, 2048, false);
      
      // receive a datagram
      server.receive(d2);
      assertNoException();
      
      //printf("Server received: %s\n", d2->getString().c_str());
      //printf("Data from: %s:%i\n",
      //   d2->getAddress()->getAddress(), d2->getAddress()->getPort());
      //assertStrCmp(
      //   d2->getAddress()->toString().c_str(), ca->toString().c_str());
      assertStrCmp(d2->getString().c_str(), d1->getString().c_str());
      
      // send a datagram with the server
      d2->assignString("G'day, Client.");
      d2->setAddress(ca);
      server.send(d2);
      assertNoException();
      
      //printf("Server sent: %s\n", d2->getString().c_str());
      
      // receive the server datagram
      ia = new InternetAddress();
      DatagramRef d3 = new Datagram(ia, d2->getString().length());
      client.receive(d3);
      assertNoException();
      
      //printf("Client received: %s\n", d3->getString().c_str());
      //printf("Data from: %s:%i\n",
      //   d3->getAddress()->getAddress(), d3->getAddress()->getPort());
      //assertStrCmp(
      //   d3->getAddress()->toString().c_str(), sa->toString().c_str());
      assertStrCmp(
         d3->getString().c_str(), d2->getString().c_str());
      
      // close sockets
      client.close();
      server.close();
      
      //printf("Sockets closed.\n");
   }
   tr.passIfNoException();
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
   
   assertStrCmp(decoded.c_str(), str.c_str());
   
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
         "/path/param1/10001?key1=value1&key2=value2&key3=two%20words%3D2");
      
      //dumpUrl(url);
      assert(!Exception::hasLast());
      assertStrCmp(url.getPath().c_str(), "/path/param1/10001");
      assertStrCmp(
         url.getQuery().c_str(),
         "key1=value1&key2=value2&key3=two%20words%3D2");
      
      DynamicObject tokens;
      assert(url.getTokenizedPath(tokens, "/path/"));
      assertStrCmp(tokens[0]->getString(), "param1");
      assert(tokens[1]->getInt32() == 10001);
      
      DynamicObject vars;
      assert(url.getQueryVariables(vars));
      assertStrCmp(vars["key1"]->getString(), "value1");
      assertStrCmp(vars["key2"]->getString(), "value2");
      assertStrCmp(vars["key3"]->getString(), "two words=2");
   }
   
   {
      Url url("http://example.com/path/"
         "?key1=100&start=2008-07-04+00%3a00%3a00&end=2008-07-04+23%3a59%3a59");
      
      //dumpUrl(url);
      DynamicObject vars;
      assert(url.getQueryVariables(vars));
      assertStrCmp(vars["key1"]->getString(), "100");
      assertStrCmp(vars["start"]->getString(), "2008-07-04 00:00:00");
      assertStrCmp(vars["end"]->getString(), "2008-07-04 23:59:59");
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
   
   {
      Url url("http://bitmunk.com/path");
      
      DynamicObject vars;
      assert(!url.getQueryVariables(vars));
      assert(vars->getType() == Map);
   }
   
   {
      Url url("http://bitmunk.com/path?");
      
      DynamicObject vars;
      assert(!url.getQueryVariables(vars));
      assert(vars->getType() == Map);
   }
   
   {
      Url url("http://bitmunk.com/path?foo=bar");
      
      DynamicObject vars;
      assert(url.getQueryVariables(vars));
      assert(vars->getType() == Map);
   }
   
   {
      Url url("http://bitmunk.com/path?foo");
      
      DynamicObject vars;
      assert(url.getQueryVariables(vars));
      assert(vars->getType() == Map);
      assertStrCmp(vars["foo"]->getString(), "");
   }
   
   {
      Url url("http://bitmunk.com/path?&");
      
      DynamicObject vars;
      assert(!url.getQueryVariables(vars));
      assert(vars->getType() == Map);
   }
   
   {
      Url url("http://bitmunk.com/path?=");
      
      DynamicObject vars;
      assert(!url.getQueryVariables(vars));
      assert(vars->getType() == Map);
   }
   
   {
      Url url("http://bitmunk.com/path?=foo");
      
      DynamicObject vars;
      assert(!url.getQueryVariables(vars));
      assert(vars->getType() == Map);
   }
   
   {
      const char* allchars =
         "0123456789"
         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
         "abcdefghijklmnopqrstuvwxyz"
         "-_.!~*'()";
      Url url;
      url.format("http://bitmunk.com/path?q=%s", allchars);
      DynamicObject vars;
      assert(url.getQueryVariables(vars));
      assert(vars->getType() == Map);
      assertStrCmp(vars["q"]->getString(), allchars);
   }
   
   {
      Url url("http://bitmunk.com");
      DynamicObject vars;
      vars["q1"] = "one";
      vars["q2"] = "two";
      url.addQueryVariables(vars);
      assertStrCmp(url.toString().c_str(), "http://bitmunk.com?q1=one&q2=two");
   }
   
   {
      Url url("http://bitmunk.com?q1=1");
      DynamicObject vars;
      vars["q2"] = "2";
      vars["q3"] = "3";
      url.addQueryVariables(vars);
      assertStrCmp(url.toString().c_str(), "http://bitmunk.com?q1=1&q2=2&q3=3");
   }
   
   {
      Url url("http://bitmunk.com");
      DynamicObject vars;
      vars["date"] = "2008-01-01 00:00:01";
      url.addQueryVariables(vars);
      assertStrCmp(
         url.toString().c_str(),
         "http://bitmunk.com?date=2008-01-01+00%3A00%3A01");
   }
   
   tr.pass();
}

class InterruptServerSocketTest : public Runnable
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

void runServerDynamicServiceTest(TestRunner& tr)
{
   tr.test("Server dynamic service");
   {
      // create kernel
      Kernel k;
      k.getEngine()->start();
      
      // create server
      Server server(&k);
      InternetAddress address1("0.0.0.0", 0);
      InternetAddress address2("0.0.0.0", 0);
      
      // create generic service
      TestConnectionServicer1 tcs1;
      Server::ServiceId id1 = server.addConnectionService(&address1, &tcs1);
      assert(id1 != 0);
      
      server.start();
      assertNoException();
      
      // create generic service
      TestConnectionServicer2 tcs2;
      Server::ServiceId id2 = server.addConnectionService(&address2, &tcs2);
      assert(id2 != 0);
      
      //printf("address1: %s\n", address1.toString().c_str());
      //printf("address2: %s\n", address2.toString().c_str());
      
      // remove service 1
      assert(server.removePortService(id1));
      
      // stop server
      server.stop();
      
      // stop kernel engine
      k.getEngine()->stop();
   }
   tr.passIfNoException();
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
   tr.group("HttpHeader");
   
   tr.test("Bicapitalization");
   {
      // test bicapitalization of http headers
      const char* tests[] = {
         "", "",
         "a", "A",
         "-", "-",
         "a--a", "A--A",
         "-aa-", "-Aa-",
         "-aa", "-Aa",
         "aa-", "Aa-",
         "aaa-zzz", "Aaa-Zzz",
         "ThIs-a-BICaPitAlized-hEADer", "This-A-Bicapitalized-Header",
         "Message-ID", "Message-Id",
         NULL
      };
      for(int i = 0; tests[i] != NULL; i +=2)
      {
         char* bic = strdup(tests[i]);
         HttpHeader::biCapitalize(bic);
         assertStrCmp(bic, tests[i+1]);
         free(bic);
      }
   }
   tr.passIfNoException();
   
   tr.test("HttpRequestHeader parse");
   {
      HttpRequestHeader header;
      header.setDate();
      header.setMethod("GET");
      header.setPath("/");
      header.setVersion("HTTP/1.1");
      header.setField("host", "localhost:80");
      header.setField("Content-Type", "text/html");
      header.setField("Connection", "close");
      
      string date;
      string expect;
      expect.append("GET / HTTP/1.1\r\n");
      expect.append("Connection: close\r\n");
      expect.append("Content-Type: text/html\r\n");
      expect.append("Date: ");
      header.getField("Date", date);
      expect.append(date);
      expect.append("\r\n");
      expect.append("Host: localhost:80\r\n");
      expect.append("\r\n");
      
      string str = header.toString();
      assertStrCmp(str.c_str(), expect.c_str());
      
      HttpRequestHeader header2;
      header2.parse(str);
      
      string str2 = header2.toString();
      assertStrCmp(str2.c_str(), expect.c_str());
   }
   tr.passIfNoException();
   
   tr.test("HttpResponseHeader parse");
   {
      HttpResponseHeader header;
      header.setDate();
      header.setVersion("HTTP/1.1");
      header.setStatus(404, "Not Found");
      header.setField("host", "localhost:80");
      header.setField("Content-Type", "text/html");
      header.setField("Connection", "close");
      
      string date;
      string expect;
      expect.append("HTTP/1.1 404 Not Found\r\n");
      expect.append("Connection: close\r\n");
      expect.append("Content-Type: text/html\r\n");
      expect.append("Date: ");
      header.getField("Date", date);
      expect.append(date);
      expect.append("\r\n");
      expect.append("Host: localhost:80\r\n");
      expect.append("\r\n");
      
      string str = header.toString();
      assertStrCmp(str.c_str(), expect.c_str());
      
      HttpResponseHeader header2;
      header2.parse(str);
      
      string str2 = header2.toString();
      assertStrCmp(str2.c_str(), expect.c_str());
   }
   tr.passIfNoException();
   
   tr.test("Multiple fields with same name");
   {
      HttpResponseHeader header;
      header.setDate();
      header.setVersion("HTTP/1.1");
      header.setStatus(404, "Not Found");
      header.setField("host", "localhost:80");
      header.setField("Content-Type", "text/html");
      header.setField("Connection", "close");
      header.addField("Set-Cookie", "cookie1=value1; max-age=0; path=/");
      header.addField("Set-Cookie", "cookie2=value2; max-age=0; path=/");
      header.addField("Set-Cookie", "cookie3=value3; max-age=0; path=/");
      
      string date;
      string expect;
      expect.append("HTTP/1.1 404 Not Found\r\n");
      expect.append("Connection: close\r\n");
      expect.append("Content-Type: text/html\r\n");
      expect.append("Date: ");
      header.getField("Date", date);
      expect.append(date);
      expect.append("\r\n");
      expect.append("Host: localhost:80\r\n");
      expect.append("Set-Cookie: cookie1=value1; max-age=0; path=/\r\n");
      expect.append("Set-Cookie: cookie2=value2; max-age=0; path=/\r\n");
      expect.append("Set-Cookie: cookie3=value3; max-age=0; path=/\r\n");
      expect.append("\r\n");
      
      string str = header.toString();
      assertStrCmp(str.c_str(), expect.c_str());
      
      HttpResponseHeader header2;
      header2.parse(str);
      
      string str2 = header2.toString();
      assertStrCmp(str2.c_str(), expect.c_str());
   }
   tr.passIfNoException();
   
   tr.ungroup();
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

void runCookieTest(TestRunner& tr)
{
   tr.group("Http Cookie");
   
   tr.test("parse Set-Cookie header");
   {
      HttpHeader header;
      header.addField("Set-Cookie",
         "cookie1_name=cookie1_value; max-age=0; path=/");
      header.addField("Set-Cookie",
         "cookie2_name=cookie2_value; max-age=0; path=/; secure");
      header.addField("Set-Cookie",
         "cookie3_name=cookie3_value; max-age=0; path=/; secure");
      header.addField("Set-Cookie",
         "cookie4_name=cookie4_value; max-age=0; path=/moo");
      
      CookieJar jar;
      jar.readCookies(&header, CookieJar::Server);
      
      Cookie cookie1 = jar.getCookie("cookie1_name");
      Cookie cookie2 = jar.getCookie("cookie2_name");
      Cookie cookie3 = jar.getCookie("cookie3_name");
      Cookie cookie4 = jar.getCookie("cookie4_name");
      Cookie cookie5 = jar.getCookie("cookie5_name");
      
      assert(!cookie1.isNull());
      assert(!cookie2.isNull());
      assert(!cookie3.isNull());
      assert(!cookie4.isNull());
      assert(cookie5.isNull());
      
      assertStrCmp(cookie1["name"]->getString(), "cookie1_name");
      assertStrCmp(cookie2["name"]->getString(), "cookie2_name");
      assertStrCmp(cookie3["name"]->getString(), "cookie3_name");
      assertStrCmp(cookie4["name"]->getString(), "cookie4_name");
      
      assertStrCmp(cookie1["value"]->getString(), "cookie1_value");
      assertStrCmp(cookie2["value"]->getString(), "cookie2_value");
      assertStrCmp(cookie3["value"]->getString(), "cookie3_value");
      assertStrCmp(cookie4["value"]->getString(), "cookie4_value");
      
      assertStrCmp(cookie1["path"]->getString(), "/");
      assertStrCmp(cookie2["path"]->getString(), "/");
      assertStrCmp(cookie3["path"]->getString(), "/");
      assertStrCmp(cookie4["path"]->getString(), "/moo");
      
      assert(!cookie1["secure"]->getBoolean());
      assert(cookie2["secure"]->getBoolean());
      assert(cookie3["secure"]->getBoolean());
      assert(!cookie4["secure"]->getBoolean());
   }
   tr.passIfNoException();
   
   tr.test("overwrite Set-Cookie header");
   {
      HttpHeader header;
      header.addField(
         "Set-Cookie",
         "cookie1_name=cookie1_value; max-age=30; path=/");
      header.addField(
         "Set-Cookie",
         "cookie2_name=cookie2_value; max-age=30; path=/; secure");
      header.addField(
         "Set-Cookie",
         "cookie3_name=cookie3_value; max-age=30; path=/");
      
      CookieJar jar;
      jar.deleteCookie("cookie2_name", false);
      jar.setCookie("cookie4_name", "cookie4_value", 0, true, true);
      jar.writeCookies(&header, CookieJar::Server, true);
      
      string cookies;
      
      header.getField("Set-Cookie", cookies, 0);
      assertStrCmp(cookies.c_str(),
         "cookie2_name=; max-age=0; path=/");
      
      header.getField("Set-Cookie", cookies, 1);
      assertStrCmp(cookies.c_str(),
         "cookie4_name=cookie4_value; max-age=0; path=/; secure; HttpOnly");
   }
   tr.passIfNoException();
   
   tr.test("extend Set-Cookie header");
   {
      HttpHeader header;
      header.setField("Set-Cookie",
         "cookie1_name=cookie1_value; max-age=0; path=/, "
         "cookie2_name=cookie2_value; max-age=0; path=/; secure, "
         "cookie3_name=cookie3_value; max-age=0; path=/");
      
      CookieJar jar;
      jar.setCookie("cookie4_name", "cookie4_value", 0, true, false);
      jar.deleteCookie("cookie5_name", true);
      jar.writeCookies(&header, CookieJar::Server, false);
      
      string cookies;
      
      header.getField("Set-Cookie", cookies, 0);
      assertStrCmp(cookies.c_str(),
         "cookie1_name=cookie1_value; max-age=0; path=/, "
         "cookie2_name=cookie2_value; max-age=0; path=/; secure, "
         "cookie3_name=cookie3_value; max-age=0; path=/");
      
      header.getField("Set-Cookie", cookies, 1);
      assertStrCmp(cookies.c_str(),
         "cookie4_name=cookie4_value; max-age=0; path=/; secure");
      
      header.getField("Set-Cookie", cookies, 2);
      assertStrCmp(cookies.c_str(),
         "cookie5_name=; max-age=0; path=/; secure");
   }
   tr.passIfNoException();
   
   tr.test("parse Cookie header");
   {
      HttpHeader header;
      header.setField("Cookie",
         "cookie1_name=cookie1_value; cookie2_name=cookie2_value; "
         "cookie3_name=cookie3_value");
      
      CookieJar jar;
      jar.readCookies(&header, CookieJar::Client);
      
      Cookie cookie1 = jar.getCookie("cookie1_name");
      Cookie cookie2 = jar.getCookie("cookie2_name");
      Cookie cookie3 = jar.getCookie("cookie3_name");
      Cookie cookie4 = jar.getCookie("cookie4_name");
      
      assert(!cookie1.isNull());
      assert(!cookie2.isNull());
      assert(!cookie3.isNull());
      assert(cookie4.isNull());
      
      assertStrCmp(cookie1["name"]->getString(), "cookie1_name");
      assertStrCmp(cookie2["name"]->getString(), "cookie2_name");
      assertStrCmp(cookie3["name"]->getString(), "cookie3_name");
      
      assertStrCmp(cookie1["value"]->getString(), "cookie1_value");
      assertStrCmp(cookie2["value"]->getString(), "cookie2_value");
      assertStrCmp(cookie3["value"]->getString(), "cookie3_value");
   }
   tr.passIfNoException();
   
   tr.test("overwrite Cookie header");
   {
      HttpHeader header;
      header.setField("Cookie",
         "cookie1_name=cookie1_value; cookie2_name=cookie2_value; "
         "cookie3_name=cookie3_value");
      
      CookieJar jar;
      jar.readCookies(&header, CookieJar::Client);
      jar.removeCookie("cookie2_name");
      jar.deleteCookie("cookie3_name", true);
      jar.setCookie("cookie1_name", "cookie1_value", 30, true, true);
      jar.setCookie("cookie4_name", "cookie4_value", 30, true, false);
      jar.writeCookies(&header, CookieJar::Client, true);
      
      string cookies;
      header.getField("Cookie", cookies);
      
      assertStrCmp(cookies.c_str(),
         "cookie1_name=cookie1_value; "
         "cookie4_name=cookie4_value");
   }
   tr.passIfNoException();
   
   tr.test("extend Cookie header");
   {
      HttpHeader header;
      header.setField("Cookie",
         "cookie1_name=cookie1_value; cookie2_name=cookie2_value; "
         "cookie3_name=cookie3_value");
      
      CookieJar jar;
      jar.setCookie("cookie4_name", "cookie4_value", 30, true, false);
      jar.writeCookies(&header, CookieJar::Client, false);
      
      string cookies;
      
      header.getField("Cookie", cookies, 0);
      assertStrCmp(cookies.c_str(),
         "cookie1_name=cookie1_value; "
         "cookie2_name=cookie2_value; "
         "cookie3_name=cookie3_value");
      
      header.getField("Cookie", cookies, 1);
      assertStrCmp(cookies.c_str(), "cookie4_name=cookie4_value");
   }
   tr.passIfNoException();
   
   tr.ungroup();
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
      DynamicObject headers;
      headers["Test-Header"] = "bacon";
      HttpResponse* response = client.get(&url, &headers);
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
                  file->getAbsolutePath() << "'" << endl;
               
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
      DynamicObject headers;
      headers["Content-Type"] = "text/plain";
      headers["Transfer-Encoding"] = "chunked";
      
      HttpTrailer trailer;
      HttpResponse* response = client.post(&url, &headers, &baos, &trailer);
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
                  file->getAbsolutePath() << "'" << endl;
               
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
      runAddressResolveTest(tr);
      runSocketTest(tr);
      runServerDynamicServiceTest(tr);
      runUrlEncodeTest(tr);
      runUrlTest(tr);
      runHttpHeaderTest(tr);
      runHttpNormalizePath(tr);
      runCookieTest(tr);
      runUdpClientServerTest(tr);
      runDatagramTest(tr);
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
