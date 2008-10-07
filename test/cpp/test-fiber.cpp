/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/crypto/BigDecimal.h"
#include "db/crypto/AsymmetricKeyFactory.h"
#include "db/crypto/DigitalSignature.h"
#include "db/data/json/JsonReader.h"
#include "db/data/json/JsonWriter.h"
#include "db/fiber/FiberScheduler.h"
#include "db/io/NullOutputStream.h"
#include "db/modest/Kernel.h"
#include "db/util/Timer.h"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cmath>

using namespace std;
using namespace db::config;
using namespace db::crypto;
using namespace db::fiber;
using namespace db::data::json;
using namespace db::io;
using namespace db::modest;
using namespace db::rt;
using namespace db::test;
using namespace db::util;

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
   int count;
   int dummy;
   int* msgs;
public:
   TestFiber(int n, int* m = NULL)
   {
      start = n;
      count = n;
      dummy = 0;
      
      msgs = (m == NULL ? &dummy : m);
   };
   virtual ~TestFiber() {};
   
   virtual void processMessage(DynamicObject& msg)
   {
      //printf("Processing msg:\n%s\n",
      //   JsonWriter::writeDynamicObjectToString(msg).c_str());
      (*msgs)++;
   }
   
   virtual void run()
   {
      //printf("Running test fiber '%d'\n", getId());
      
      iterate();
      
      if(--count == 0)
      {
         //printf("total iterations: %d\n", start);
         exit();
      }
//      else if(count == 99990)
//      {
//         sleep();
//      }
   }
   
   virtual void interrupted()
   {
      printf("\nTest fiber '%d' interrupted after %d iterations!\n",
         getId(), start - count);
      exit();
      //resume();
   }
};

void runFiberTest(TestRunner& tr)
{
   tr.group("Fibers");
   
   tr.test("single fiber");
   {
      Kernel k;
      k.getEngine()->start();
      
      FiberScheduler fs;
      fs.start(&k, 2);
      
      TestFiber* fiber = new TestFiber(10);
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
      fs.start(&k, 2);
      
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
   
   tr.test("messages");
   {
      Kernel k;
      k.getEngine()->start();
      
      FiberScheduler fs;
      
      FiberId id;
      for(int i = 0; i < 50; i++)
      {
         id = fs.addFiber(new TestFiber(1000));
         DynamicObject msg;
         msg["helloId"] = i + 1;
         for(int n = 0; n < 1000; n++)
         {
            fs.sendMessage(id, msg);
         }
      }
      
      uint64_t startTime = Timer::startTiming();
      fs.start(&k, 2);
      
      int msgs = 0;
      for(int i = 0; i < 20; i++)
      {
         id = fs.addFiber(new TestFiber(1000, &msgs));
         DynamicObject msg;
         msg["helloId"] = i + 1;
         for(int n = 0; n < 10000; n++)
         {
            fs.sendMessage(id, msg);
         }
      }
      
      fs.waitForLastFiberExit(true);
      printf("msgs=%d, time=%g secs... ", msgs, Timer::getSeconds(startTime));
      k.getEngine()->stop();
      
      // assert all messages were delivered
      assert(msgs == 200000);
   }
   tr.passIfNoException();
   
   tr.test("interrupted fiber");
   {
      Kernel k;
      k.getEngine()->start();
      
      FiberScheduler fs;
      fs.start(&k, 2);
      
      TestFiber* fiber = new TestFiber(100000);
      FiberId id = fs.addFiber(fiber);
      Thread::sleep(10);
      fs.interrupt(id);
      
      fs.waitForLastFiberExit(true);
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

void runSpeedTest(TestRunner& tr)
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
      fs.start(&k, 2);
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
      fs.start(&k, 2);
      fs.waitForLastFiberExit(true);
      printf("time=%g secs... ", Timer::getSeconds(startTime));
      
      k.getEngine()->stop();
   }
   tr.passIfNoException();
   
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
      
      exit();
   }
};

void runConcurrentSigningTest(TestRunner& tr)
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
      uint64_t startTime = Timer::startTiming();
      fs.start(&k, 2);
      fs.waitForLastFiberExit(true);
      printf("time=%g secs... ", Timer::getSeconds(startTime));
      
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
   JsonReader::readDynamicObjectFromString(d, s, slen);
   assertNoException();
   
   // encode dyno -> json
   NullOutputStream os;
   JsonWriter jw;
   jw.setCompact(true);
   jw.write(d, &os);
   assertNoException();
}

/**
 * Fiber that will read and write a JSON string once then exit.
 */
class JsonRWFiber : public Fiber
{
protected:
   const char* mStr;
   size_t mStrlen;
   
public:
   JsonRWFiber(const char* str) :
      mStr(str),
      mStrlen(strlen(mStr)) {}
   
