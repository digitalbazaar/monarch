/*
 * Copyright (c) 2008-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/crypto/BigDecimal.h"
#include "monarch/crypto/AsymmetricKeyFactory.h"
#include "monarch/crypto/DigitalSignature.h"
#include "monarch/data/json/JsonReader.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/fiber/FiberScheduler.h"
#include "monarch/fiber/FiberMessageCenter.h"
#include "monarch/io/NullOutputStream.h"
#include "monarch/modest/Kernel.h"
#include "monarch/util/Timer.h"

#include <cstdlib>
#include <cmath>

using namespace std;
using namespace monarch::config;
using namespace monarch::crypto;
using namespace monarch::fiber;
using namespace monarch::data::json;
using namespace monarch::io;
using namespace monarch::modest;
using namespace monarch::rt;
using namespace monarch::test;
using namespace monarch::util;

static inline void iterate()
{
   BigDecimal bd1("80932149813491423134299827397162412482");
   BigDecimal bd2("23974321498129821741298721");
   bd1 = bd1 / bd2;
}

class TestFiber : public Fiber
{
public:
   int start;

public:
   TestFiber(int n)
   {
      start = n;
   };
   virtual ~TestFiber() {};

   virtual void run()
   {
      for(int i = start; i > 0; i--)
      {
//         printf("TestFiber %i doing iteration %i on thread %p\n",
//            getId(), i, Thread::currentThread());
         iterate();
         yield();
      }
   }
};

class TestFiberSleep : public Fiber
{
public:
   TestFiberSleep()
   {
   };
   virtual ~TestFiberSleep() {};

   virtual void run()
   {
      //printf("going to sleep...\n");
      sleep();
      //printf("awake!\n");
   }
};

class TestMessagableFiber : public MessagableFiber
{
public:
   int start;
   int expectMessages;

public:
   TestMessagableFiber(FiberMessageCenter* fmc, int n, int expectMsgs) :
      MessagableFiber(fmc)
   {
      start = n;
      expectMessages = expectMsgs;
   };
   virtual ~TestMessagableFiber() {};

   virtual void processMessages()
   {
      int count = start;
      int messages = 0;

      // test sleeping and waking up due to new messages
      //printf("sleeping...\n");
      sleep();
      //printf("awake!\n");

      while(count > 0)
      {
         FiberMessageQueue* msgs = getMessages();
         while(!msgs->empty())
         {
            DynamicObject msg = msgs->front();
            msgs->pop_front();
//            printf("Processing msg:\n%s\n",
//               JsonWriter::writeToString(msg).c_str());
            messages++;
            yield();
         }

         if(count > 0)
         {
            count--;
            iterate();
            yield();
         }
      }

      //printf("TestMessagableFiber %i received %i messages,exiting.\n",
      //   getId(), messages);
   }
};

class TestChildFiber : public MessagableFiber
{
public:
   FiberId parentId;

public:
   TestChildFiber(FiberMessageCenter* fmc, FiberId parent) :
      MessagableFiber(fmc),
      parentId(parent)
   {
   };
   virtual ~TestChildFiber() {};

   virtual void processMessages()
   {
      DynamicObject msg;
      msg["fiberId"] = getId();
      msg["wakeup"] = true;
      sendMessage(parentId, msg);
   }
};

class TestParentFiber : public MessagableFiber
{
public:
   TestParentFiber(FiberMessageCenter* fmc) :
      MessagableFiber(fmc)
   {
   };
   virtual ~TestParentFiber() {};

   virtual void processMessages()
   {
      TestChildFiber* child = new TestChildFiber(mMessageCenter, getId());
      FiberId childId = mScheduler->addFiber(child);
      sleep();

      FiberMessageQueue* msgs = getMessages();
      assert(msgs->size() == 1);
      DynamicObject msg = msgs->front();
      assert(msg["fiberId"]->getUInt32() == childId);
      assert(msg->hasMember("wakeup"));
      assert(msg["wakeup"]->getBoolean());
   }
};

static void runFiberTest(TestRunner& tr)
{
   tr.group("Fibers");

   tr.test("single fiber");
   {
      Kernel k;
      k.getEngine()->start();

      FiberScheduler fs;
      fs.start(&k, 1);

      TestFiber* fiber = new TestFiber(100);
      fs.addFiber(fiber);

      fs.waitForLastFiberExit(true);
      k.getEngine()->stop();
   }
   tr.passIfNoException();

   tr.test("many fibers");
   {
      Kernel k;
      k.getEngine()->start();

      FiberScheduler fs;

      // queue up some fibers prior to starting
      for(int i = 0; i < 1000; i++)
      {
         fs.addFiber(new TestFiber(20));
      }

      for(int i = 0; i < 400; i++)
      {
         fs.addFiber(new TestFiber(50));
      }

      uint64_t startTime = Timer::startTiming();
      fs.start(&k, 4);

      // add more fibers
      for(int i = 0; i < 20; i++)
      {
         fs.addFiber(new TestFiber(100));
      }

      fs.waitForLastFiberExit(true);
      printf("time=%g secs... ", Timer::getSeconds(startTime));

      k.getEngine()->stop();
   }
   tr.passIfNoException();

   tr.test("sleep fiber");
   {
      Kernel k;
      k.getEngine()->start();

      FiberScheduler fs;
      fs.start(&k, 1);

      FiberId id = fs.addFiber(new TestFiberSleep());

      // wait, and then wakeup sleeping fiber
      Thread::sleep(500);
      //printf("waking up fiber...\n");
      fs.wakeup(id);

      fs.waitForLastFiberExit(true);
      k.getEngine()->stop();
   }
   tr.passIfNoException();

   tr.test("messages");
   {
      Kernel k;
      k.getEngine()->start();

      FiberScheduler fs;
      FiberMessageCenter fmc;

      FiberId id;
      for(int i = 0; i < 50; i++)
      {
         MessagableFiber* fiber = new TestMessagableFiber(&fmc, 1000, 1000);
         id = fs.addFiber(fiber);
         fmc.registerFiber(fiber);
         DynamicObject msg;
         msg["helloId"] = i + 1;
         for(int n = 0; n < 1000; n++)
         {
            fmc.sendMessage(id, msg);
         }
      }

      uint64_t startTime = Timer::startTiming();
      fs.start(&k, 4);

      fs.waitForLastFiberExit(true);
      printf("time=%g secs... ", Timer::getSeconds(startTime));
      k.getEngine()->stop();
   }
   tr.passIfNoException();

   tr.test("parent/child fiber");
   {
      Kernel k;
      k.getEngine()->start();

      FiberScheduler fs;
      FiberMessageCenter fmc;

      fs.addFiber(new TestParentFiber(&fmc));

      uint64_t startTime = Timer::startTiming();
      fs.start(&k, 4);

      fs.waitForLastFiberExit(true);
      printf("time=%g secs... ", Timer::getSeconds(startTime));
      k.getEngine()->stop();
   }
   tr.passIfNoException();

   tr.ungroup();
}

class SpeedTestRunnable : public Runnable
{
protected:
   int count;
public:
   SpeedTestRunnable(int n) { count = n; };
   virtual ~SpeedTestRunnable() {};

   virtual void run()
   {
      while(--count >= 0)
      {
         iterate();
      }
   }
};

static void runFiberSpeedTest(TestRunner& tr)
{
   tr.group("Fiber speed");

   tr.test("300 threads,100 iterations");
   {
      Kernel k;
      k.getEngine()->getThreadPool()->setPoolSize(300);
      k.getEngine()->start();

      // queue up Operations
      OperationList opList;
      for(int i = 0; i < 300; i++)
      {
         RunnableRef r = new SpeedTestRunnable(100);
         Operation op(r);
         opList.add(op);
      }

      uint64_t startTime = Timer::startTiming();
      opList.queue(&k);
      opList.waitFor();
      printf("time=%g secs... ", Timer::getSeconds(startTime));

      k.getEngine()->stop();
   }
   tr.passIfNoException();

   tr.test("300 fibers,100 iterations");
   {
      Kernel k;
      k.getEngine()->start();

      FiberScheduler fs;

      // queue up fibers
      for(int i = 0; i < 300; i++)
      {
         fs.addFiber(new TestFiber(100));
      }

      uint64_t startTime = Timer::startTiming();
      fs.start(&k, 4);
      fs.waitForLastFiberExit(true);
      printf("time=%g secs... ", Timer::getSeconds(startTime));

      k.getEngine()->stop();
   }
   tr.passIfNoException();

   tr.test("10,000 fibers,3 iterations");
   {
      Kernel k;
      k.getEngine()->start();

      FiberScheduler fs;

      // queue up fibers
      for(int i = 0; i < 10000; i++)
      {
         fs.addFiber(new TestFiber(3));
      }

      uint64_t startTime = Timer::startTiming();
      fs.start(&k, 4);
      fs.waitForLastFiberExit(true);
      printf("time=%g secs... ", Timer::getSeconds(startTime));

      k.getEngine()->stop();
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runFiberSpeedTest2(TestRunner& tr)
{
   tr.group("Fiber speed 2");

   Timer timer;
   for(int fibers = 100; fibers <= 1000; fibers += 100)
   {
      for(int iterations = 10; iterations <= 100; iterations += 10)
      {
         char testname[100];
         snprintf(testname, 100, "%i Fibers,%i iterations",
            fibers, iterations);
         tr.test(testname);
         {
            Kernel k;
            k.getEngine()->start();

            FiberScheduler fs;

            // queue up fibers
            for(int i = 0; i < fibers; i++)
            {
               fs.addFiber(new TestFiber(iterations));
            }

            timer.start();
            fs.start(&k, 4);
            fs.waitForLastFiberExit(true);
            printf("time=%g secs... ", timer.getElapsedSeconds());

            k.getEngine()->stop();
         }
         tr.passIfNoException();
      }
   }

   tr.ungroup();
}

class ConcurrentSigner : public Fiber
{
protected:
   PrivateKeyRef mPrivateKey;
   PublicKeyRef mPublicKey;

public:
   ConcurrentSigner(PrivateKeyRef& privateKey, PublicKeyRef& publicKey)
   {
      mPrivateKey = privateKey;
      mPublicKey = publicKey;
   };
   virtual ~ConcurrentSigner() {};

   virtual void run()
   {
      string test =
         "POST /api/3.0/sva/contracts/media/2 HTTP/1.1localhost:19100";

      DigitalSignature* ds = new DigitalSignature(mPrivateKey);
      ds->update(test.c_str(), test.length());
      char sig[ds->getValueLength()];
      unsigned int length;
      ds->getValue(sig, length);
      delete ds;

      ds = new DigitalSignature(mPublicKey);
      ds->update(test.c_str(), test.length());
      bool verified = ds->verify(sig, length);
      delete ds;
      if(verified)
      {
         printf("VERIFIED!\n");
      }
      else
      {
         printf("NOT VERIFIED!\n");
      }
      //assert(verified);
   }
};

static void runConcurrentSigningTest(TestRunner& tr)
{
   tr.group("DigitalSignature fiber concurrency");

   // generate keys
   PrivateKeyRef privateKey;
   PublicKeyRef publicKey;
   AsymmetricKeyFactory afk;
   afk.createKeyPair("RSA", privateKey, publicKey);
   assertNoException();

   tr.test("10 fibers");
   {
      Kernel k;
      k.getEngine()->start();

      FiberScheduler fs;

      // queue up fibers
      int count = 10;
      for(int i = 0; i < count; i++)
      {
         fs.addFiber(new ConcurrentSigner(privateKey, publicKey));
      }

      printf("\n");
      Timer timer;
      timer.start();
      fs.start(&k, 4);
      fs.waitForLastFiberExit(true);
      printf("time=%g secs... ", timer.getElapsedSeconds());

      k.getEngine()->stop();
   }
   tr.passIfNoException();

   tr.ungroup();
}

/**
 * Make a DynamicObject with various content to stress test JSON reader/writer.
 *
 * @return test DynamicObject
 */
