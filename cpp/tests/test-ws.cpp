/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/io/ByteArrayOutputStream.h"
#include "monarch/modest/Kernel.h"
#include "monarch/net/Url.h"
#include "monarch/http/HttpHeader.h"
#include "monarch/http/HttpRequest.h"
#include "monarch/http/HttpResponse.h"
#include "monarch/http/HttpConnectionServicer.h"
#include "monarch/http/HttpRequestServicer.h"
#include "monarch/http/HttpClient.h"
#include "monarch/net/Server.h"
#include "monarch/rt/System.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/util/Date.h"
#include "monarch/util/StringTools.h"
#include "monarch/ws/PathHandlerDelegate.h"
#include "monarch/ws/RequestAuthenticatorDelegate.h"
#include "monarch/ws/RestfulHandler.h"
#include "monarch/ws/WebServer.h"

using namespace std;
using namespace monarch::config;
using namespace monarch::data::json;
using namespace monarch::http;
using namespace monarch::io;
using namespace monarch::modest;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::test;
using namespace monarch::util;
using namespace monarch::ws;

namespace mo_test_ws
{

class TestWebService;
typedef PathHandlerDelegate<TestWebService> Handler;
typedef RequestAuthenticatorDelegate<TestWebService> AuthHandler;

class TestWebService : public WebService
{
public:
   char* mContent;
   char* mRegexContent;
   TestWebService(const char* path, const char* content,
      const char* regexContent) : WebService(path)
   {
      mContent = strdup(content);
      mRegexContent = strdup(regexContent);
   }

   virtual ~TestWebService()
   {
      free(mContent);
      free(mRegexContent);
   }

   virtual bool initialize()
   {
      // root handler
      RestfulHandlerRef root = new RestfulHandler();
      addHandler("/", root);

      // GET /
      {
         PathHandlerRef h = new Handler(
            this, &TestWebService::handleRequest);
         root->addHandler(h, Message::Get, 0);
      }

      // GET /(.*)/regextest/(.*)
      {
         PathHandlerRef h = new Handler(
            this, &TestWebService::handleRegexRequest);
         h->addRequestAuthenticator(new AuthHandler(
            this, &TestWebService::authenticate));
         root->addRegexHandler("/(.*)/regextest/(.*)", h, Message::Get);
      }

      // GET /(.*)/regextest2/(.*)
      {
         DynamicObject userData;
         userData["foo"] = "bar";
         PathHandlerRef h = new Handler(
            this, &TestWebService::handleRegexRequest2);
         h->addRequestAuthenticator(new AuthHandler(
            this, &TestWebService::authenticate2, userData));
         root->addRegexHandler("/(.*)/regextest2/(.*)", h, Message::Get);
      }

      return true;
   }

   virtual void cleanup()
   {
   }

   virtual void handleRequest(ServiceChannel* ch)
   {
      // send 200 OK
      HttpResponseHeader* h = ch->getResponse()->getHeader();
      h->setStatus(200, "OK");
      //h->setField("Content-Length", 0);
      h->setField("Transfer-Encoding", "chunked");
      h->setField("Connection", "close");
      ch->getResponse()->sendHeader();

      ByteArrayInputStream bais(mContent, strlen(mContent));
      ch->getResponse()->sendBody(&bais);
   }

   virtual void handleRegexRequest(ServiceChannel* ch)
   {
      // send 200 OK
      HttpResponseHeader* h = ch->getResponse()->getHeader();
      h->setStatus(200, "OK");
      //h->setField("Content-Length", 0);
      h->setField("Transfer-Encoding", "chunked");
      h->setField("Connection", "close");
      ch->getResponse()->sendHeader();

      ByteArrayInputStream bais(mRegexContent, strlen(mRegexContent));
      ch->getResponse()->sendBody(&bais);
   }

   virtual void handleRegexRequest2(ServiceChannel* ch)
   {
      // send 200 OK
      HttpResponseHeader* h = ch->getResponse()->getHeader();
      h->setStatus(200, "OK");
      //h->setField("Content-Length", 0);
      h->setField("Transfer-Encoding", "chunked");
      h->setField("Connection", "close");
      ch->getResponse()->sendHeader();

      string out = JsonWriter::writeToString(ch->getHandlerInfo());
      ByteArrayInputStream bais(out.c_str(), out.length());
      ch->getResponse()->sendBody(&bais);
   }

