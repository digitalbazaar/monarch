/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/data/json/JsonWriter.h"
#include "db/fiber/FiberScheduler.h"
#include "db/modest/Kernel.h"
#include "db/util/Timer.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

using namespace std;
using namespace db::fiber;
using namespace db::data::json;
using namespace db::modest;
using namespace db::rt;
using namespace db::test;
using namespace db::util;

class TestFiber : public Fiber
{
protected:
   int count;
public:
   TestFiber(int n) { count = n; };
   virtual ~TestFiber() {};
   
   virtual void processMessage(DynamicObject& msg)
   {
      printf("Processing msg:\n%s\n",
         JsonWriter::writeDynamicObjectToString(msg).c_str());
   }
   
   virtual void run()
   {
      printf("Running test fiber '%d'\n", getId());
      
      if(--count == 0)
      {
         exit();
      }
   }
};

void runFiberTest(TestRunner& tr)
{
   tr.group("Fibers");

#if 0
   tr.test("single fiber");
   {
      Kernel k;
      k.getEngine()->start();
      
      FiberScheduler fs;
      fs.start(&k, 2);
      
      TestFiber* fiber = new TestFiber(10);
      fs.addFiber(fiber);
      
      fs.stopOnLastFiberExit();
      k.getEngine()->stop();
   }
   tr.passIfNoException();
#endif
   
   tr.test("many fibers");
   {
      Kernel k;
      k.getEngine()->start();
      
      FiberScheduler fs;
      
      // queue up some fibers prior to starting
      for(int i = 0; i < 100; i++)
      {
         fs.addFiber(new TestFiber(20));
      }
      
      for(int i = 0; i < 40; i++)
      {
         fs.addFiber(new TestFiber(50));
      }
      
      fs.start(&k, 2);
      
      // add more fibers
      for(int i = 0; i < 20; i++)
      {
         fs.addFiber(new TestFiber(20));
      }
      
      // FIXME: uncomment line below
      //fs.stopOnLastFiberExit();
      
      // FIXME: remove 2 lines below
      Thread::sleep(2000);
      fs.stop();
      
      k.getEngine()->stop();
   }
   tr.passIfNoException();
   
#if 0
   tr.test("messages");
   {
      Kernel k;
      k.getEngine()->start();
      
      FiberScheduler fs;
      
      for(int i = 0; i < 20; i++)
      {
         fs.addFiber(new TestFiber(10));
         DynamicObject msg;
         msg["helloId"] = i + 1;
         fs.sendMessage(i + 1, msg);
      }
      
      fs.start(&k, 2);
      fs.stopOnLastFiberExit();
      k.getEngine()->stop();
   }
   tr.passIfNoException();
#endif
   
   tr.ungroup();
}

class SpeedFiber : public Fiber
{
public:
   SpeedFiber() {};
   virtual ~SpeedFiber() {};
   
   virtual void run()
   {
      printf("Running speed fiber '%d'\n", getId());
   }
};

void runSpeedTest(TestRunner& tr)
{
   tr.group("Fiber speed");
   
   tr.test("1,000,000 fibers");
   {
      // FIXME: do it
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
      //runSpeedTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbFiberTester)
#endif