static DynamicObject makeJsonTestDyno1()
{
   DynamicObject d3;
   d3["a"] = 123;
   d3["b"] = true;
   d3["c"] = "sea";

   DynamicObject loremIpsum;
   loremIpsum =
      "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do "
      "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad "
      "minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip "
      "ex ea commodo consequat. Duis aute irure dolor in reprehenderit in "
      "voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur "
      "sint occaecat cupidatat non proident, sunt in culpa qui officia "
      "deserunt mollit anim id est laborum.";

   DynamicObject d;
   d["zeroth"] = false;
   d["first"] = "one";
   d["second"] = 2.0;
   d["third"] = 3;
   d["fourth"]->setType(Array);
   d["fourth"]->append() = d3.clone();
   d["fourth"]->append() = d3.clone();
   d["fourth"]->append() = d3.clone();
   d["fourth"]->append() = d3.clone();
   d["fifth"] = d3.clone();
   d["sixth"].setNull();
   d["seventh"] = loremIpsum.clone();
   d["eighth"]["one"] = loremIpsum.clone();
   d["eighth"]["two"] = loremIpsum.clone();
   d["eighth"]["three"] = loremIpsum.clone();
   d["eighth"]["four"] = loremIpsum.clone();
   d["ninth"] = "WUVT 90.7 FM - The Greatest Radio Station on Earth";

   return d;
}

