/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/io/File.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/io/FileList.h"
#include "monarch/modest/Kernel.h"
#include "monarch/net/TcpSocket.h"
#include "monarch/net/UdpSocket.h"
#include "monarch/net/DatagramSocket.h"
#include "monarch/net/Internet6Address.h"
#include "monarch/net/SslSocket.h"
#include "monarch/net/Url.h"
#include "monarch/net/Server.h"
#include "monarch/net/NullSocketDataPresenter.h"
#include "monarch/net/SslSocketDataPresenter.h"
#include "monarch/net/SocketDataPresenterList.h"
#include "monarch/net/SocketTools.h"
#include "monarch/rt/System.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/util/Date.h"
#include "monarch/util/StringTools.h"

using namespace std;
using namespace monarch::data;
using namespace monarch::test;
using namespace monarch::io;
using namespace monarch::modest;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;

void runAddressResolveTest(TestRunner& tr)
{
   tr.test("Address Resolution");

   Exception::clear();

   // create IPv4 address
   InternetAddress ip4;

   //printf("Testing IPv4...\n\n");

   ip4.setHost("www.bitmunk.com");
   ip4.getAddress();
   assertNoException();
   //printf("www.bitmunk.com = %s\n", ip4.getAddress());

   ip4.setHost("www.google.com");
   ip4.getAddress();
   assertNoException();
   //printf("www.google.com = %s\n", ip4.getAddress());

   ip4.setHost("www.yahoo.com");
   ip4.getAddress();
   assertNoException();
   //printf("www.yahoo.com = %s\n", ip4.getAddress());

   ip4.setHost("www.microsoft.com");
   ip4.getAddress();
   assertNoException();
   //printf("www.microsoft.com = %s\n\n" << ip4.getAddress());

   ip4.setAddress("192.168.0.1");
   ip4.getAddress();
   ip4.getHost();
   assertNoException();
   //printf("%s = %s\n", ip4.getAddress(), ip4.getHost());

   ip4.setAddress("192.168.0.8");
   ip4.getAddress();
   ip4.getHost();
   assertNoException();
   //printf("%s = %s\n", ip4.getAddress(), ip4.getHost());

   ip4.setAddress("216.239.51.99");
   ip4.getAddress();
   ip4.getHost();
   assertNoException();
   //printf("%s = %s\n", ip4.getAddress(), ip4.getHost());

   string strAddress = ip4.toString();
   InternetAddress another;
   another.fromString(strAddress.c_str());
   assertStrCmp(another.toString().c_str(), strAddress.c_str());

//   // create IPv6 address
//   Internet6Address ip6;
//
//   printf("\nTesting IPv6...\n\n");
//
//   ip6.setHost("ip6-localhost");
//   printf("ip6-localhost = %s\n", ip6.getAddress());
//
//   ip6.setHost("yuna.digitalbazaar.com");
//   printf("yuna.digitalbazaar.com = %s\n", ip6.getAddress());
//
//   ip6.setHost("www.google.com");
//   printf("www.google.com = %s\n", ip6.getAddress());
//
//   ip6.setHost("www.yahoo.com");
//   printf("www.yahoo.com = %s\n", ip6.getAddress());
//
//   ip6.setHost("www.microsoft.com");
//   printf("www.microsoft.com = %s\n\n", ip6.getAddress());
//
//   ip6.setAddress("fc00:840:db:bb:d::8");
//   printf("%s = %s\n", ip6.getAddress(), ip6.getHost());

   tr.passIfNoException();
}

