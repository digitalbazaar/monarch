/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include <iostream>

#include "db/test/Test.h"
#include "db/test/Tester.h"
#include "db/test/TestRunner.h"
#include "db/rt/Object.h"
#include "db/rt/Runnable.h"
#include "db/rt/Thread.h"
#include "db/rt/Semaphore.h"
#include "db/rt/SharedLock.h"
#include "db/rt/System.h"
#include "db/rt/JobDispatcher.h"

using namespace std;
using namespace db::test;
using namespace db::rt;

void runTimeTest(TestRunner& tr)
{
   tr.test("Time");
   
   unsigned long long start = System::getCurrentMilliseconds();
   
   cout << "Time start=" << start << endl;
   
   unsigned long long end = System::getCurrentMilliseconds();
   
   cout << "Time end=" << end << endl;
   
   tr.pass();
}

class TestRunnable : public virtual Object, public Runnable
{
public:
   bool mustWait;
   
   TestRunnable()
   {
      mustWait = true;
   }
   
   virtual ~TestRunnable()
   {
   }
   
   virtual void run()
   {
      Thread* t = Thread::currentThread();
      string name = t->getName();
      //cout << name << ": This is a TestRunnable thread,addr=" << t << endl;
      
      if(name == "Thread 1")
      {
         //cout << "Thread 1 Waiting for interruption..." << endl;
         
         lock();
         {
            lock();
            lock();
            lock();
            // thread 1 should be interrupted
            bool interrupted = !wait();
            assert(interrupted);
            unlock();
            unlock();
            unlock();
         }
         unlock();
         
//         if(Thread::interrupted())
//         {
//            cout << "Thread 1 Interrupted. Exception message="
//                 << e->getMessage() << endl;
//         }
//         else
//         {
//            cout << "Thread 1 Finished." << endl;
//         }
      }
      else if(name == "Thread 2")
      {
         //cout << "Thread 2 Finished." << endl;
      }
      else if(name == "Thread 3")
      {
         //cout << "Thread 3 Waiting for Thread 5..." << endl;
         
         lock();
         lock();
         lock();
         {
            //cout << "Thread 3 starting wait..." << endl;
            while(mustWait)
            {
               // thread 3 should be notified, not interrupted
               bool interrupted = !wait(5000);
               assert(!interrupted);
            }
            //cout << "Thread 3 Awake!" << endl;
         }
         unlock();
         unlock();
         unlock();
         
//         if(Thread::interrupted())
//         {
//            cout << "Thread 3 Interrupted." << endl;
//         }
//         else
//         {
//            cout << "Thread 3 Finished." << endl;
//         }         
      }
      else if(name == "Thread 4")
      {
         //cout << "Thread 4 Finished." << endl;
      }
      else if(name == "Thread 5")
      {
         //cout << "Thread 5 waking up a thread..." << endl;
         
         lock();
         lock();
         lock();
         lock();
         {
            // wait for a moment
            Thread::sleep(100);
            mustWait = false;
            //cout << "Thread 5 notifying a thread..." << endl;
            notifyAll();
            //cout << "Thread 5 notified another thread." << endl;
         }
         unlock();
         unlock();
         unlock();
         unlock();
         
         //cout << "Thread 5 Finished." << endl;
      }
   }
};

void runThreadTest(TestRunner& tr)
{
   tr.test("Thread");
   
   //cout << "Running Thread Test" << endl << endl;
   
   TestRunnable r1;
   Thread t1(&r1, "Thread 1");
   Thread t2(&r1, "Thread 2");
   Thread t3(&r1, "Thread 3");
   Thread t4(&r1, "Thread 4");
   Thread t5(&r1, "Thread 5");
   
   //cout << "Threads starting..." << endl;
   
   size_t stackSize = 131072;
   t1.start(stackSize);
   t2.start(stackSize);
   t3.start(stackSize);
   t4.start(stackSize);
   t5.start(stackSize);
   
   t1.interrupt();
   
   t2.join();
   t3.join();
   t1.join();
   t4.join();
   t5.join();
   
   tr.pass();
   
   //cout << endl << "Thread Test complete." << endl;
}

class TestJob : public Runnable
{
public:
   string mName;
   
   TestJob(const string& name)
   {
      mName = name;
   }
   
   virtual ~TestJob()
   {
   }
   
   virtual void run()
   {
      //cout << endl << "TestJob: Running a job,name=" << mName << endl;
      
      if(mName == "1")
      {
         Thread::sleep(375);
      }
      else if(mName == "2")
      {
         Thread::sleep(125);
      }
      else
      {
         Thread::sleep(125);
      }
      
      //cout << endl << "TestJob: Finished a job,name=" << mName << endl;
   }
};