/**
 * Decode a JSON string into a DynamicObject then encode to a null output
 * stream.
 *
 * @param s JSON string
 * @param slen length of s
 */
static void jsonReadWrite(const char* s, size_t slen)
{
   // decode json -> dyno
   JsonReader jr;
   DynamicObject d;
   JsonReader::readFromString(d, s, slen);
   assertNoException();

   // encode dyno -> json
   NullOutputStream os;
   JsonWriter jw;
   jw.setCompact(true);
   jw.write(d, &os);
   assertNoException();
}

/**
 * Fiber that will read and write a JSON string a number of times.
 */
class JsonRWFiber : public Fiber, public Runnable
{
protected:
   const char* mStr;
   size_t mStrlen;
   int mLoops;

public:
   JsonRWFiber(const char* str, int loops) :
      mStr(str),
      mStrlen(strlen(mStr)),
      mLoops(loops) {}

   virtual ~JsonRWFiber() {}

   virtual void run()
   {
      while(mLoops-- > 0)
      {
         jsonReadWrite(mStr, mStrlen);
         yield();
      }
   }
};

/**
 * Runnable that will read and write a JSON string a number of times.
 */
class JsonRWRunnable : public Runnable
{
protected:
   const char* mStr;
   size_t mStrlen;
   int mLoops;

public:
   JsonRWRunnable(const char* str, int loops) :
      mStr(str),
      mStrlen(strlen(mStr)),
      mLoops(loops) {}

