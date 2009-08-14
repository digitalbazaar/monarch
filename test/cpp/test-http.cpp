/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/io/ByteArrayInputStream.h"
#include "db/io/File.h"
#include "db/io/FileInputStream.h"
#include "db/io/FileOutputStream.h"
#include "db/io/FileList.h"
#include "db/modest/Kernel.h"
#include "db/net/TcpSocket.h"
#include "db/net/Url.h"
#include "db/http/CookieJar.h"
#include "db/http/HttpHeader.h"
#include "db/http/HttpRequest.h"
#include "db/http/HttpResponse.h"
#include "db/http/HttpConnectionServicer.h"
#include "db/http/HttpRequestServicer.h"
#include "db/http/HttpClient.h"
#include "db/net/Server.h"
#include "db/net/NullSocketDataPresenter.h"
#include "db/net/SslSocketDataPresenter.h"
#include "db/net/SocketDataPresenterList.h"
#include "db/rt/System.h"
#include "db/rt/DynamicObject.h"
#include "db/rt/DynamicObjectIterator.h"
#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/util/Date.h"
#include "db/util/StringTools.h"

using namespace std;
using namespace db::test;
using namespace db::io;
using namespace db::modest;
using namespace db::net;
using namespace db::http;
using namespace db::rt;
using namespace db::util;

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

void runHttpServerTest(TestRunner& tr)
{
   tr.test("Http Server");

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
      printf("Server started.\n");
   }
   else if(Exception::get() != NULL)
   {
      printf("Server started with errors=%s\n",
         Exception::get()->getMessage());
   }

   // sleep
   Thread::sleep(30000);

   server.stop();
   printf("Server stopped.\n");

   // stop kernel engine
   k.getEngine()->stop();

   tr.passIfNoException();
}

void runHttpClientGetTest(TestRunner& tr)
{
   tr.test("Http Client GET");

   // create client
   HttpClient client;

   // connect
   Url url("http://www.bitmunk.com");
   if(client.connect(&url))
   {
      printf("Connected to: %s\n", url.toString().c_str());
      InternetAddress address(url.getHost().c_str(), url.getPort());
      printf("%s\n", address.toString().c_str());

      // do get
      DynamicObject headers;
      headers["Test-Header"] = "bacon";
      HttpResponse* response = client.get(&url, &headers);
      if(response != NULL)
      {
         printf("Response=\n%s\n", response->getHeader()->toString().c_str());
         if(response->getHeader()->getStatusCode() == 200)
         {
            // receive content
            HttpTrailer trailer;
            File file("/tmp/index.html");
            FileOutputStream fos(file);
            if(client.receiveContent(&fos, &trailer))
            {
               printf("Content downloaded to '%s'\n",
                  file->getAbsolutePath());

               printf("HTTP trailers=\n%s\n", trailer.toString().c_str());
            }
            assertNoException();
         }
      }
      else
      {
         printf("There was no response!\n");
      }

      client.disconnect();
   }

   tr.passIfNoException();
}

void runHttpClientPostTest(TestRunner& tr)
{
   tr.test("Http Client POST");

   // create client
   HttpClient client;

   // connect
   Url url("http://www.bitmunk.com");
   if(client.connect(&url))
   {
      printf("Connected to: %s\n", url.toString().c_str());
      InternetAddress address(url.getHost().c_str(), url.getPort());
      printf("%s\n", address.toString().c_str());

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
         printf("Response=\n%s\n",
            response->getHeader()->toString().c_str());
         if(response->getHeader()->getStatusCode() == 200)
         {
            // receive content
            trailer.clearFields();
            File file("/tmp/postresponse.txt");
            FileOutputStream fos(file);
            if(client.receiveContent(&fos, &trailer))
            {
               printf("Content downloaded to '%s'\n",
                  file->getAbsolutePath());

               printf("HTTP trailers=\n%s\n", trailer.toString().c_str());
            }
            assertNoException();
         }
      }
      else
      {
         printf("There was no response!\n");
      }

      client.disconnect();
   }

   tr.passIfNoException();
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

void runPingTest(TestRunner& tr)
{
   tr.test("Ping");

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
      printf("Server started.\n");
   }
   else if(Exception::get() != NULL)
   {
      printf("Server started with errors=%s\n",
         Exception::get()->getMessage());
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
   printf("Server stopped.\n");

   // stop kernel engine
   k.getEngine()->stop();

   unsigned long long millis = end - start;
   printf("Connection Time: %llu\n", millis);

//   unsigned long long millis = test1.end - test1.start;
//   long double cps = ((long double)pcs.serviced) / millis * 1000.0;
//   printf("Connections serviced: %d\n", pcs.serviced);
//   printf("Time: llu\n", millis);
//   printf("Connections/Second: %Lf\n", cps);

   tr.passIfNoException();
}

class DbHttpTester : public db::test::Tester
{
public:
   DbHttpTester()
   {
      setName("http");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runHttpHeaderTest(tr);
      runHttpNormalizePath(tr);
      runCookieTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      runHttpServerTest(tr);
//      runHttpClientGetTest(tr);
//      runHttpClientPostTest(tr);
//      runPingTest(tr);
      return 0;
   }
};

db::test::Tester* getDbHttpTester() { return new DbHttpTester(); }


DB_TEST_MAIN(DbHttpTester)