void runThreadPoolTest(TestRunner& tr)
{
   tr.test("ThreadPool");
   
   Exception::clearLast();
   
   // create a thread pool
   ThreadPool pool(3);
   
   // create jobs
   TestJob job1("1");
   TestJob job2("2");
   TestJob job3("3");
   TestJob job4("4");
   TestJob job5("5");
   
   // run jobs
   pool.runJob(job1);
   pool.runJob(job2);
   pool.runJob(job3);
   pool.runJob(job4);
   pool.runJob(job5);
   
   // wait
   Thread::sleep(1250);
   
   // terminate all threads
   pool.terminateAllThreads();
   
   tr.passIfNoException();
}

void runJobDispatcherTest(TestRunner& tr)
{
   tr.test("JobDispatcher");
   
   Exception::clearLast();
   
   // create a job dispatcher
   //JobDispatcher jd;
   ThreadPool pool(3);
   JobDispatcher jd(&pool, false);
   
   // create jobs
   TestJob job1("1");
   TestJob job2("2");
   TestJob job3("3");
   TestJob job4("4");
   TestJob job5("5");
   TestJob job6("6");
   
   // queue jobs
   jd.queueJob(job1);
   jd.queueJob(job2);
   jd.queueJob(job3);
   jd.queueJob(job4);
   jd.queueJob(job5);
   jd.queueJob(job6);
   
   // start dispatching
   jd.startDispatching();
   
   // wait
   Thread::sleep(1250);
   
   // stop dispatching
   jd.stopDispatching();      
   
   tr.passIfNoException();
}

class SharedLockRunnable : public Runnable
{
public:
   SharedLock* mLock;
   int* mTotal;
   bool mWrite;
   int mNumber;
   
   SharedLockRunnable(SharedLock* lock, int* total, bool write, int number)
   {
      mLock = lock;
      mTotal = total;
      mWrite = write;
      mNumber = number;
   }
   
   virtual ~SharedLockRunnable()
   {
   }
   
   virtual void run()
   {
      Thread::sleep(rand() % 10 + 1);
      
      if(mWrite)
      {
         mLock->lockExclusive();
         {
            for(int i = 0; i < 1000; i++)
            {
               *mTotal += mNumber;
            }
         }
         mLock->unlockExclusive();
      }
      else
      {
         mLock->lockShared();
         {
            assert(
               *mTotal == 0 || *mTotal == 2000 ||
               *mTotal == 3000 || *mTotal == 5000);
            //cout << "read total=" << *mTotal << std::endl;
            
            mLock->lockShared();
            {
               assert(
                  *mTotal == 0 || *mTotal == 2000 ||
                  *mTotal == 3000 || *mTotal == 5000);
               //cout << "read total=" << *mTotal << std::endl;
            }
            mLock->unlockShared();
         }
         mLock->unlockShared();
      }
   }
};

void runSharedLockTest(TestRunner& tr)
{
   tr.test("SharedLock");
   
   for(int i = 0; i < 200; i++)
   {
      SharedLock lock;
      int total = 0;
      
      SharedLockRunnable r1(&lock, &total, false, 0);
      SharedLockRunnable r2(&lock, &total, true, 2);
      SharedLockRunnable r3(&lock, &total, false, 0);
      SharedLockRunnable r4(&lock, &total, true, 3);
      SharedLockRunnable r5(&lock, &total, false, 0);
      
      Thread t1(&r1);
      Thread t2(&r2);
      Thread t3(&r3);
      Thread t4(&r4);
      Thread t5(&r5);
      
      t1.start();
      t2.start();
      t3.start();
      t4.start();
      t5.start();
      
      lock.lockShared();
      assert(total == 0 || total == 2000 || total == 3000 || total == 5000);
      lock.unlockShared();
      
      lock.lockShared();
      assert(total == 0 || total == 2000 || total == 3000 || total == 5000);
      lock.unlockShared();
      
      lock.lockShared();
      assert(total == 0 || total == 2000 || total == 3000 || total == 5000);
      lock.unlockShared();
      
      lock.lockShared();
      assert(total == 0 || total == 2000 || total == 3000 || total == 5000);
      lock.unlockShared();
      
      t1.join();
      t2.join();
      t3.join();
      t4.join();
      t5.join();
      
      lock.lockShared();
      assert(total == 5000);
      lock.unlockShared();
   }
   
   tr.passIfNoException();
}