   virtual ~JsonRWRunnable() {}

   virtual void run()
   {
      while(mLoops--)
      {
         jsonReadWrite(mStr, mStrlen);
      }
   }
};

static bool header = true;

/**
 * JSON test.
 * The "threads" mode is using 1 thread per operation.  To normalize this
 * behavior with "fibers" and "modest" the 'ops' parameter is used in "threads"
 * mode to control how many threads are used.
 *
 * @param mode "fibers", "modest", or "threads"
 * @param threads number of threads to run fibers or size of modest thread pool
 *        (not used for "threads" test)
 * @param ops number of fibers, modest operations, or operation threads.
 * @param oploops number of times to repeat each op
 * @param dyno id of dyno to use.  1=complex 2=simple
 * @param csv output in CSV format with '#' comments and spaces around data
 */
static void runJsonTest(
   TestRunner& tr,
   const char* mode, int threads, int ops, int oploops, int dyno, bool csv)
{
   string s;
   switch(dyno)
   {
      case 1:
      {
         /* somewhat complex JSON */
         DynamicObject d = makeJsonTestDyno1();
         s.assign(JsonWriter::writeToString(d, true));
         break;
      }
      case 2:
         /* trivial JSON */
         s.assign("{}");
         break;
      default:
         /* invalid JSON */
         s.assign("");
         break;
   }

   char name[200];
   snprintf(name, 200, "JSON r/w mode:%s threads:%d ops:%d",
      mode, threads, ops);
   uint64_t start_init = 0;
   uint64_t start_process = 0;
   uint64_t proc_dt = 0;
   uint64_t init_dt = 0;

   tr.test(name);
   if(strcmp(mode, "fibers") == 0)
   {
      Kernel k;
      k.getEngine()->start();

      FiberScheduler fs;

      start_init = Timer::startTiming();
      // queue up fibers
      for(int i = 0; i < ops; i++)
      {
         fs.addFiber(new JsonRWFiber(s.c_str(), oploops));
      }

      start_process = Timer::startTiming();
      fs.start(&k, threads);
      fs.waitForLastFiberExit(true);

      proc_dt = Timer::getMilliseconds(start_process);
      init_dt = start_process - start_init;

      k.getEngine()->stop();
   }
   else if(strcmp(mode, "modest") == 0)
   {
      Kernel k;
      k.getEngine()->getThreadPool()->setPoolSize(threads);

      // queue up Operations
      OperationList opList;
      start_init = Timer::startTiming();
      for(int i = 0; i < ops; i++)
      {
         RunnableRef r = new JsonRWRunnable(s.c_str(), oploops);
         Operation op(r);
         opList.add(op);
      }
      opList.queue(&k);

      start_process = Timer::startTiming();
      k.getEngine()->start();
      opList.waitFor();

      proc_dt = Timer::getMilliseconds(start_process);
      init_dt = start_process - start_init;

      k.getEngine()->stop();
   }
   else if(strcmp(mode, "threads") == 0)
   {
      // using 1 thread per op
      Thread* t[ops];
      JsonRWRunnable* r[ops];

      // queue up Operations
      start_init = Timer::startTiming();
      for(int i = 0; i < ops; i++)
      {
         r[i] = new JsonRWRunnable(s.c_str(), oploops);
         t[i] = new Thread(r[i]);
      }

      start_process = Timer::startTiming();
      for(int i = 0; i < ops; i++)
      {
         t[i]->start();
      }
      for(int i = 0; i < ops; i++)
      {
         t[i]->join();
      }

      proc_dt = Timer::getMilliseconds(start_process);
      init_dt = start_process - start_init;

      for(int i = 0; i < ops; i++)
      {
         delete r[i];
         delete t[i];
      }
   }
   else
   {
      printf("BAD MODE: %s\n", mode);
   }

   // handle thread mem issue by making process time 0
   if(Exception::isSet() &&
      strcmp(Exception::get()->getType(),
         "monarch.rt.Thread.InsufficientMemory") == 0)
   {
      proc_dt = 0;
      Exception::clear();
   }

   if(tr.getOutputLevel() == TestRunner::None)
   {
      const char* comment = csv ? "#" : "";
      const char* sep = csv ? "," : " ";
      //if(!csv && header)
      if(header)
      {
         printf(
            "%1s%8s%s%9s%s"
            "%8s%s%9s%s"
            "%8s%s%10s%s"
            "%9s\n",
            comment, "threads", sep, "ops", sep,
            "init (s)", sep, "opsi/ms", sep,
            "run (s)", sep, "ops/s", sep,
            "total (s)");
         header = false;
      }
      printf(
         "%9d%s%9d%s"
         "%8.3f%s%9.3f%s"
         "%8.3f%s%10.3f%s"
         "%9.3f\n",
         threads, sep, ops, sep,
         init_dt/1000.0, sep, ops/(double)init_dt, sep,
         proc_dt/1000.0, sep, ops/(proc_dt/1000.0), sep,
         (init_dt + proc_dt)/1000.0);
   }

   tr.passIfNoException();
}