void runSocketTest(TestRunner& tr)
{
   tr.test("Socket");

   Exception::clear();

   // create address
   //InternetAddress address("127.0.0.1", 80);
   InternetAddress address("www.google.com", 80);

   // ensure host was known
   assertNoException();

   address.getAddress();
   assertNoException();
   //printf("Connecting to: %s\n", address.getAddress());

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

   //printf("\nDOING A PEEK!\n");

   string peek;
   numBytes = socket.getInputStream()->peek(response, 2048);
   if(numBytes > 0)
   {
      //printf("Peeked %d bytes\n", numBytes);
      peek.append(response, numBytes);
      //printf("Peek bytes=%s\n", peek.c_str());
   }
   assertNoException();

   //printf("\nDOING ACTUAL READ NOW!\n");
   int peekBytes = numBytes;
   while((numBytes = socket.getInputStream()->read(response, 2048)) > 0)
   {
      //printf("numBytes received: %d\n", numBytes);
      str.append(response, numBytes);
   }

   // confirm peek bytes check out
   assert(strncmp(peek.c_str(), str.c_str(), peekBytes) == 0);

//   char response[2048];
//   int numBytes = 0;
//   string str = "";
//   while((numBytes = socket.receive(response, 0, 2048)) > 0)
//   {
//      printf("numBytes received: %d\n");
//      str.append(response, numBytes);
//   }

   //printf("Response:\n%s\n", str.c_str());

   // close
   socket.close();

   tr.passIfNoException();
}

void runSslSocketTest(TestRunner& tr)
{
   tr.test("SSL Socket");

   // create address
   InternetAddress address("127.0.0.1", 443);
   //InternetAddress address("127.0.0.1", 19020);
   //InternetAddress address("www.google.com", 80);
   printf("%s\n", address.getAddress());

   // ensure host was known
   if(!Exception::isSet())
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

      printf("\nDOING A PEEK!\n");

      numBytes = sslSocket.getInputStream()->peek(response, 2048);
      if(numBytes > 0)
      {
         printf("Peeked %d bytes.\n", numBytes);
         string peek = "";
         peek.append(response, numBytes);
         printf("Peek bytes=%s\n", peek.c_str());
      }

      printf("\nDOING ACTUAL READ NOW!\n");

      while((numBytes = sslSocket.getInputStream()->read(response, 2048)) > 0)
      {
         printf("numBytes received: %d\n", numBytes);
         str.append(response, numBytes);
      }

      printf("Response:\n%s\n", str.c_str());

      // close
      sslSocket.close();
   }

   tr.passIfNoException();
}