   virtual bool authenticate(ServiceChannel* ch)
   {
      // anonymous authentication
      ch->setAuthenticationMethod(NULL);
      return true;
   }

   virtual bool authenticate2(ServiceChannel* ch, DynamicObject& data)
   {
      // anonymous authentication
      ch->setAuthenticationMethod(NULL);
      assertStrCmp(data["foo"]->getString(), "bar");
      return true;
   }
};

/**
 * Check a GET to a url returns a certain code and data.
 */
static void _checkUrlText(
   TestRunner& tr, Url* url, int code, const char* expected, int length)
{
   // create client
   HttpClient client;

   // connect
   assertNoException(client.connect(url));

   if(tr.getVerbosityLevel() > 1)
   {
      printf("Connected to: %s\n", url->toString().c_str());
      InternetAddress address(url->getHost().c_str(), url->getPort());
      printf("%s\n", address.toString().c_str());
   }

   // do get
   HttpResponse* response = client.get(url);
   assert(response != NULL);

   if(tr.getVerbosityLevel() > 1)
   {
      printf("Response header:\n%s\n",
         response->getHeader()->toString().c_str());
   }

   assert(response->getHeader()->getStatusCode() == 200);

   // receive content
   HttpTrailer trailer;
   ByteBuffer b;
   ByteArrayOutputStream baos(&b);
   assertNoException(client.receiveContent(&baos, &trailer));

   // put data in strings for strcmp since it may not be NULL terminated
   string strexpected;
   strexpected.assign(expected, length);

   string strdata;
   strdata.assign(b.data(), b.length());

   if(tr.getVerbosityLevel() > 1)
   {
      printf("Response content (%d bytes):\n%s\n", b.length(), strdata.c_str());
      printf("Response trailers:\n%s\n", trailer.toString().c_str());
   }

   // check content
   assertStrCmp(strdata.c_str(), strexpected.c_str());
   assert(b.length() == length);

   client.disconnect();

   assertNoExceptionSet();
}

static void runWebServerTest(TestRunner& tr)
{
   const char* path = "/test";
   const char* content = "web server test";
   const char* regexPath = "/test/dumplings/regextest/turkey";
   const char* regexPath2 = "/test/dumplings/regextest2/turkey";
   const char* regexContent = "web server test (regex)";

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
   Server server;

   WebServer ws;
   Config cfg;
   cfg["host"] = "localhost";
   cfg["port"] = 0;
   cfg["security"] = "off";
   WebServiceContainerRef wsc = new WebServiceContainer();
   ws.setContainer(wsc);
   ws.initialize(cfg);
   WebServiceRef tws = new TestWebService(path, content, regexContent);
   wsc->addService(tws, WebService::Both);
   ws.enable(&server);

   // start server
   assertNoException(server.start(&k));

   // get server port
   int port = ws.getHostAddress()->getPort();

   // check the regular path and data
   tr.test("WebServer - regular path handler");
   {
      Url url;
      url.format("http://%s:%d%s", cfg["host"]->getString(), port, path);
      _checkUrlText(tr, &url, 200, content, strlen(content));
   }
   tr.passIfNoException();

   // check the regex path and data
   tr.test("WebServer - regex path handler");
   {
      Url url;
      url.format("http://%s:%d%s", cfg["host"]->getString(), port, regexPath);
      _checkUrlText(tr, &url, 200, regexContent, strlen(regexContent));
   }
   tr.passIfNoException();

   // check the regex path and data
   tr.test("WebServer - regex path handler matches");
   {
      DynamicObject info;
      DynamicObject& matches = info["monarch.ws.RestfulHandler"]["matches"];
      matches[0] = "dumplings";
      matches[1] = "turkey";
      string expect = JsonWriter::writeToString(info);

      Url url;
      url.format("http://%s:%d%s", cfg["host"]->getString(), port, regexPath2);
      _checkUrlText(tr, &url, 200, expect.c_str(), expect.length());
   }
   tr.passIfNoException();

   server.stop();

   // stop kernel engine
   k.getEngine()->stop();
}

static bool run(TestRunner& tr)
{
   if(tr.isTestEnabled("ws-server"))
   {
      runWebServerTest(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.ws.test", "1.0", mo_test_ws::run)