/**
 * Calculate size of a test array.
 *
 * @param lin true if linear, false if log
 * @param min min value
 * @param max max value
 * @param mag log base 10 magnitude max
 */
static int calculateTestArraySize(bool lin, int min, int max, int mag)
{
   int rval;
   if(lin)
   {
      rval = max - min + 1;
   }
   else
   {
      // full from 1 to 10^mag
      //int full = 9*mag + 1;
      // FIXME find algorithm for num vals between min/max for log 10 n
      rval = 0;
      // loop over base 10 log vals from 1 to 10*tmag
      int p = 1;
      for(int e = 0; e < mag; e++)
      {
         for(int n = 0; n < 9; n++)
         {
            int val = (n + 1) * p;
            if(val >= min && val <= max)
            {
               rval++;
            }
         }
         p = p * 10;
      }

      // add final value if needed
      int last = (int)pow((double)10, mag);
      if(last <= max)
      {
         rval++;
      }
   }
   return rval;
}

/**
 * Fill a test array.  If lin, then from min to max, else all log base 10
 * values from 1 to 10^mag between min and max.
 *
 * @param d the test array to fill
 * @param lin true if linear, false if log
 * @param min min value
 * @param max max value
 * @param mag log base 10 magnitude max
 */
static void fillTestArray(int* d, bool lin, int min, int max, int mag)
{
   if(lin)
   {
      for(int i = min; i <= max; i++)
      {
         d[i - min] = i;
      }
   }
   else
   {
      // current index
      int i = 0;
      // base 10 log vals from 1 to 10*tmag
      int p = 1;
      for(int e = 0; e < mag; e++)
      {
         for(int n = 0; n < 9; n++)
         {
            int val = (n + 1) * p;
            if(val >= min && val <= max)
            {
               d[i] = val;
               i++;
            }
         }
         p = p * 10;
      }

      // add final value if needed
      int last = (int)pow((double)10, mag);
      if(last <= max)
      {
         d[i] = last;
      }
   }
}