void runServerSocketTest(TestRunner& tr, bool passIfException = false)
{
   tr.test("Server Socket");

   Exception::clear();

   // bind and listen
   InternetAddress address("127.0.0.1", 19100);

   // ensure host was known
   if(!Exception::isSet())
   {
      // create tcp socket
      TcpSocket socket;

      if(socket.bind(&address))
      {
         printf("Server socket bound...\n");
      }
      else
      {
         printf("Could not bind server socket!\n");
      }

      if(socket.listen())
      {
         printf("Listening for a connection...\n");
      }
      else
      {
         printf("Could not listen with server socket!\n");
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
   }

   if(passIfException)
   {
      tr.passIfException();
   }
   else
   {
      tr.passIfNoException();
   }
}

void runSslServerSocketTest(TestRunner& tr)
{
   tr.test("SSL Server Socket");

   // bind and listen
   InternetAddress address("127.0.0.1", 1024);

   // ensure host was known
   if(!Exception::isSet())
   {
      // create tcp socket
      TcpSocket socket;

      if(socket.bind(&address))
      {
         printf("Server socket bound...\n");
      }
      else
      {
         printf("Could not bind server socket!\n");
      }

      if(socket.listen())
      {
         printf("Listening for a connection...\n");
      }
      else
      {
         printf("Could not listen with server socket!\n");
      }

      // accept a connection
      TcpSocket* worker = (TcpSocket*)socket.accept(10);
      if(worker != NULL)
      {
         printf("Accepted a connection!\n");

         // create an SSL context
         SslContext context(NULL, false);

         // create an SSL socket
         SslSocket sslSocket(&context, worker, false, false);

         // set receive timeout (10 seconds = 10000 milliseconds)
         sslSocket.setReceiveTimeout(10000);

         char request[2048];
         int numBytes = 0;
         string str = "";

         printf("\nDOING A PEEK!\n");

         numBytes = worker->getInputStream()->peek(request, 2048);
         if(numBytes > 0)
         {
            printf("Peeked %d bytes.\n", numBytes);
            string peek = "";
            peek.append(request, numBytes);
            printf("Peek bytes=%s\n", peek.c_str());
         }

         printf("\nDOING ACTUAL READ NOW!\n");

         while((numBytes = sslSocket.getInputStream()->read(request, 2048)) > 0)
         {
            printf("numBytes received: %d\n", numBytes);
            str.append(request, numBytes);
         }

         printf("Request:\n%s\n", str.c_str());

         // close ssl socket socket
         sslSocket.close();
         delete worker;
      }
      else
      {
         printf("Could not accept a connection!\n");
      }

      // close server socket
      socket.close();
   }

   tr.passIfNoException();
}

void runTcpClientServerTest(TestRunner& tr)
{
   tr.test("TCP Client/Server");

   InternetAddress* address;
   InternetAddress ia("127.0.0.1", 9999);
   //Internet6Address ia("::0", 9999);
   address = &ia;

   // ensure host was known
   if(!Exception::isSet())
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

      printf("Server listening at host: %s\n", address->getHost());
      printf("Server listening at address: %s\n", address->getAddress());
      printf("Server listening on port: %d\n", address->getPort());

      // connect with client
      client.connect(address);

      printf("Client connected.\n");

      // accept a connection
      TcpSocket* worker = (TcpSocket*)server.accept(10);

      printf("Client connection accepted by Server.\n");

      // send some data with client
      string clientData = "Hello there, Server.";
      client.getOutputStream()->write(clientData.c_str(), clientData.length());

      printf("Client sent: %s\n", clientData.c_str());

      // receive the client data
      char read[2048];
      int numBytes = worker->getInputStream()->read(read, 2048);
      string serverReceived(read, numBytes);

      printf("Server received: %s\n", serverReceived.c_str());

      // send some data with server
      string serverData = "G'day, Client.";
      worker->getOutputStream()->write(serverData.c_str(), serverData.length());

      printf("Server sent: %s\n", serverData.c_str());

      // receive the server data
      numBytes = client.getInputStream()->read(read, 2048);
      string clientReceived(read, numBytes);

      printf("Client received: %s\n", clientReceived.c_str());

      // close sockets
      client.close();
      server.close();

      // delete worker
      if(worker != NULL)
      {
         worker->close();
         delete worker;
      }

      printf("Sockets closed.\n");
   }

   tr.passIfNoException();
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

   tr.ungroup();
}

void runUrlEncodeTest(TestRunner& tr)
{
   tr.test("Url Encode/Decode");

   string str = "billy bob & \"jane\" +^%2{13.";

   string encoded = Url::encode(str.c_str(), str.length());
   string decoded = Url::decode(encoded.c_str(), encoded.length());

   assertStrCmp(decoded.c_str(), str.c_str());

   tr.pass();
}

