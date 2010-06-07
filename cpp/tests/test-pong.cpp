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
#include "monarch/io/InputStream.h"
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
#include "monarch/net/Url.h"
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
 *   ./monarch-run pong
 *
 * See configs/apps/pong.config.in for option defaults. Use standard monarch
 * "--json-option key=value" option to adjust options.
 *
 *    pong.chunked=<bool>: use chunked encoding
 *    pong.dynoStats=<bool>: return DynamicObject stats with regular stats
 *    pong.num=<int32>: number of connections to service
 *    pong.port=<int32>: port to serve on
 *    pong.ssl=<bool>: use SSL
 *    pong.time=<int32>: milliseconds to run the test
 *    pong.threadStackSize=<int32>: set stack size
 *    pong.threads=<int32>: set number of threads
 *    pong.maxConnections=<int32>: set max number of connections
 *    pong.backlog=<int32>: set connection backlog queue size
 *
 * Endpoints:
 *    /: return "204 No Content"
 *    /pong: return "Pong!"
 *    /data[/size]: return a specified number of bytes of content. default=0.
 *    /stats: return JSON object with various
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

class NoContentServicer : public HttpRequestServicer
{
protected:
   PingPong* mPingPong;

public:
   NoContentServicer(PingPong* pingPong, const char* path) :
      HttpRequestServicer(path),
      mPingPong(pingPong)
   {
   }

   virtual ~NoContentServicer()
   {
   }

   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
      response->getHeader()->setStatus(204, "No Content");
      response->getHeader()->setField("Content-Length", 0);
      response->getHeader()->setField("Connection", "close");
      response->sendHeader();
      mPingPong->service();
   }
};

class PingServicer : public HttpRequestServicer
{
protected:
   PingPong* mPingPong;
   const char* mContent;
   bool mChunked;

public:
   PingServicer(PingPong* pingPong, const char* path) :
      HttpRequestServicer(path),
      mPingPong(pingPong)
   {
      mChunked = pingPong->getConfig()["chunked"]->getBoolean();
   }

   virtual ~PingServicer()
   {
   }

   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
      const char* str = "Pong!";
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
      ByteArrayInputStream bais(str, len);
      response->sendBody(&bais, NULL);
      mPingPong->service();
   }
};

class ConstByteInputStream : public InputStream
{
protected:
   int mLength;
   char mData[4096];

public:
   ConstByteInputStream(char b, int length) :
      mLength(length)
   {
      memset(mData, b, min(mLength, 4096));
   }

   int read(char* b, int length)
   {
      // max is 4k or remaining, let higher levels loop
      int max = min(mLength, 4096);
      // return max or length
      int rval = min(max, length);
      // reduce remaining
      mLength -= rval;
      memcpy(b, mData, rval);
      return rval;
   }
};

class DataServicer : public HttpRequestServicer
{
protected:
   PingPong* mPingPong;
   string mPath;
   bool mChunked;

public:
   DataServicer(PingPong* pingPong, const char* path) :
      HttpRequestServicer(path),
      mPingPong(pingPong),
      mPath(path)
   {
      mChunked = pingPong->getConfig()["chunked"]->getBoolean();
      if(mPath.length() > 0 && mPath[mPath.length()] != '/')
      {
         mPath.push_back('/');
      }
   }

   virtual ~DataServicer()
   {
   }

   virtual void serviceRequest(
      HttpRequest* request, HttpResponse* response)
   {
      // get length param from value after root path, else 0
      const char* path = request->getHeader()->getPath();
      Url url(path);
      DynamicObject tokens;
      url.getTokenizedPath(tokens, mPath.c_str());
      int len = 0;
      if(tokens->length() > 0)
      {
         len = tokens[0]->getInt32();
      }

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
      // send const bytes of a specified length
      ConstByteInputStream cbis('.', len);
      response->sendBody(&cbis, NULL);
      mPingPong->service();
   }
};

class StatsServicer : public HttpRequestServicer
{
protected:
   PingPong* mPingPong;

public:
   StatsServicer(PingPong* pingPong, const char* path) :
      HttpRequestServicer(path),
      mPingPong(pingPong)
   {
   }

   virtual ~StatsServicer()
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

class QuitServicer : public HttpRequestServicer
{
protected:
   PingPong* mPingPong;

public:
   QuitServicer(PingPong* pingPong, const char* path) :
      HttpRequestServicer(path),
      mPingPong(pingPong)
   {
   }

   virtual ~QuitServicer()
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
   server.setMaxConnectionCount(cfg["maxConnections"]->getInt32());
   InternetAddress address("0.0.0.0", cfg["port"]->getUInt32());

   // create SSL/generic http connection servicer
   HttpConnectionServicer hcs;
//   SslContext context;
//   SslSocketDataPresenter presenter1(&context);
//   NullSocketDataPresenter presenter2;
//   SocketDataPresenterList list(false);
//   list.add(&presenter1);
//   list.add(&presenter2);
   //server.addConnectionService(&address, &hcs);//, &list);
   server.addConnectionService(
      &address, &hcs, NULL, "pong",
      cfg["maxConnections"]->getInt32(),
      cfg["backlog"]->getInt32());

   // create test http request servicer
   NoContentServicer noContentSrv(&pingPong, "/");
   hcs.addRequestServicer(&noContentSrv, false);

   PingServicer ping(&pingPong, "/pong");
   hcs.addRequestServicer(&ping, false);

   DataServicer data(&pingPong, "/data");
   hcs.addRequestServicer(&data, false);

   StatsServicer stats(&pingPong, "/stats");
   hcs.addRequestServicer(&stats, false);

   QuitServicer quit(&pingPong, "/quit");
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