/**
 * Runs speed test.
 *
 * Options:
 * --test all - run all tests
 * --test sign - signing test
 * --test json - run one json encode/decode test
 * --test jsonmatrix - run a matrix of json encode/decode tests
 * --option loops <n> - number of times to run each individual test
 * --option dyno 1 - complex dynamic object
 * --option dyno 2 - trivial "{}" dynamic object
 * --option csv true - output in CSV format
 * --option mode fibers - use fibers for ops with 'threads' threads
 * --option mode modest - use modest operations for ops with 'threads'
 *                        thread pool size
 * --option mode threads - use 'threads' threads for ops
 * --option threads <n> - how many threads to use (direct or pool size)
 * --option ops <n> - how many operations to perform
 * --option oploops <n> - how many times to run each operation
 *
 * For jsonmatrix:
 * For the threads (t) and operations (o) parameters an array will be
 * created of test values.  Then each combination of t and o values will be
 * tested.  The values will range from 1 to [t,o]max on a base 10 log scale
 * by default.  The min and max values can be specified with [t,o]min and
 * [t,o]max.  If [t,o]lin is true then the values will be linear between min
 * and max.
 *
 * --option tmag <n> - max log thread magnitude
 * --option tmin <n> - min number of threads
 * --option tmax <n> - max number of threads
 * --option tlin <true|false> - if true, use linear scale from tmin to tmax
 * (similar for operations via omag/omin/omax/olin)
 */