void dumpUrl(Url url)
{
   if(Exception::isSet())
   {
      printf("url=[exception]\n");
   }
   else
   {
      string str = url.toString();

      printf("url=%s\n", str.c_str());
      printf(" scheme=%s\n", url.getScheme().c_str());
      printf(" scheme specific part=%s\n", url.getSchemeSpecificPart().c_str());
      printf(" authority=%s\n", url.getAuthority().c_str());
      printf(" userinfo=%s\n", url.getUserInfo().c_str());
      printf(" user=%s\n", url.getUser().c_str());
      printf(" password=%s\n", url.getPassword().c_str());
      printf(" host=%s\n", url.getHost().c_str());
      printf(" port=%d\n", url.getPort());
      printf(" path=%s\n", url.getPath().c_str());
      printf(" query=%s\n", url.getQuery().c_str());
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
      assert(!Exception::isSet());
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
      assert(!Exception::isSet());
      assert(url.getScheme() == "scheme");
      assert(url.getSchemeSpecificPart() == "schemespecific");
   }

   {
      Url url(
         "scheme://user:password@host:1234/path?key1=value1&key2=value2"
         "&key3=two%20words%3D2");

      //dumpUrl(url);
      assert(!Exception::isSet());
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
      assert(!Exception::isSet());
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
      assert(!Exception::isSet());
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
      // check using last value for key
      Url url("http://bitmunk.com/path?a=1&a=2");

      DynamicObject vars;
      assert(url.getQueryVariables(vars));
      DynamicObject expect;
      expect["a"] = "2";
      assertDynoCmp(vars, expect);
   }

   {
      // check key arrays
      Url url("http://bitmunk.com/path?a=1&a=2&a=");

      DynamicObject vars;
      assert(url.getQueryVariables(vars, true));
      DynamicObject expect;
      expect["a"][0] = "1";
      expect["a"][1] = "2";
      expect["a"][2] = "";
      assertDynoCmp(vars, expect);
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
   TestRunner& testRunner;
   InterruptServerSocketTest(TestRunner& tr) :
      testRunner(tr)
   {
   };
   ~InterruptServerSocketTest() {};

   /**
    * Runs the unit tests.
    */
   virtual void run()
   {
      runServerSocketTest(testRunner, true);

      if(Exception::isSet())
      {
         ExceptionRef e = Exception::get();
         printf("Exception occurred!\n");
         printf("message: %s\n", e->getMessage());
         printf("type: %s\n", e->getType());
         printf("code: %d\n", e->getCode());
      }
   }
};

void runInterruptServerSocketTest(TestRunner& tr)
{
   tr.test("Thread Interrupt");

   InterruptServerSocketTest runnable(tr);
   Thread t(&runnable);
   t.start(131072);

   Thread::sleep(2000);
   t.interrupt();
   t.join();

   tr.passIfNoException();
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
      //printf("1: Servicing connection!\n");

      char b[100];
      int numBytes = 0;

      //printf("\nReading HTTP...\n");

      InputStream* is = c->getInputStream();
      numBytes = is->peek(b, 100);
      if(numBytes > 0)
      {
//         printf("Server read %d bytes.\n", numBytes);
//         string str;
//         str.append(b, numBytes);
//         printf("DATA=\n%s\n", str.c_str());
      }
      else if(numBytes == -1)
      {
         printf("Server Exception=%s\n%s\n",
            Exception::get()->getMessage(),
            Exception::get()->getType());
      }

      OutputStream* os = c->getOutputStream();
      if(os->write(reply.c_str(), reply.length()))
      {
//         printf("Server sent=\n%s\n", reply.c_str());
      }
      else
      {
         printf("Server Exception=%s\n%s\n",
            Exception::get()->getMessage(),
            Exception::get()->getType());
      }

//      printf("1: Finished servicing connection.\n");

      serviced++;
//      printf("Connections serviced=%d\n", serviced);
   }
};

class TestConnectionServicer2 : public ConnectionServicer
{
   void serviceConnection(Connection* c)
   {
      printf("2: Servicing connection.\n");
      printf("2: Finished servicing connection.\n");
   }
};

class TestConnectionServicer3 : public ConnectionServicer
{
   void serviceConnection(Connection* c)
   {
      printf("3: Servicing connection.\n");
      printf("3: Finished servicing connection.\n");
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
      Server server;
      InternetAddress address1("0.0.0.0", 0);
      InternetAddress address2("0.0.0.0", 0);

      // create generic service
      TestConnectionServicer1 tcs1;
      Server::ServiceId id1 = server.addConnectionService(&address1, &tcs1);
      assert(id1 != 0);

      server.start(&k);
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
            //printf("connected\n");

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
//               printf("Client read %d bytes.\n", numBytes);
//               printf("DATA=%s\n", string(b, numBytes).c_str());
            }
            else
            {
               printf("Client Exception=%s\n%s\n",
                  Exception::get()->getMessage(),
                  Exception::get()->getType());
            }
         }
         else
         {
            printf("Client Exception=%s\n%s\n",
               Exception::get()->getMessage(),
               Exception::get()->getType());
         }

         // close socket
         socket->close();

         // clean up socket
         delete socket;
      }
   }
};