void runDynamicObjectTest(TestRunner& tr)
{
   tr.test("DynamicObject");
   
   DynamicObject dyno1;
   dyno1["id"] = 2;
   dyno1["username"] = "testuser1000";
   dyno1["somearray"][0] = "item1";
   dyno1["somearray"][1] = "item2";
   dyno1["somearray"][2] = "item3";
   
   DynamicObject dyno2;
   dyno2["street"] = "1700 Kraft Dr.";
   dyno2["zip"] = "24060";
   
   dyno1["address"] = dyno2;
   
   assert(dyno1["id"]->getInt32() == 2);
   assertStrCmp(dyno1["username"]->getString(), "testuser1000");
   
   assertStrCmp(dyno1["somearray"][0]->getString(), "item1");
   assertStrCmp(dyno1["somearray"][1]->getString(), "item2");
   assertStrCmp(dyno1["somearray"][2]->getString(), "item3");
   
   DynamicObject dyno3 = dyno1["address"];
   assertStrCmp(dyno3["street"]->getString(), "1700 Kraft Dr.");
   assertStrCmp(dyno3["zip"]->getString(), "24060");
   
   DynamicObject dyno4;
   dyno4["whatever"] = "test";
   dyno4["someboolean"] = true;
   assert(dyno4["someboolean"]->getBoolean());
   dyno1["somearray"][3] = dyno4;
   
   dyno1["something"]["strange"] = "tinypayload";
   assertStrCmp(dyno1["something"]["strange"]->getString(), "tinypayload");
   
   DynamicObject dyno5;
   dyno5[0] = "mustard";
   dyno5[1] = "ketchup";
   dyno5[2] = "pickles";
   
   int count = 0;
   DynamicObjectIterator i = dyno5.getIterator();
   while(i->hasNext())
   {
      DynamicObject next = i->next();
      
      if(count == 0)
      {
         assertStrCmp(next->getString(), "mustard");
      }
      else if(count == 1)
      {
         assertStrCmp(next->getString(), "ketchup");
      }
      else if(count == 2)
      {
         assertStrCmp(next->getString(), "pickles");
      }
      
      count++;
   }
   
   DynamicObject dyno6;
   dyno6["eggs"] = "bacon";
   dyno6["milk"] = "yum";
   assertStrCmp(dyno6->removeMember("milk")->getString(), "yum");
   count = 0;
   i = dyno6.getIterator();
   while(i->hasNext())
   {
      DynamicObject next = i->next();
      assertStrCmp(i->getName(), "eggs");
      assertStrCmp(next->getString(), "bacon");
      count++;
   }
   
   assert(count == 1);
   
   // test clone
   dyno1["dyno5"] = dyno5;
   dyno1["dyno6"] = dyno6;
   dyno1["clone"] = dyno1.clone();
   
   DynamicObject clone = dyno1.clone();
   assert(dyno1 == clone);
   
   // test subset
   clone["mrmessy"] = "weirdguy";
   assert(dyno1.isSubset(clone));
   
   // test print out code
   //cout << endl;
   //dumpDynamicObject(dyno1);
   
   {
      // test int iterator
      DynamicObject d;
      d = 123;
      int count = 0;
      DynamicObjectIterator i = d.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         assert(next->getUInt32() == 123);
         count++;
      }
      assert(count == 1);
   }
   
   {
      // test string iterator
      DynamicObject d;
      d = "123";
      int count = 0;
      DynamicObjectIterator i = d.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         assertStrCmp(next->getString(), "123");
         count++;
      }
      assert(count == 1);
   }

   {
      // test auto-created string iterator
      DynamicObject d;
      int count = 0;
      DynamicObjectIterator i = d["moo!"].getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         assertStrCmp(next->getString(), "");
         count++;
      }
      assert(count == 1);
   }

   tr.pass();
}

void runDynoClearTest(TestRunner& tr)
{
   tr.test("DynamicObject clear");
   
   DynamicObject d;
   
   d = "x";
   assert(d->getType() == String);
   d->clear();
   assert(d->getType() == String);
   assertStrCmp(d->getString(), "");
   
   d = (int)1;
   assert(d->getType() == Int32);
   d->clear();
   assert(d->getType() == Int32);
   assert(d->getInt32() == 0);
   
   d = (unsigned int)1;
   assert(d->getType() == UInt32);
   d->clear();
   assert(d->getType() == UInt32);
   assert(d->getBoolean() == false);
   
   d = (long long)1;
   assert(d->getType() == Int64);
   d->clear();
   assert(d->getType() == Int64);
   assert(d->getInt64() == 0);
   
   d = (unsigned long long)1;
   d->clear();
   assert(d->getType() == UInt64);
   assert(d->getUInt64() == 0);
   
   d = (double)1.0;
   d->clear();
   assert(d->getType() == Double);
   assert(d->getDouble() == 0.0);
   
   d["x"] = 0;
   d->clear();
   assert(d->getType() == Map);
   assert(d->length() == 0);
   
   d[0] = 0;
   d->clear();
   assert(d->getType() == Array);
   assert(d->length() == 0);
   
   tr.passIfNoException();
}

