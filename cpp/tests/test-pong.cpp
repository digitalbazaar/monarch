/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/data/json/JsonWriter.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/io/File.h"
#include "monarch/io/FileInputStream.h"
#include "monarch/io/FileOutputStream.h"
#include "monarch/io/FileList.h"
#include "monarch/logging/Logging.h"
#include "monarch/modest/Kernel.h"
#include "monarch/net/TcpSocket.h"
#include "monarch/net/Url.h"
#include "monarch/http/CookieJar.h"
#include "monarch/http/HttpHeader.h"
#include "monarch/http/HttpRequest.h"
#include "monarch/http/HttpResponse.h"
#include "monarch/http/HttpConnectionServicer.h"
#include "monarch/http/HttpRequestServicer.h"
#include "monarch/http/HttpClient.h"
#include "monarch/net/Server.h"
#include "monarch/net/NullSocketDataPresenter.h"
#include "monarch/net/SslSocketDataPresenter.h"
#include "monarch/net/SocketDataPresenterList.h"
#include "monarch/rt/Atomic.h"
#include "monarch/rt/ExclusiveLock.h"
#include "monarch/rt/System.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/util/Date.h"
#include "monarch/util/StringTools.h"

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

namespace mo_test_pong
{

/**
 * PingPong Test
 * 
 * Invoke with the following:
 *
 *   ./monarch-run pong -t pong
 *
 * See configs/apps/pong.config.in for option defaults. Use standard monarch
 * "--json-option key=value" option to adjust options.
 *
 * pong.chunked=<bool>: use chunked encoding
 * pong.dynoStats=<bool>: return DynamicObject stats with regular stats
 * pong.num=<int32>: number of connections to service
 * pong.port=<int32>: port to serve on
 * pong.ssl=<bool>: use SSL
 * pong.time=<int32>: milliseconds to run the test
 * pong.threadStackSize=<int32>: set stack size
 * pong.threads=<int32>: set number of threads
 *
 * Endpoints:
 * /: return "Pong!"
 * /stats: return JSON object with various
 *
 */

// stats and control
class PingPong
{
private:
   uint64_t mStart;
   uint64_t mLast;
   uint64_t mServiced;
   uint64_t mNum;
   ExclusiveLock mLock;
   Config mConfig;

public:
   PingPong(Config cfg) :
      mStart(0),
      mLast(0),
      mServiced(0),
      mConfig(cfg)
   {
      mNum = cfg["num"]->getUInt64();
   }

   virtual ~PingPong() {}

   virtual void start()
   {
      mStart = System::getCurrentMilliseconds();
      mLast = mStart;
   }

   virtual void quit()
   {
      mLock.notifyAll();
   }

   virtual Config& getConfig()
   {
      return mConfig;
   }

   virtual ExclusiveLock& getLock()
   {
      return mLock;
   }

   virtual void service()
   {
      // set last serviced time for effective ping service time
      // avoids counting time between ping and stats calls
      mLast = System::getCurrentMilliseconds();

      // This is a bit sloppy and may do the lock notify multiple times and
      // increase mServiced more than mNum.  Assumption is that this doesn't
      // matter for this sort of performance testing.
      uint64_t s = Atomic::incrementAndFetch(&mServiced);
      if(mNum != 0 && s >= mNum)
      {
         quit();
      }
   }

   virtual DynamicObject getStats()
   {
      uint64_t tms = mLast - mStart;
      double rate = (tms == 0) ?
         0.0 :
         (double)mServiced * 1000.0 / (double)tms;

      DynamicObject stats;
      stats["serviced"] = mServiced;
      stats["num"] = mNum;
      stats["elapsed ms"] = tms;
      stats["req/s"] = rate;

      /*
      printf("Serviced: %" PRIu64 "/%" PRIu64 " in %0.3gs (%g r/s)\n",
         mServiced, mNum, (double)t, rate);
      if(mConfig["dynoStats"]->getBoolean())
      {
         JsonWriter::writeToStdOut(DynamicObjectImpl::getStats());
      }
      */

      return stats;
   }
};

class PingHttpRequestServicer : public HttpRequestServicer
{
protected:
   PingPong* mPingPong;
   const char* mContent;
   bool mChunked;

public:
   PingHttpRequestServicer(PingPong* pingPong, const char* path) :
      HttpRequestServicer(path),
      mPingPong(pingPong)
   {
      mContent = "Pong!";
      mChunked = pingPong->getConfig()["chunked"]->getBoolean();
   }

