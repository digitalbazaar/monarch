/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/crypto/BigDecimal.h"
#include "db/crypto/AsymmetricKeyFactory.h"
#include "db/crypto/DigitalSignature.h"
#include "db/data/json/JsonWriter.h"
#include "db/fiber/FiberScheduler.h"
#include "db/modest/Kernel.h"
#include "db/util/Timer.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace std;
using namespace db::crypto;
using namespace db::fiber;
using namespace db::data::json;
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
         exit();
      }
   }
   
   virtual void interrupted()
   {
      printf("\nTest fiber '%d' interrupted after %d iterations!\n",
         getId(), start - count);
      exit();
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

class DbFiberTester : public db::test::Tester
{
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
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      runConcurrentSigningTest(tr);
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbFiberTester)
#endif