void runDynoConversionTest(TestRunner& tr)
{
   tr.test("DynamicObject conversion");
   
   DynamicObject d;
   d["int"] = 2;
   d["-int"] = -2;
   d["str"] = "hello";
   d["true"] = "true";
   d["false"] = "false";
   
   const char* s;
   s = d["int"]->getString();
   assertStrCmp(s, "2");

   s = d["-int"]->getString();
   assertStrCmp(s, "-2");

   s = d["str"]->getString();
   assertStrCmp(s, "hello");

   s = d["true"]->getString();
   assertStrCmp(s, "true");

   s = d["false"]->getString();
   assertStrCmp(s, "false");
   
   tr.pass();
}

void runDynoRemoveTest(TestRunner& tr)
{
   tr.group("DynamicObject remove");

   tr.test("array");
   {
      DynamicObject d1;
      d1[0] = 0;
      d1[1] = 1;
      d1[2] = 2;
   
      DynamicObject d2;
      d2[0] = 0;
      d2[1] = 2;
   
      int count = 0;
      DynamicObjectIterator i = d1.getIterator();
      while(i->hasNext())
      {
         DynamicObject next = i->next();
   
         if(count == 1)
         {
            assert(next->getUInt32() == 1);
            i->remove();
         }
         
         count++;
      }
      
      assert(d1 == d2);
   }
   tr.passIfNoException();
   
   tr.test("map");
   {
      DynamicObject d1;
      d1["0"] = 0;
      d1["1"] = 1;
      d1["2"] = 2;
   
      DynamicObject d2;
      d2["0"] = 0;
      d2["2"] = 2;
   
      int count = 0;
      DynamicObjectIterator i = d1.getIterator();
      while(i->hasNext())
      {
         DynamicObject next = i->next();
   
         if(count == 1)
         {
            assert(next->getUInt32() == 1);
            i->remove();
         }
         
         count++;
      }
      
      assert(d1 == d2);
   }
   tr.passIfNoException();

   tr.ungroup();
}

void runDynoIndexTest(TestRunner& tr)
{
   tr.group("DynamicObject index");

   tr.test("array (iter)");
   {
      DynamicObject d;
      d[0] = 0;
      d[1] = 1;
      d[2] = 2;
   
      int count = 0;
      DynamicObjectIterator i = d.getIterator();
      while(i->hasNext())
      {
         DynamicObject next = i->next();
         assert(count == i->getIndex());
         count++;
      }
   }
   tr.passIfNoException();
   
   tr.test("array (rem)");
   {
      DynamicObject d;
      d[0] = 0;
      d[1] = 1;
      d[2] = 2;
   
      int count = 0;
      DynamicObjectIterator i = d.getIterator();
      while(i->hasNext())
      {
         DynamicObject next = i->next();
         assert(count == i->getIndex());
   
         if(count == 1)
         {
            assert(next->getUInt32() == 1);
            i->remove();
            assert(i->getIndex() == (count - 1));
         }
         
         count++;
      }
   }
   tr.passIfNoException();
   
   tr.test("map (iter)");
   {
      DynamicObject d;
      d["0"] = 0;
      d["1"] = 1;
      d["2"] = 2;
   
      int count = 0;
      DynamicObjectIterator i = d.getIterator();
      while(i->hasNext())
      {
         DynamicObject next = i->next();
         assert(count == i->getIndex());
         count++;
      }
   }
   tr.passIfNoException();

   tr.test("map (rem)");
   {
      DynamicObject d;
      d["0"] = 0;
      d["1"] = 1;
      d["2"] = 2;
   
      int count = 0;
      DynamicObjectIterator i = d.getIterator();
      while(i->hasNext())
      {
         DynamicObject next = i->next();
   
         if(count == 1)
         {
            assert(next->getUInt32() == 1);
            i->remove();
            assert(i->getIndex() == (count - 1));
         }
         
         count++;
      }
   }
   tr.passIfNoException();

   tr.ungroup();
}

class DbRtTester : public db::test::Tester
{
public:
   DbRtTester()
   {
      setName("rt");
   }

   /**
    * Run automatic unit tests.
    */
   virtual int runAutomaticTests(TestRunner& tr)
   {
      runThreadTest(tr);
      runThreadPoolTest(tr);
      runJobDispatcherTest(tr);
      runSharedLockTest(tr);
      runDynamicObjectTest(tr);
      runDynoClearTest(tr);
      runDynoConversionTest(tr);
      runDynoRemoveTest(tr);
      runDynoIndexTest(tr);
      return 0;
   }

   /**
    * Runs interactive unit tests.
    */
   virtual int runInteractiveTests(TestRunner& tr)
   {
//      runTimeTest(tr);
      return 0;
   }
};

#ifndef DB_TEST_NO_MAIN
DB_TEST_MAIN(DbRtTester)
#endif