   virtual ~JsonRWFiber() {}
   
   virtual void run()
   {
      //printf("[%d] JsonFiber running.\n", getId());
      jsonReadWrite(mStr, mStrlen);
      exit();
   }
   
   virtual void interrupted()
   {
      printf("[%d] JsonFiber interrupted, exiting.\n", getId());
      exit();
   }
};

/**
 * Runnable that will read and write a JSON string once then exit.
 */
class JsonRWRunnable : public Runnable
{
protected:
   const char* mStr;
   size_t mStrlen;
   
public:
   JsonRWRunnable(const char* str) :
      mStr(str),
      mStrlen(strlen(mStr)) {}
   
   virtual ~JsonRWRunnable() {}
   
   virtual void run()
   {
      jsonReadWrite(mStr, mStrlen);
   }
};

static bool header = true;

/**
 * JSON test
 * 
 * @param mode "fibers", "modest", or "threads"
 * @param threads number of threads to run fibers, size of modest thread pool
 *        or number of threads.
 * @param ops number of fibers or modest operations.  not used for threads.
 * @param dyno id of dyno to use.  1=complex 2=simple
 * @param csv output in CSV format with '#' comments and spaces around data
 */
void runJsonTest(TestRunner& tr,
   const char* mode, int threads, int ops, int dyno, bool csv)
{
   string s;
   switch(dyno)
   {
      case 1:
      {
         /* somewhat complex JSON */
         DynamicObject d = makeJsonTestDyno1();
         s.assign(JsonWriter::writeDynamicObjectToString(d, true));
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
         fs.addFiber(new JsonRWFiber(s.c_str()));
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
         RunnableRef r = new JsonRWRunnable(s.c_str());
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
      Thread* t[threads];
      
      // queue up Operations
      start_init = Timer::startTiming();
      JsonRWRunnable r(s.c_str());
      for(int i = 0; i < threads; i++)
      {
         t[i] = new Thread(&r);
      }
      
      start_process = Timer::startTiming();
      for(int i = 0; i < threads; i++)
      {
         t[i]->start();
      }
      for(int i = 0; i < threads; i++)
      {
         t[i]->join();
      }
      
      proc_dt = Timer::getMilliseconds(start_process);
      init_dt = start_process - start_init;
      
      for(int i = 0; i < threads; i++)
      {
         delete t[i];
      }
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

class DbFiberTester : public db::test::Tester
{
protected:
   /**
    * Calculate size of a test array.
    * 
    * @param lin true if linear, false if log
    * @param min min value
    * @param max max value
    * @param mag log base 10 magnitude max
    */
   virtual int calculateTestArraySize(bool lin, int min, int max, int mag)
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
   virtual void fillTestArray(int* d, bool lin, int min, int max, int mag)
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
   
public:
   DbFiberTester()
   {
      setName("fiber");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runFiberTest(tr);
      runSpeedTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
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
   virtual int runInteractiveTests(TestRunner& tr)
   {
      Config& cfg = tr.getApp()->getConfig();
      const char* test = cfg["db.test.Tester"]["test"]->getString();
      bool all = (strcmp(test, "all") == 0);
      
      if(all || (strcmp(test, "sign") == 0))
      {
         runConcurrentSigningTest(tr);
      }
      
      if(/*all ||*/
         (strcmp(test, "json") == 0) ||
         (strcmp(test, "jsonmatrix") == 0))
      {
         // number of loops for each test
         int loops = cfg->hasMember("loops") ? cfg["loops"]->getInt32() : 1;
         // dyno to use (check code)
         int dyno = cfg->hasMember("dyno") ? cfg["dyno"]->getInt32() : 1;
         // CSV output mode
         bool csv = cfg->hasMember("csv") ? cfg["csv"]->getBoolean() : false;
         // test mode: fibers, modest, or threads
         const char* mode = cfg->hasMember("mode") ?
            cfg["mode"]->getString() : "fibers";
         
         if(all || (strcmp(test, "json") == 0))
         {
            // number of threads
            int threads = cfg->hasMember("threads") ?
               cfg["threads"]->getInt32() : 1;
            // number of ops - (fibers, operations)
            int ops = cfg->hasMember("ops") ?
               cfg["ops"]->getInt32() : 1;
            for(int i = 0; i < loops; i++)
            {
               runJsonTest(tr, mode, threads, ops, dyno, csv);
            }
         }
         
         if(all || (strcmp(test, "jsonmatrix") == 0))
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
                     runJsonTest(tr, mode, td[ti], od[oi], dyno, csv);
                  }
               }
            }
         }
      }
      
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbFiberTester)
#endif