void runServerSslConnectionTest(TestRunner& tr)
{
   tr.test("Server SSL Connection");

   // create kernel
   Kernel k;
   k.getEngine()->start();

   // create server
   Server server;
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

   assert(server.start(&k));
   printf("Server started.\n");

   BlastConnections bc(&address, true);
   Thread t1(&bc);
   Thread t2(&bc);
   Thread t3(&bc);
   Thread t4(&bc);
   Thread t5(&bc);
   Thread t6(&bc);
   Thread t7(&bc);
   Thread t8(&bc);

   uint64_t start = System::getCurrentMilliseconds();

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
//   printf("all client threads joined.\n");

   uint64_t end = System::getCurrentMilliseconds();

   // stop server
   server.stop();
   printf("Server stopped.\n");

   // stop kernel engine
   k.getEngine()->stop();

   double time = end - start;
   double secs = time / 1000.0;
   double rate = (double)tcs1.serviced / secs;

   printf("Connections=%d\n", tcs1.serviced);
   printf("Time=%f ms = %f secs\n", time, secs);
   printf("Time/Connection=%f ms\n", (double)time / tcs1.serviced);
   printf("Connections/second=%f\n", rate);

   tr.passIfNoException();
}

class TestDatagramServicer : public DatagramServicer
{
   void serviceDatagrams(DatagramSocket* s)
   {
      printf("Servicing datagrams.\n");
      printf("Finished servicing datagrams.\n");
   }
};

void runServerDatagramTest(TestRunner& tr)
{
   tr.test("Server Datagram");

   // create kernel
   Kernel k;
   k.getEngine()->start();

   // create server
   Server server;
   InternetAddress address("localhost", 10080);

   // create datagram service
   TestDatagramServicer tds;
   server.addDatagramService(&address, &tds);

   if(server.start(&k))
   {
      printf("Server started.\n");
   }
   else if(Exception::get() != NULL)
   {
      printf("Server started with errors=%s\n",
         Exception::get()->getMessage());
   }

   Thread::sleep(10000);

   server.stop();
   printf("Server stopped.\n");

   // stop kernel engine
   k.getEngine()->stop();

   tr.passIfNoException();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runAddressResolveTest(tr);
      runSocketTest(tr);
      runServerDynamicServiceTest(tr);
      runUdpClientServerTest(tr);
      runDatagramTest(tr);
      runUrlEncodeTest(tr);
      runUrlTest(tr);
   }
   if(tr.isTestEnabled("local-hostname"))
   {
      printf("\nLocal hostname: %s\n", SocketTools::getHostname().c_str());
   }
   if(tr.isTestEnabled("interrupt-server-socket"))
   {
      runInterruptServerSocketTest(tr);
   }
   if(tr.isTestEnabled("ssl-socket"))
   {
      runSslSocketTest(tr);
   }
   if(tr.isTestEnabled("server-socket"))
   {
      runServerSocketTest(tr);
   }
   if(tr.isTestEnabled("ssl-server-socket"))
   {
      runSslServerSocketTest(tr);
   }
   if(tr.isTestEnabled("tcp-client-server"))
   {
      runTcpClientServerTest(tr);
   }
   if(tr.isTestEnabled("server-ssl-connection"))
   {
      runServerSslConnectionTest(tr);
   }
   if(tr.isTestEnabled("server-datagram"))
   {
      runServerDatagramTest(tr);
   }
   return true;
}

MO_TEST_MODULE_FN("monarch.tests.net.test", "1.0", run)