static int runFiberCompareTest(TestRunner& tr)
{
   Config cfg = tr.getApp()->getConfig();
   bool all = tr.isTestEnabled("all");

   if(/*all ||*/
      tr.isTestEnabled("json") ||
      tr.isTestEnabled("jsonmatrix"))
   {
      // number of loops for each test
      int loops = cfg->hasMember("loops") ? cfg["loops"]->getInt32() : 1;
      // number of loops in each op (can be used to increase cpu load)
      int oploops =
         cfg->hasMember("oploops") ? cfg["oploops"]->getInt32() : 1;
      // dyno to use (check code)
      int dyno = cfg->hasMember("dyno") ? cfg["dyno"]->getInt32() : 1;
      // CSV output mode
      bool csv = cfg->hasMember("csv") ? cfg["csv"]->getBoolean() : false;
      // test mode: fibers, modest, or threads
      const char* mode = cfg->hasMember("mode") ?
         cfg["mode"]->getString() : "fibers";

      if(all || tr.isTestEnabled("json"))
      {
         // number of threads
         int threads = cfg->hasMember("threads") ?
            cfg["threads"]->getInt32() : 1;
         // number of ops - (fibers, operations)
         int ops = cfg->hasMember("ops") ?
            cfg["ops"]->getInt32() : 1;
         for(int i = 0; i < loops; i++)
         {
            runJsonTest(tr, mode, threads, ops, oploops, dyno, csv);
         }
      }

      if(all || tr.isTestEnabled("jsonmatrix"))
      {
         bool tlin = cfg->hasMember("tlin") ?
            cfg["tlin"]->getBoolean() : false;
         int tmag = cfg->hasMember("tmag") ?
            cfg["tmag"]->getInt32() : 1;
         int tmin = cfg->hasMember("tmin") ?
            cfg["tmin"]->getInt32() : 1;
         int tmax = cfg->hasMember("tmax") ?
            cfg["tmax"]->getInt32() :
               (tlin ? 10 : (int)pow((double)10, tmag));

         bool olin = cfg->hasMember("olin") ?
            cfg["olin"]->getBoolean() : false;
         int omag = cfg->hasMember("omag") ?
            cfg["omag"]->getInt32() : 1;
         int omin = cfg->hasMember("omin") ?
            cfg["omin"]->getInt32() : 1;
         int omax = cfg->hasMember("omax") ?
            cfg["omax"]->getInt32() :
               (olin ? 10 : (int)pow((double)10, omag));

         // make the thread and ops count arrays
         int tsize = calculateTestArraySize(tlin, tmin, tmax, tmag);
         int td[tsize];
         fillTestArray(td, tlin, tmin, tmax, tmag);

         int osize = calculateTestArraySize(olin, omin, omax, omag);
         int od[osize];
         fillTestArray(od, olin, omin, omax, omag);

         // matrix of threads vs ops
         for(int ti = 0; ti < tsize; ti++)
         {
            for(int oi = 0; oi < osize; oi++)
            {
               for(int i = 0; i < loops; i++)
               {
                  //printf("test: t:%d o:%d\n", td[ti], od[oi]);
                  runJsonTest(tr, mode, td[ti], od[oi], oploops, dyno, csv);
               }
            }
         }
      }
   }

   return 0;
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runFiberTest(tr);
      runFiberSpeedTest(tr);
      //runFiberSpeedTest2(tr);
   }
   if(tr.isTestEnabled("sign"))
   {
      runConcurrentSigningTest(tr);
   }
   if(tr.isTestEnabled("fiber-compare"))
   {
      runFiberCompareTest(tr);
   }
   return true;
}

MO_TEST_MODULE_FN("monarch.tests.fiber.test", "1.0", run)