   virtual ~PingHttpRequestServicer()
   {
   }

   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
      int len = 5;
      // send 200 OK
      response->getHeader()->setStatus(200, "OK");
      if(mChunked)
      {
         response->getHeader()->setField("Transfer-Encoding", "chunked");
      }
      else
      {
         response->getHeader()->setField("Content-Length", len);
      }
      response->getHeader()->setField("Content-Type", "text/plain");
      response->getHeader()->setField("Connection", "close");
      response->sendHeader();
      ByteArrayInputStream bais(mContent, len);
      response->sendBody(&bais, NULL);
      mPingPong->service();
   }
};

class StatsHttpRequestServicer : public HttpRequestServicer
{
protected:
   PingPong* mPingPong;

public:
   StatsHttpRequestServicer(PingPong* pingPong, const char* path) :
      HttpRequestServicer(path),
      mPingPong(pingPong)
   {
   }

   virtual ~StatsHttpRequestServicer()
   {
   }

   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
      Config& c = mPingPong->getConfig();
      // generate stats string
      DynamicObject stats;
      if(c["dynoStats"]->getBoolean())
      {
         stats["dyno"] = DynamicObjectImpl::getStats();
      }
      stats["ping"] = mPingPong->getStats();
      string statsstr = JsonWriter::writeToString(stats);
      int len = statsstr.length();

      // send 200 OK
      response->getHeader()->setStatus(200, "OK");
      if(mPingPong->getConfig()["chunked"]->getBoolean())
      {
         response->getHeader()->setField("Transfer-Encoding", "chunked");
      }
      else
      {
         response->getHeader()->setField("Content-Length", len);
      }
      response->getHeader()->setField("Content-Type", "application/json");
      response->getHeader()->setField("Connection", "close");
      response->sendHeader();
      ByteArrayInputStream bais(statsstr.c_str(), len);
      response->sendBody(&bais, NULL);
   }
};

class QuitHttpRequestServicer : public HttpRequestServicer
{
protected:
   PingPong* mPingPong;

public:
   QuitHttpRequestServicer(PingPong* pingPong, const char* path) :
      HttpRequestServicer(path),
      mPingPong(pingPong)
   {
   }

   virtual ~QuitHttpRequestServicer()
   {
   }

   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
      response->getHeader()->setStatus(204, "No Content");
      response->getHeader()->setField("Content-Length", 0);
      response->getHeader()->setField("Connection", "close");
      response->sendHeader();
      mPingPong->quit();
   }
};

static void runPingTest(TestRunner& tr)
{
   tr.test("Ping");

   Config cfg = tr.getApp()->getConfig()["pong"];

   // Stats and control
   PingPong pingPong(cfg);

   // create kernel
   Kernel k;

   // set thread stack size in engine (128k)
   k.getEngine()->getThreadPool()->setThreadStackSize(
      cfg["threadStackSize"]->getUInt32());

   // optional for testing --
   // limit threads to 2: one for accepting, 1 for handling
   //k.getEngine()->getThreadPool()->setPoolSize(2);
   k.getEngine()->getThreadPool()->setPoolSize(cfg["threads"]->getUInt32());

   // start engine
   k.getEngine()->start();

   // create server
   Server server;
   InternetAddress address("0.0.0.0", cfg["port"]->getUInt32());

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
   PingHttpRequestServicer ping(&pingPong, "/");
   hcs.addRequestServicer(&ping, false);

   StatsHttpRequestServicer stats(&pingPong, "/stats");
   hcs.addRequestServicer(&stats, false);

   QuitHttpRequestServicer quit(&pingPong, "/quit");
   hcs.addRequestServicer(&quit, false);

   if(server.start(&k))
   {
      uint64_t num = cfg["num"]->getUInt64();
      MO_INFO("Server started.");
      if(num == 0)
      {
         MO_INFO("Servicing forever. CTRL-C to quit.");
      }
      {
         MO_INFO("Servicing approximately %" PRIu64 " connections.", num);
      }
   }
   else if(Exception::get() != NULL)
   {
      MO_ERROR("Server started with errors=%s\n",
         Exception::get()->getMessage());
   }

   // start timing
   pingPong.start();

   // either serve for limited time, or wait for lock
   uint32_t time = cfg["time"]->getUInt32();
   if(time != 0)
   {
      Thread::sleep(time);
   }
   else
   {
      pingPong.getLock().wait();
   }

   server.stop();
   MO_INFO("Server stopped.");

   // stop kernel engine
   k.getEngine()->stop();

   tr.passIfNoException();
}

static bool run(TestRunner& tr)
{
   if(tr.isTestEnabled("pong"))
   {
      runPingTest(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.pong.test", "1.0", mo_test_pong::run)
