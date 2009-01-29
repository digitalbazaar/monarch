/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/fiber/FiberScheduler.h"
#include "db/modest/Kernel.h"
#include "db/util/Timer.h"

#include <sstream>
#include <cstdlib>
#include <cmath>

using namespace std;
using namespace db::config;
using namespace db::fiber;
using namespace db::modest;
using namespace db::rt;
using namespace db::test;
using namespace db::util;

class TestFiber : public Fiber
{
public:
   int count;
   
public:
   TestFiber(int n)
   {
      count = n;
   };
   virtual ~TestFiber() {};
   
   virtual void run()
   {
      printf("Running test fiber '%d'\n", getId());
      
      int i = 0;
      for(; i < count; i++)
      {
         printf("Test fiber '%d' yielding...\n", getId());
         yield();
         printf("Test fiber '%d' continuing.\n", getId());
      }
      
      printf("Test fiber '%d' done with '%d' iterations.\n", getId(), i);
   }
};

void runFiberYieldTest(TestRunner& tr)
{
   tr.group("Fiber Yield");
   
   tr.test("10 yielding fibers/10 iterations");
   {
      Kernel k;
      k.getEngine()->start();
      
      FiberScheduler fs;
      
      // queue up some fibers prior to starting
      for(int i = 0; i < 10; i++)
      {
         fs.addFiber(new TestFiber(10));
      }
      
      uint64_t startTime = Timer::startTiming();
      fs.start(&k, 1);
      
      fs.waitForLastFiberExit(true);
      printf("time=%g secs... ", Timer::getSeconds(startTime));
      
      k.getEngine()->stop();
   }
   tr.passIfNoException();
   
   tr.ungroup();
}

class DbFiberYieldTester : public db::test::Tester
{
public:
   DbFiberYieldTester()
   {
      setName("fiber yield");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      runFiberYieldTest(tr);
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbFiberYieldTester)
#endif
