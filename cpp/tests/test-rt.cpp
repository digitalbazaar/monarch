/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/data/json/JsonWriter.h"
#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/rt/ExclusiveLock.h"
#include "monarch/rt/Runnable.h"
#include "monarch/rt/RunnableDelegate.h"
#include "monarch/rt/Thread.h"
#include "monarch/rt/Semaphore.h"
#include "monarch/rt/SharedLock.h"
#include "monarch/rt/System.h"
#include "monarch/rt/JobDispatcher.h"
#include "monarch/util/Macros.h"

#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <map>

using namespace std;
using namespace monarch::test;
using namespace monarch::rt;

namespace mo_test_rt
{

static void runTimeTest(TestRunner& tr)
{
   tr.test("Time");

   uint64_t start = System::getCurrentMilliseconds();

   printf("Time start=%" PRIu64 "\n", start);

   uint64_t end = System::getCurrentMilliseconds();

   printf("Time end=%" PRIu64 "\n", end);

   tr.pass();
}

static void runCpuInfoTest(TestRunner& tr)
{
   tr.test("CPU info");

   uint32_t count = System::getCpuCoreCount();

   printf("# cores=%" PRIu32 "\n", count);

   tr.pass();
}

class TestRunnable : public virtual ExclusiveLock, public Runnable
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
      //printf("%s: This is a TestRunnable thread,addr=%p\n", name, t);

      if(name == "Thread 1")
      {
         //printf("Thread 1 Waiting for interruption...\n");

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
//            printf("Thread 1 Interrupted. Exception message=%s\n",
//               e->getMessage());
//         }
//         else
//         {
//            printf("Thread 1 Finished.\n");
//         }
      }
      else if(name == "Thread 2")
      {
         //printf("Thread 2 Finished.\n");
      }
      else if(name == "Thread 3")
      {
         //printf("Thread 3 Waiting for Thread 5...\n");

         lock();
         lock();
         lock();
         {
            //printf("Thread 3 starting wait...\n");
            while(mustWait)
            {
               // thread 3 should be notified, not interrupted
               bool interrupted = !wait(5000);
               assert(!interrupted);
            }
            //printf("Thread 3 Awake!\n");
         }
         unlock();
         unlock();
         unlock();

//         if(Thread::interrupted())
//         {
//            printf("Thread 3 Interrupted.\n");
//         }
//         else
//         {
//            printf("Thread 3 Finished.\n");
//         }
      }
      else if(name == "Thread 4")
      {
         //printf("Thread 4 Finished.\n");
      }
      else if(name == "Thread 5")
      {
         //printf("Thread 5 waking up a thread...\n");

         lock();
         lock();
         lock();
         lock();
         {
            // wait for a moment
            Thread::sleep(100);
            mustWait = false;
            //printf("Thread 5 notifying a thread...\n");
            notifyAll();
            //printf("Thread 5 notified another thread.\n");
         }
         unlock();
         unlock();
         unlock();
         unlock();

         //printf("Thread 5 Finished.\n");
      }
   }
};

static void runThreadTest(TestRunner& tr)
{
   tr.test("Thread");

   //printf("Running Thread Test\n\n");

   TestRunnable r1;
   Thread t1(&r1, "Thread 1");
   Thread t2(&r1, "Thread 2");
   Thread t3(&r1, "Thread 3");
   Thread t4(&r1, "Thread 4");
   Thread t5(&r1, "Thread 5");

   //printf("Threads starting...\n");

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

   //printf("\nThread Test complete.\n");
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
      //printf("\nTestJob: Running a job,name=%s\n", mName);

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

      //printf("\nTestJob: Finished a job,name=%s\n", mName);
   }
};

static void runThreadPoolTest(TestRunner& tr)
{
   tr.test("ThreadPool");

   Exception::clear();

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

static void runJobDispatcherTest(TestRunner& tr)
{
   tr.test("JobDispatcher");

   Exception::clear();

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

class ExclusiveLockRunnable : public Runnable
{
public:
   ExclusiveLock* mLock;
   volatile bool* mCondition;
   ExclusiveLockRunnable(ExclusiveLock* lock, volatile bool* condition) :
      mLock(lock), mCondition(condition) {}
   virtual ~ExclusiveLockRunnable() {}

   virtual void run()
   {
      // already locked in test so should fail
      assert(!mLock->tryLock());

      // set condition
      *mCondition = true;

      // now lock
      mLock->lock();

      // update condition in test so it will wait for thread to join
      *mCondition = false;

      // grabbing lock again should work
      assert(mLock->tryLock());

      // release lock
      mLock->unlock();

      // grabbing lock again should work
      assert(mLock->tryLock());
      mLock->unlock();
   }
};

static void runExclusiveLockTest(TestRunner& tr)
{
   tr.group("ExclusiveLock");

   tr.test("try lock");
   {
      ExclusiveLock lock;
      volatile bool condition = false;

      ExclusiveLockRunnable r1(&lock, &condition);
      Thread t1(&r1);

      // grap lock
      lock.lock();

      // start thread, spin until it sets condition
      t1.start();
      while(!condition);
      lock.unlock();

      // join thread
      t1.join();

      assert(!condition);
   }
   tr.passIfNoException();

   tr.ungroup();
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
            for(int i = 0; i < 1000; ++i)
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
            int total = *mTotal;
            assert(
               total == 0 || total == 2000 ||
               total == 3000 || total == 5000);

            mLock->lockShared();
            {
               assert(
                  total == 0 || total == 2000 ||
                  total == 3000 || total == 5000);

               mLock->lockShared();
               {
                  assert(
                     total == 0 || total == 2000 ||
                     total == 3000 || total == 5000);
               }
               mLock->unlockShared();
            }
            mLock->unlockShared();
         }
         mLock->unlockShared();
      }
   }
};

class DeadlockRunnable : public Runnable
{
public:
   SharedLock* mLock;
   ExclusiveLock* mSignalLock;
   bool* mSignal;
   bool mWrite;

   DeadlockRunnable(
      SharedLock* lock, ExclusiveLock* signalLock, bool* signal, bool write) :
      mLock(lock),
      mSignalLock(signalLock),
      mSignal(signal),
      mWrite(write)
   {
   }

   virtual ~DeadlockRunnable()
   {
   }

   virtual void run()
   {
      if(mWrite)
      {
         // wait for signal to get exclusive lock
         mSignalLock->lock();
         while(!(*mSignal))
         {
            mSignalLock->wait();
         }
         mSignalLock->unlock();

         // get exclusive lock
         mLock->lockExclusive();

         // should block forever if test fails

         mLock->unlockExclusive();
      }
      else
      {
         // get shared lock
         mLock->lockShared();
         {
            // set signal
            mSignalLock->lock();
            *mSignal = true;
            mSignalLock->notifyAll();
            mSignalLock->unlock();

            // wait to allow lock exclusive to occur in write thread
            Thread::sleep(250);

            // try to get shared lock
            mLock->lockShared();

            // should block here for ever if test fails

            // recursive unlock shared lock
            mLock->unlockShared();
         }
         mLock->unlockShared();
      }
   }
};

static void _runSharedLockDeadlockTest()
{
   // this test checks to see if thread 1 can get a read lock,
   // wait for thread 2 to get a write lock, and then see if
   // thread 1 can recurse its read lock (it should be able to)

   SharedLock lock;
   ExclusiveLock signalLock;
   bool signal = false;

   DeadlockRunnable r1(&lock, &signalLock, &signal, false);
   DeadlockRunnable r2(&lock, &signalLock, &signal, true);

   Thread t1(&r1);
   Thread t2(&r2);

   t2.start();
   t1.start();

   t1.join();
   t2.join();
}

static void runSharedLockTest(TestRunner& tr)
{
   tr.group("SharedLock");

   tr.test("simple read/write");
   {
      uint64_t start = System::getCurrentMilliseconds();
      for(int i = 0; i < 200; ++i)
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

         lock.lockExclusive();
         lock.lockShared();
         assert(total == 0 || total == 2000 || total == 3000 || total == 5000);
         lock.unlockShared();
         lock.unlockExclusive();

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
      uint64_t end = System::getCurrentMilliseconds();
      double secs = (end - start) / 1000.;
      printf("time=%.2f secs... ", secs);
   }
   tr.passIfNoException();

   tr.test("recursive read+write+read");
   {
      _runSharedLockDeadlockTest();
   }
   tr.passIfNoException();

   tr.ungroup();
}

struct StarvationData
{
   SharedLock lock;
   ExclusiveLock protect;
   bool signal;
   bool write;
   bool reader;
   int count;
   int maxReaders;
   int maxWriters;
};

class StarvationRunnable : public Runnable
{
public:
   StarvationData* mStarvationData;
   bool mWriter;

   StarvationRunnable(StarvationData* sd, bool writer) :
      mStarvationData(sd),
      mWriter(writer)
   {
   }

   virtual ~StarvationRunnable()
   {
   }

   virtual void run()
   {
      // wait for signal to start
      mStarvationData->protect.lock();
      while(!mStarvationData->signal)
      {
         mStarvationData->protect.wait();
      }
      mStarvationData->protect.unlock();

      Thread::sleep(rand() % 10 + 1);

      if(mWriter)
      {
         mStarvationData->lock.lockExclusive();
         {
            if(mStarvationData->reader)
            {
               mStarvationData->maxReaders = mStarvationData->count;
               mStarvationData->count = 1;
               mStarvationData->reader = false;
            }
            else
            {
               ++mStarvationData->count;
            }
         }
         mStarvationData->lock.unlockExclusive();
      }
      else
      {
         mStarvationData->lock.lockShared();
         {
            if(!mStarvationData->reader)
            {
               mStarvationData->maxWriters = mStarvationData->count;
               mStarvationData->count = 1;
               mStarvationData->reader = true;
            }
            else
            {
               ++mStarvationData->count;
            }
         }
         mStarvationData->lock.unlockShared();
      }
   }
};

static void runInteractiveSharedLockTest(TestRunner& tr)
{
   tr.group("SharedLock");

   tr.test("recursive read+write+read");
   {
      _runSharedLockDeadlockTest();
   }
   tr.passIfNoException();

   tr.test("starvation");
   {
      // this test checks to ensure that neither readers
      // nor writers starve each other out
      for(int i = 0; i < 200; ++i)
      {
         StarvationData sd;
         sd.signal = false;
         sd.reader = false;
         sd.count = 0;
         sd.maxReaders = 0;
         sd.maxWriters = 0;

         int num = 50;
         Thread* threads[(num * 2)];

         // create readers
         for(int n = 0; n < num; ++n)
         {
            RunnableRef r = new StarvationRunnable(&sd, false);
            threads[n] = new Thread(r);
         }

         // create writers
         for(int n = 0; n < num; ++n)
         {
            RunnableRef r = new StarvationRunnable(&sd, true);
            threads[n + num] = new Thread(r);
         }

         // start threads
         for(int n = 0; n < (num * 2); ++n)
         {
            threads[n]->start();
         }

         // set signal to start
         sd.protect.lock();
         sd.signal = true;
         sd.protect.notifyAll();
         sd.protect.unlock();

         // join threads
         for(int n = 0; n < (num * 2); ++n)
         {
            threads[n]->join();
            delete threads[n];
         }

         // report max counts
         printf(
            "max consecutive readers: %i, "
            "max consecutive writers: %i... \n",
            sd.maxReaders, sd.maxWriters);
      }
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runCollectableTest(TestRunner& tr)
{
   tr.group("Collectable");

   tr.test("basic");
   {
      Collectable<int> c = new int(10);
   }
   tr.passIfNoException();

   tr.test("relinquish");
   {
      Collectable<int> c = new int(10);
      int* foo = c.relinquish();
      delete foo;
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runDynamicObjectTest(TestRunner& tr)
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
   assert(dyno1["username"] == "testuser1000");
   assert("testuser1000" == dyno1["username"]);

   assertStrCmp(dyno1["somearray"][0]->getString(), "item1");
   assertStrCmp(dyno1["somearray"][1]->getString(), "item2");
   assertStrCmp(dyno1["somearray"][2]->getString(), "item3");
   assert(dyno1["somearray"][0] == "item1");
   assert(dyno1["somearray"][1] == "item2");
   assert(dyno1["somearray"][2] == "item3");
   assert("item1" == dyno1["somearray"][0]);
   assert("item2" == dyno1["somearray"][1]);
   assert("item3" == dyno1["somearray"][2]);

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

      ++count;
   }

   DynamicObject dyno6;
   dyno6["eggs"] = "bacon";
   dyno6["milk"] = "yum";
   assertStrCmp(dyno6["milk"]->getString(), "yum");
   dyno6->removeMember("milk");
   assert(!dyno6->hasMember("milk"));
   assert(dyno6->length() == 1);
   count = 0;
   i = dyno6.getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();
      assertStrCmp(i->getName(), "eggs");
      assertStrCmp(next->getString(), "bacon");
      ++count;
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
   //printf("\n");
   //dumpDynamicObject(dyno1);

   {
      DynamicObject d;
      d = "abc";
      assert(d == "abc");
      assert(d != "abd");
      assert(d != "aba");
      assert(d < "abd");
      assert(d <= "abc");
      assert(d <= "abd");
      assert(d > "aba");
      assert(d >= "abc");
      assert(d >= "aba");
      assert("abc" == d);
      assert("abd" != d);
      assert("aba" != d);
      assert("abd" > d);
      assert("abc" >= d);
      assert("abd" >= d);
      assert("aba" < d);
      assert("abc" <= d);
      assert("aba" <= d);
   }

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
         ++count;
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
         ++count;
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
         ++count;
      }
      assert(count == 1);
   }

   {
      // test name for null value
      DynamicObject d;
      d["a"].setNull();
      int count = 0;
      DynamicObjectIterator i = d.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         const char* name = i->getName();
         assertStrCmp(name, "a");
         assert(next.isNull());
         ++count;
      }
      assert(count == 1);
   }

   {
      // test formatted string
      DynamicObject d;
      d->format("String %s, Integer %i", "mystring", 1);
      assertStrCmp(d->getString(), "String mystring, Integer 1");
   }

   {
      // test formatted string using current string
      DynamicObject d;
      d = "b";
      d->format("a%sc", d->getString());
      assertStrCmp(d->getString(), "abc");
   }

   {
      // test length types
      {
         DynamicObject d;
         assert(d->length() == 0);
      }
      {
         DynamicObject d;
         d->setType(String);
         assert(d->length() == 0);
         d = "123";
         assert(d->length() == 3);
      }
      {
         DynamicObject d;
         d->setType(Map);
         assert(d->length() == 0);
         d["1"] = 1;
         d["2"] = 2;
         d["3"] = 3;
         assert(d->length() == 3);
      }
      {
         DynamicObject d;
         d->setType(Array);
         assert(d->length() == 0);
         d[0] = 1;
         d[1] = 2;
         d[2] = 3;
         assert(d->length() == 3);
      }
      {
         DynamicObject d;
         d->setType(Array);
         assert(d->length() == 0);
         d->append(1);
         d->append(2);
         d->append(3);
         assert(d->length() == 3);
      }
      {
         DynamicObject d;
         d->setType(Boolean);
         assert(d->length() == 1);
      }
      {
         DynamicObject d;
         d->setType(Int32);
         assert(d->length() == sizeof(int32_t));
      }
      {
         DynamicObject d;
         d->setType(UInt32);
         assert(d->length() == sizeof(uint32_t));
      }
      {
         DynamicObject d;
         d->setType(Int64);
         assert(d->length() == sizeof(int64_t));
      }
      {
         DynamicObject d;
         d->setType(UInt64);
         assert(d->length() == sizeof(uint64_t));
      }
      {
         DynamicObject d;
         d->setType(Double);
         assert(d->length() == sizeof(double));
      }
   }

   {
      // test '==' and '<' compare of null strings
      DynamicObject d1;
      d1->setType(String);
      DynamicObject d2;
      d2->setType(String);
      assert(d1 == d2);
      d2 = "x";
      assert(d1 < d2);
   }

   {
      DynamicObject dArray;
      DynamicObject d1;
      d1 = "first";
      DynamicObject d2;
      d2 = "second";
      DynamicObject d3;
      d3 = "third";
      dArray[0] = d1;
      dArray[1] = d2;
      dArray[2] = d3;

      DynamicObject d4;
      d4 = "second";
      //printf("index=%i\n", dArray->getIndex(d4));
      assert(dArray->indexOf(d4) == 1);

      DynamicObject d5;
      d5 = "fourth";
      assert(dArray->indexOf(d5) == -1);
   }

   {
      DynamicObject d;
      d->append("bar");
      d->append("baz");
      d->append(1);
      d->append(true);

      int i;
      i = d->indexOf("bar");
      assert(i == 0);
      i = d->indexOf("baz");
      assert(i == 1);
      i = d->indexOf(1);
      assert(i == 2);
      i = d->indexOf(true);
      assert(i == 3);
      i = d->indexOf(false);
      assert(i == -1);
      i = d->indexOf("bank");
      assert(i == -1);
   }

   {
      // test compare cloned object
      DynamicObject d1;
      d1["map"]["key"] = "value";
      DynamicObject d2 = d1.clone();
      assert(d1 == d2);
      assert(!(d1 < d2));
   }

   {
      // test same object as map key
      DynamicObject d1;
      d1["map"]["key"] = "value";
      DynamicObject d2;
      d2["map"]["key"] = "value";
      assert(d1 == d2);
      assert(!(d1 < d2));

      std::map<DynamicObject, int> maptest;
      maptest.insert(make_pair(d1, 1));
      maptest.insert(make_pair(d2, 1));
      assert(maptest.size() == 1);
   }

   {
      // test '<' for arrays
      DynamicObject d1;
      d1->setType(Array);
      DynamicObject d2;
      d2->setType(Array);

      d1[0] = 0;
      d1[1] = 1;

      d2[0] = 0;
      d2[1] = 2;

      assert(!(d1 == d2));
      assert(d1 < d2);
   }

   {
      // test for string equality
      DynamicObject d1;
      d1 = "foo";
      assert(d1 == "foo");
      assert(d1 != "bar");

      DynamicObject d2;
      d2 = 1;
      assert(!(d1 == "1"));
      assert(d1 != "1");
   }

   {
      // test positive index array expansion
      DynamicObject d;
      d->setType(Array);
      assert(d->length() == 0);
      d[1] = 1;
      assert(d->length() == 2);
      d[1];
      assert(d->length() == 2);
   }

   {
      // test negative index array expansion
      DynamicObject d;
      d->setType(Array);
      assert(d->length() == 0);
      d[-1] = 1;
      assert(d->length() == 1);
   }

   {
      // test pop
      DynamicObject d;
      d->setType(Array);
      assert(d->length() == 0);
      d->append();
      assert(d->length() == 1);
      d->pop();
      assert(d->length() == 0);
      // pop on empty array a no-op
      d->pop();
      assert(d->length() == 0);
   }

   tr.pass();
}

static void runDynoClearTest(TestRunner& tr)
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

   d = (int64_t)1;
   assert(d->getType() == Int64);
   d->clear();
   assert(d->getType() == Int64);
   assert(d->getInt64() == 0);

   d = (uint64_t)1;
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

static void runDynoConversionTest(TestRunner& tr)
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

static void runDynoCastTest(TestRunner& tr)
{
   tr.test("DynamicObject casting");

   {
      DynamicObject d;
      bool v;

      d = true;
      v = d;
      assert(v == d->getBoolean());

      d = false;
      v = d;
      assert(v == d->getBoolean());
   }

   {
      DynamicObject d;
      const char* t = "test";
      const char* v;

      d = t;
      v = d;
      // check pointers are different
      assert(v != t);
      assertStrCmp(v, d->getString());
      assertStrCmp(v, t);
   }

   {
      DynamicObject d;
      char* t = (char*)malloc(10);
      strcpy(t, "value");

      d["x"] = t;
      assertStrCmp(d["x"]->getString(), t);
      free(t);
   }

   {
      DynamicObject d;
      // const vals
      const char* cn = "cn";
      const char* cv = "cv";
      // array vals
      char an[10];
      strcpy(an, "an");
      char av[10];
      strcpy(av, "av");

      d[cn] = cv;
      assertStrCmp(d[cn]->getString(), cv);
      d[cn] = av;
      assertStrCmp(d[cn]->getString(), av);
      d[an] = cv;
      assertStrCmp(d[an]->getString(), cv);
      d[an] = av;
      assertStrCmp(d[an]->getString(), av);
   }

   {
      DynamicObject d;
      uint32_t t = 123;
      uint32_t v;

      d = t;
      v = d;
      assert(d->getUInt32() == t);
      assert(v == t);
   }

   {
      // lval and rval

      DynamicObject d;
      uint32_t t = 123;

      d = t;
      assert(d->getUInt32() == t);
      assert(t == d->getUInt32());
      // do need to cast here to avoid ambiguous comparison errors
      assert((uint32_t)d == t);
      assert(t == (uint32_t)d);
   }

   tr.pass();
}

static void runDynoRemoveTest(TestRunner& tr)
{
   tr.group("DynamicObject remove");

   tr.test("array of 1");
   {
      DynamicObject d1;
      d1[0] = 0;

      DynamicObject d2;
      d2->setType(Array);

      DynamicObjectIterator i = d1.getIterator();
      assert(i->hasNext());
      i->next();
      i->remove();
      assert(!i->hasNext());
      assertDynoCmp(d1, d2);
   }
   tr.passIfNoException();

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
         DynamicObject& next = i->next();

         if(count == 1)
         {
            assert(next->getUInt32() == 1);
            i->remove();
         }

         ++count;
      }

      assertDynoCmp(d1, d2);
   }
   tr.passIfNoException();

   tr.test("map of 1");
   {
      DynamicObject d1;
      d1["0"] = 0;

      DynamicObject d2;
      d2->setType(Map);

      DynamicObjectIterator i = d1.getIterator();
      assert(i->hasNext());
      i->next();
      i->remove();
      assert(!i->hasNext());
      assertDynoCmp(d1, d2);
   }
   tr.passIfNoException();

   tr.test("map of 2");
   {
      DynamicObject d1;
      d1["0"] = 0;
      d1["1"] = 0;

      DynamicObject d2;
      d2["0"] = 0;

      DynamicObjectIterator i = d1.getIterator();
      assert(i->hasNext());
      i->next();
      assert(i->hasNext());
      i->next();
      assert(!i->hasNext());
      i->remove();
      assert(!i->hasNext());
      assertDynoCmp(d1, d2);
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
         DynamicObject& next = i->next();

         if(count == 1)
         {
            assert(next->getUInt32() == 1);
            i->remove();
         }

         ++count;
      }

      assertDynoCmp(d1, d2);
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runDynoIndexTest(TestRunner& tr)
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
         i->next();
         assert(count == i->getIndex());
         ++count;
      }
   }
   tr.passIfNoException();

   tr.test("array (rem)");
   {
      DynamicObject d;
      d[0] = 0;
      d[1] = 1;
      d[2] = 2;

      int count = -1;
      bool done = false;
      DynamicObjectIterator i = d.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         ++count;
         assert(count == i->getIndex());

         if(!done && count == 1)
         {
            uint32_t val = next->getUInt32();
            assert(val == 1);
            i->remove();
            --count;
            assert(i->getIndex() == count);
            done = true;
         }
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
         i->next();
         assert(count == i->getIndex());
         ++count;
      }
   }
   tr.passIfNoException();

   tr.test("map (rem)");
   {
      DynamicObject d;
      d["0"] = 0;
      d["1"] = 1;
      d["2"] = 2;

      int count = -1;
      DynamicObjectIterator i = d.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();
         ++count;

         if(count == 1)
         {
            assert(next->getUInt32() == 1);
            i->remove();
            assert(i->getIndex() == (count - 1));
         }
      }
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runDynoTypeTest(TestRunner& tr)
{
   tr.group("DynamicObject types");

   tr.test("determineType");
   {
      DynamicObject d;

      d = 0;
      assert(DynamicObject::determineType(d->getString()) == UInt64);

      d = "0";
      assert(DynamicObject::determineType(d->getString()) == UInt64);

      d = 1;
      assert(DynamicObject::determineType(d->getString()) == UInt64);

      d = "1";
      assert(DynamicObject::determineType(d->getString()) == UInt64);

      d = -1;
      assert(DynamicObject::determineType(d->getString()) == Int64);

      d = "-1";
      assert(DynamicObject::determineType(d->getString()) == Int64);

      d = " -1";
      assert(DynamicObject::determineType(d->getString()) == String);

      d = " ";
      assert(DynamicObject::determineType(d->getString()) == String);

      d = "x";
      assert(DynamicObject::determineType(d->getString()) == String);

      // FIXME: check for Double
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runDynoAppendTest(TestRunner& tr)
{
   tr.group("DynamicObject append");

   tr.test("append basic");
   {
      DynamicObject d;

      DynamicObject& next = d->append();
      next = "test";

      assert(d->length() == 1);
      assertStrCmp(d[0]->getString(), "test");
   }
   tr.passIfNoException();

   tr.test("append ref");
   {
      DynamicObject d;

      DynamicObject& next = d->append();
      next = "test";

      assert(d->length() == 1);
      assertStrCmp(d[0]->getString(), "test");
   }
   tr.passIfNoException();

   tr.test("append inline");
   {
      DynamicObject d;

      d->append("test");

      assert(d->length() == 1);
      assertStrCmp(d[0]->getString(), "test");
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runDynoMergeTest(TestRunner& tr)
{
   tr.group("DynamicObject merge");

   tr.test("merge basic");
   {
      DynamicObject d;
      d->setType(Map);

      DynamicObject d2;
      d2["a"] = true;

      d.merge(d2, true);

      DynamicObject expect;
      expect["a"] = true;
      assert(d == expect);
   }
   tr.passIfNoException();

   tr.test("merge no append");
   {
      DynamicObject d;
      d[0] = "d-0";

      DynamicObject d2;
      d2[0] = "d2-0";
      d2[1] = "d2-1";

      d.merge(d2, false);

      DynamicObject expect;
      expect[0] = "d2-0";
      expect[1] = "d2-1";
      assert(d == expect);
   }
   tr.passIfNoException();

   tr.test("merge append");
   {
      DynamicObject d;
      d[0] = "d-0";

      DynamicObject d2;
      d2[0] = "d2-0";
      d2[1] = "d2-1";

      d.merge(d2, true);

      DynamicObject expect;
      expect[0] = "d-0";
      expect[1] = "d2-0";
      expect[2] = "d2-1";
      assert(d == expect);
   }
   tr.passIfNoException();

   tr.test("merge shallow");
   {
      DynamicObject d;
      d["0"] = "d-0";

      DynamicObject d2;
      d2["1"] = "d2-1";
      d2["2"] = "d2-2";

      d.merge(d2, true);

      DynamicObject expect;
      expect["0"] = "d-0";
      expect["1"] = "d2-1";
      expect["2"] = "d2-2";
      assert(d == expect);
   }
   tr.passIfNoException();

   tr.test("merge deep");
   {
      DynamicObject d;
      d["0"]["0"] = "d-0-0";

      DynamicObject d2;
      d2["0"]["1"] = "d2-0-1";

      d.merge(d2, true);

      DynamicObject expect;
      expect["0"]["0"] = "d-0-0";
      expect["0"]["1"] = "d2-0-1";
      assert(d == expect);
   }
   tr.passIfNoException();

   tr.test("merge deep overwrite");
   {
      DynamicObject d;
      d["0"]["0"] = "d-0-0";

      DynamicObject d2;
      d2["0"]["0"] = "d2-0-0";

      d.merge(d2, true);

      DynamicObject expect;
      expect["0"]["0"] = "d2-0-0";
      assert(d == expect);
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runDynoDiffTest(TestRunner& tr)
{
   tr.group("DynamicObject diff");

   tr.test("no diff");
   {
      DynamicObject d1;
      d1["a"] = true;

      DynamicObject d2;
      d2["a"] = true;

      DynamicObject diff;
      assert(!d1.diff(d2, diff));
   }
   tr.passIfNoException();

   tr.test("basic types");
   {
      DynamicObject d1;
      d1 = true;

      DynamicObject d2;
      d2 = true;

      DynamicObject diff;
      assert(!d1.diff(d2, diff));

      d2 = false;
      DynamicObject expect;
      expect["type"] = "valueChanged";
      expect["source"] = true;
      expect["target"] = false;
      assert(d1.diff(d2, diff));
   }
   tr.passIfNoException();

   tr.test("basic map");
   {
      DynamicObject d1;
      d1->setType(Map);

      DynamicObject d2;
      d2["a"] = true;

      DynamicObject expect;
      DynamicObject diff;

      // d1 diff d2
      expect->clear();
      expect[0]["key"] = "a";
      expect[0]["added"] = true;
      assert(d1.diff(d2, diff));
      assertDynoCmp(diff, expect);

      // d2 diff d1 (reverse above)
      expect->clear();
      expect[0]["key"] = "a";
      expect[0]["removed"] = true;
      assert(d2.diff(d1, diff));
      assertDynoCmp(diff, expect);

      // d1 diff d2 (changed value)
      d1["a"] = false;
      expect->clear();
      expect[0]["key"] = "a";
      expect[0]["changed"]["type"] = "valueChanged";
      expect[0]["changed"]["source"] = false;
      expect[0]["changed"]["target"] = true;
      assert(d1.diff(d2, diff));
      assertDynoCmp(diff, expect);
   }
   tr.passIfNoException();

   tr.test("basic array");
   {
      DynamicObject d1;
      d1->setType(Array);

      DynamicObject d2;
      d2[0] = true;

      DynamicObject expect;

      // d1 diff d2
      DynamicObject diff;
      expect[0]["index"] = 0;
      expect[0]["added"] = true;
      assert(d1.diff(d2, diff));
      assertDynoCmp(diff, expect);

      // d2 diff d1 (reverse above)
      expect->clear();
      expect[0]["index"] = 0;
      expect[0]["removed"] = true;
      assert(d2.diff(d1, diff));
      assertDynoCmp(diff, expect);
   }
   tr.passIfNoException();

   tr.test("deep 1");
   {
      DynamicObject d1;
      d1->setType(Map);

      DynamicObject d2;
      d2["a"]["a1"] = true;
      d2["a"]["a2"] = 123;
      d2["b"]["b1"] = "Hello, World!";

      DynamicObject expect;
      expect[0]["key"] = "a";
      expect[0]["added"] = d2["a"].clone();
      expect[1]["key"] = "b";
      expect[1]["added"] = d2["b"].clone();

      DynamicObject diff;
      assert(d1.diff(d2, diff));
      assertDynoCmp(diff, expect);
   }
   tr.passIfNoException();

   tr.test("deep 2");
   {
      DynamicObject d1;
      d1["a"]["same"] = true;
      d1["a"]["ch"] = 234;
      d1["a"]["rem"] = 1;
      d1["b"]["b1"] = "Hello, World!";

      DynamicObject d2;
      d2["a"]["same"] = true;
      d2["a"]["ch"] = 123;
      d2["a"]["add"] = 2;
      d2["c"]["c1"] = "Hello, World!";

      DynamicObject expect;
      expect[0]["key"] = "a";
      expect[0]["changed"][0]["key"] = "ch";
      expect[0]["changed"][0]["changed"]["type"] = "valueChanged";
      expect[0]["changed"][0]["changed"]["source"] = 234;
      expect[0]["changed"][0]["changed"]["target"] = 123;
      expect[0]["changed"][1]["key"] = "rem";
      expect[0]["changed"][1]["removed"] = 1;
      expect[0]["changed"][2]["key"] = "add";
      expect[0]["changed"][2]["added"] = 2;
      expect[1]["key"] = "b";
      expect[1]["removed"]["b1"] = "Hello, World!";
      expect[2]["key"] = "c";
      expect[2]["added"]["c1"] = "Hello, World!";

      DynamicObject diff;
      assert(d1.diff(d2, diff));
      assertDynoCmp(diff, expect);
   }
   tr.passIfNoException();

   tr.test("ints");
   {
      // common value
      #define V 123

      DynamicObject d1;
      d1["u32-u32"] = (uint32_t)V;
      d1["u32-u64"] = (uint32_t)V;
      d1["u32-s32"] = (uint32_t)V;
      d1["u32-s64"] = (uint32_t)V;

      d1["s32-u32"] = (int32_t)V;
      d1["s32-u64"] = (int32_t)V;
      d1["s32-s32"] = (int32_t)V;
      d1["s32-s64"] = (int32_t)V;

      d1["u64-u32"] = (uint64_t)V;
      d1["u64-u64"] = (uint64_t)V;
      d1["u64-s32"] = (uint64_t)V;
      d1["u64-s64"] = (uint64_t)V;

      d1["s64-u32"] = (uint64_t)V;
      d1["s64-u64"] = (int64_t)V;
      d1["s64-s32"] = (int64_t)V;
      d1["s64-s64"] = (int64_t)V;

      DynamicObject d2;
      d2["u32-u32"] = (uint32_t)V;
      d2["u32-u64"] = (uint64_t)V;
      d2["u32-s32"] = (int32_t)V;
      d2["u32-s64"] = (int64_t)V;

      d2["s32-u32"] = (uint32_t)V;
      d2["s32-u64"] = (uint64_t)V;
      d2["s32-s32"] = (int32_t)V;
      d2["s32-s64"] = (int64_t)V;

      d2["u64-u32"] = (uint32_t)V;
      d2["u64-u64"] = (uint64_t)V;
      d2["u64-s32"] = (int32_t)V;
      d2["u64-s64"] = (int64_t)V;

      d2["s64-u32"] = (uint32_t)V;
      d2["s64-u64"] = (uint64_t)V;
      d2["s64-s32"] = (int32_t)V;
      d2["s64-s64"] = (int64_t)V;

      #undef V

      DynamicObject diff;
      // types cause difference
      assert(d1.diff(d2, diff, DynamicObject::DiffEqual));
      assert(diff->length() == 12);
      // no diff with comparisons using int64s
      assert(!(d1.diff(d2, diff, DynamicObject::DiffIntegersAsInt64s)));
      assert(diff->length() == 0);
   }
   tr.passIfNoException();

   tr.test("doubles");
   {
      #define V 1.23456789

      DynamicObject d1;
      d1["d1"] = (double)V;

      DynamicObject d2;
      // set and convert through a string back to a double
      d2["d1"] = (double)V;
      d2["d1"]->setType(String);
      d2["d1"]->setType(Double);

      #undef V

      DynamicObject diff;
      // not exact due to double->string->double conversion
      assert(d1.diff(d2, diff, DynamicObject::DiffEqual));
      assert(diff->length() == 1);
      // no diff when compared as strings
      assert(!(d1.diff(d2, diff, DynamicObject::DiffDoublesAsStrings)));
      assert(diff->length() == 0);
   }
   tr.passIfNoException();

   tr.test("array no diff");
   {
      DynamicObject d1;
      d1["a"][0] = "data";

      DynamicObject d2;
      d2["a"][0] = "data";

      DynamicObject diff;
      DynamicObject expect;
      expect->setType(Map);
      assert(!d1.diff(d2, diff));
      assert(diff->length() == 0);
      assert(!d2.diff(d1, diff));
      assert(diff->length() == 0);
   }
   tr.passIfNoException();

   tr.test("array added/removed diffs");
   {
      DynamicObject d1;
      d1["a"]->setType(Array);

      DynamicObject d2;
      d2["a"]->setType(Array);
      DynamicObject d3;
      d3 = "data";
      d2["a"]->append(d3);

      DynamicObject diff;

      {
         DynamicObject expect;
         expect[0]["key"] = "a";
         expect[0]["changed"][0]["index"] = 0;
         expect[0]["changed"][0]["added"] = "data";
         assert(d1.diff(d2, diff));
         assertDynoCmp(diff, expect);
      }

      {
         DynamicObject expect;
         expect[0]["key"] = "a";
         expect[0]["changed"][0]["index"] = 0;
         expect[0]["changed"][0]["removed"] = "data";
         assert(d2.diff(d1, diff));
         assertDynoCmp(diff, expect);
      }
   }
   tr.passIfNoException();

   tr.test("array changed diffs");
   {
      DynamicObject d1;
      d1["a"][0] = "1";

      DynamicObject d2;
      d2["a"][0] = "2";

      DynamicObject expect;
      expect[0]["key"] = "a";
      expect[0]["changed"][0]["index"] = 0;
      expect[0]["changed"][0]["changed"]["type"] = "valueChanged";
      expect[0]["changed"][0]["changed"]["source"] = "1";
      expect[0]["changed"][0]["changed"]["target"] = "2";

      DynamicObject diff;
      assert(d1.diff(d2, diff));
      assertDynoCmp(diff, expect);
   }
   tr.passIfNoException();

   tr.test("map no diff");
   {
      DynamicObject d1;
      d1["a"]["b"] = "data";

      DynamicObject d2;
      d2["a"]["b"] = "data";

      DynamicObject diff;
      DynamicObject expect;
      expect->setType(Map);
      assert(!d1.diff(d2, diff));
      assert(diff->length() == 0);
      assert(!d2.diff(d1, diff));
      assert(diff->length() == 0);
   }
   tr.passIfNoException();

   tr.test("map added/removed diffs");
   {
      DynamicObject d1;
      d1["a"]->setType(Map);

      DynamicObject d2;
      d2["a"]["b"] = "data";

      DynamicObject diff;

      {
         DynamicObject expect;
         expect[0]["key"] = "a";
         expect[0]["changed"][0]["key"] = "b";
         expect[0]["changed"][0]["added"] = "data";
         assert(d1.diff(d2, diff));
         assertDynoCmp(diff, expect);
      }

      {
         DynamicObject expect;
         expect[0]["key"] = "a";
         expect[0]["changed"][0]["key"] = "b";
         expect[0]["changed"][0]["removed"] = "data";
         assert(d2.diff(d1, diff));
         assertDynoCmp(diff, expect);
      }
   }
   tr.passIfNoException();

   tr.test("map changed diffs");
   {
      DynamicObject d1;
      d1["a"][0] = "1";

      DynamicObject d2;
      d2["a"][0] = "2";

      DynamicObject expect;
      expect[0]["key"] = "a";
      expect[0]["changed"][0]["index"] = 0;
      expect[0]["changed"][0]["changed"]["type"] = "valueChanged";
      expect[0]["changed"][0]["changed"]["source"] = "1";
      expect[0]["changed"][0]["changed"]["target"] = "2";

      DynamicObject diff;
      assert(d1.diff(d2, diff));
      assertDynoCmp(diff, expect);
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runDynoCopyTest(TestRunner& tr)
{
   tr.group("DynamicObject copy");

   tr.test("impl");
   {
      DynamicObject d;
      d = "foo";
      DynamicObjectImpl* diaddr = &(*d);

      {
         DynamicObject d2;
         d2 = "bar";
         *d = *d2;
         assertStrCmp(d->getString(), d2->getString());
         // string address compare
         assert(d->getString() != d2->getString());
         // clear to something else
         d2->clear();
      }

      assertStrCmp(d->getString(), "bar");

      DynamicObject d3;
      d3 = (int32_t)1;
      *d = *d3;
      assert(d->getType() == d3->getType());
      assert(d->getType() == Int32);
      assert(d->getInt32() == 1);

      {
         DynamicObject d4;
         d4["cow"] = "moo";
         d4["dog"] = "woof";
         d4["deep"]["cat"] = "meow";
         *d = *d4;
         d4["deep"]["cat"] = "screech";
      }

      {
         DynamicObject expect;
         expect["cow"] = "moo";
         expect["dog"] = "woof";
         expect["deep"]["cat"] = "screech";
         assert(d == expect);
      }

      {
         DynamicObject d5;
         d5[0] = "zero";
         d5[1] = "one";
         d5[2]["two"] = "deep";
         *d = *d5;
         d5[2]["two"] = "wide";
      }

      {
         DynamicObject expect;
         expect[0] = "zero";
         expect[1] = "one";
         expect[2]["two"] = "wide";
         assert(d == expect);
      }

      assert(diaddr == &(*d));
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runDynoReverseTest(TestRunner& tr)
{
   tr.group("DynamicObject reverse");

   tr.test("str");
   {
      DynamicObject d;
      d->setType(String);

      d->clear();
      d->reverse();
      assertStrCmp(d->getString(), "");

      d = "012";
      d->reverse();
      assertStrCmp(d->getString(), "210");

   }
   tr.passIfNoException();

   tr.test("array");
   {
      DynamicObject d;
      d->setType(Array);
      DynamicObject expect;
      expect->setType(Array);

      d->clear();
      expect->clear();
      d->reverse();
      assert(d == expect);

      d->clear();
      expect->clear();
      d[0] = "zero";
      d[1] = "one";
      d[2] = "two";
      d->reverse();
      expect[0] = "two";
      expect[1] = "one";
      expect[2] = "zero";
      assert(d == expect);

      d->clear();
      expect->clear();
      d[0]["zero"] = 0;
      d[1]["one"] = 1;
      d->reverse();
      expect[0]["one"] = 1;
      expect[1]["zero"] = 0;
      assert(d == expect);
   }
   tr.passIfNoException();

   tr.ungroup();
}

static bool customSortTuples(DynamicObject a, DynamicObject b)
{
   return a[0] < b[0];
}

struct StructCustomSortTuples : std::less<DynamicObject>
{
   bool operator()(DynamicObject a, DynamicObject b)
   {
      return a[0] < b[0];
   }
};

class ClassCustomSortTuples : public std::less<DynamicObject>
{
public:
   virtual bool operator()(DynamicObject a, DynamicObject b)
   {
      return a[0] < b[0];
   }
};

static void runDynoSortTest(TestRunner& tr)
{
   tr.group("DynamicObject sort");

   tr.test("default strings");
   {
      DynamicObject d;
      d[0] = "b";
      d[1] = "c";
      d[2] = "a";
      d[3] = "d";
      d.sort();

      DynamicObject expect;
      expect[0] = "a";
      expect[1] = "b";
      expect[2] = "c";
      expect[3] = "d";
      assertDynoCmp(expect, d);
   }
   tr.passIfNoException();

   tr.test("default integers");
   {
      DynamicObject d;
      d[0] = 2;
      d[1] = 3;
      d[2] = 1;
      d[3] = 4;
      d.sort();

      DynamicObject expect;
      expect[0] = 1;
      expect[1] = 2;
      expect[2] = 3;
      expect[3] = 4;
      assertDynoCmp(expect, d);
   }
   tr.passIfNoException();

   tr.test("sort with function");
   {
      DynamicObject d;

      DynamicObject t1;
      t1[0] = "b";
      t1[1] = "foo1";
      d[0] = t1;

      DynamicObject t2;
      t2[0] = "c";
      t2[1] = "foo2";
      d[1] = t2;

      DynamicObject t3;
      t3[0] = "a";
      t3[1] = "foo3";
      d[2] = t3;

      d.sort(&customSortTuples);

      DynamicObject expect;
      expect[0] = t3;
      expect[1] = t1;
      expect[2] = t2;
      assertDynoCmp(expect, d);
   }
   tr.passIfNoException();

   tr.test("sort with object (struct)");
   {
      DynamicObject d;

      DynamicObject t1;
      t1[0] = "b";
      t1[1] = "foo1";
      d[0] = t1;

      DynamicObject t2;
      t2[0] = "c";
      t2[1] = "foo2";
      d[1] = t2;

      DynamicObject t3;
      t3[0] = "a";
      t3[1] = "foo3";
      d[2] = t3;

      StructCustomSortTuples compare;
      d.sort(compare);

      DynamicObject expect;
      expect[0] = t3;
      expect[1] = t1;
      expect[2] = t2;
      assertDynoCmp(expect, d);
   }
   tr.passIfNoException();

   tr.test("sort with object (class)");
   {
      DynamicObject d;

      DynamicObject t1;
      t1[0] = "b";
      t1[1] = "foo1";
      d[0] = t1;

      DynamicObject t2;
      t2[0] = "c";
      t2[1] = "foo2";
      d[1] = t2;

      DynamicObject t3;
      t3[0] = "a";
      t3[1] = "foo3";
      d[2] = t3;

      ClassCustomSortTuples compare;
      d.sort(compare);

      DynamicObject expect;
      expect[0] = t3;
      expect[1] = t1;
      expect[2] = t2;
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runDynoStatsTest(TestRunner& tr)
{
   tr.group("DynamicObject stats");

#ifdef MO_DYNO_COUNTS
   // zeroed stats
   #define SETSTAT(d, livec, deadc, maxc, liveb, deadb, maxb) \
      MO_STMT_START { \
         d["counts"]["live"] = livec; \
         d["counts"]["dead"] = deadc; \
         d["counts"]["max"] = maxc; \
         d["bytes"]["live"] = liveb; \
         d["bytes"]["dead"] = deadb; \
         d["bytes"]["max"] = maxb; \
      } MO_STMT_END
   #define SETTYPESTAT(s, field, livec, deadc, maxc, liveb, deadb, maxb) \
      MO_STMT_START { \
         DynamicObject& d = s[MO_STRINGIFY(field)]; \
         SETSTAT(d, livec, deadc, maxc, liveb, deadb, maxb); \
      } MO_STMT_END
   DynamicObject zero;
   SETTYPESTAT(zero, Object, 0, 0, 0, 0, 0, 0);
   SETTYPESTAT(zero, String, 0, 0, 0, 0, 0, 0);
   SETTYPESTAT(zero, Boolean, 0, 0, 0, 0, 0, 0);
   SETTYPESTAT(zero, Int32, 0, 0, 0, 0, 0, 0);
   SETTYPESTAT(zero, UInt32, 0, 0, 0, 0, 0, 0);
   SETTYPESTAT(zero, Int64, 0, 0, 0, 0, 0, 0);
   SETTYPESTAT(zero, UInt64, 0, 0, 0, 0, 0, 0);
   SETTYPESTAT(zero, Double, 0, 0, 0, 0, 0, 0);
   SETTYPESTAT(zero, Map, 0, 0, 0, 0, 0, 0);
   SETTYPESTAT(zero, Array, 0, 0, 0, 0, 0, 0);
   SETTYPESTAT(zero, Key, 0, 0, 0, 0, 0, 0);
   SETTYPESTAT(zero, StringValue, 0, 0, 0, 0, 0, 0);
   zero["KeyCounts"]["count"] = 0;
   zero["KeyCounts"]["keys"]->setType(Map);

   tr.test("clear");
   {
      DynamicObjectImpl::enableStats(false);
      DynamicObjectImpl::clearStats();
      DynamicObject stats = DynamicObjectImpl::getStats();
      assertDynoCmp(stats, zero);
   }
   tr.passIfNoException();

   tr.test("one live");
   {
      DynamicObjectImpl::enableStats(true);
      DynamicObjectImpl::clearStats();
      DynamicObject d;
      DynamicObjectImpl::enableStats(false);
      DynamicObject stats = DynamicObjectImpl::getStats();
      DynamicObject expect = zero.clone();
      SETTYPESTAT(expect, Object, 1, 0, 1, 0, 0, 0);
      SETTYPESTAT(expect, String, 1, 0, 1, 0, 0, 0);
      assertDynoCmp(stats, expect);
   }
   tr.passIfNoException();

   tr.test("one dead");
   {
      DynamicObjectImpl::enableStats(true);
      DynamicObjectImpl::clearStats();
      {
         DynamicObject d;
      }
      DynamicObjectImpl::enableStats(false);
      DynamicObject stats = DynamicObjectImpl::getStats();
      DynamicObject expect = zero.clone();
      SETTYPESTAT(expect, Object, 0, 1, 1, 0, 0, 0);
      SETTYPESTAT(expect, String, 0, 1, 1, 0, 0, 0);
      assertDynoCmp(stats, expect);
   }
   tr.passIfNoException();

   // FIXME: expected results not correct and checked for accuracy yet
   /*
   tr.test("many sets");
   {
      DynamicObjectImpl::clearStats();
      DynamicObjectImpl::enableStats(true);
      {
         DynamicObject d;
         d->setType(Array);
         DynamicObject& d2 = d->append();
         d2 = "123";
         d2 = "456";
         d2 = "789";
      }
      DynamicObjectImpl::enableStats(false);
      DynamicObject stats = DynamicObjectImpl::getStats();
      DynamicObject expect = zero.clone();
      SETTYPESTAT(expect, Object, 0, 2, 2, 0, 0, 0);
      SETTYPESTAT(expect, Array, 0, 1, 1, 0, 0, 0);
      SETTYPESTAT(expect, String, 0, 3, 1, 0, 9, 3);
      assertDynoCmp(stats, expect);
   }
   tr.passIfNoException();
   */

   tr.test("key counts");
   {
      DynamicObjectImpl::enableStats(true);
      DynamicObjectImpl::clearStats();
      {
         DynamicObject d;
         d["key1"] = true;
      }
      DynamicObjectImpl::enableStats(false);
      DynamicObject stats = DynamicObjectImpl::getStats();
      DynamicObject expect = zero.clone();
      SETTYPESTAT(expect, Object, 0, 2, 2, 0, 0, 0);
      SETTYPESTAT(expect, Map, 0, 1, 1, 0, 0, 0);
      SETTYPESTAT(expect, Boolean, 0, 1, 1, 0, 0, 0);
      SETTYPESTAT(expect, Key, 0, 1, 1, 0, 4, 4);
      SETTYPESTAT(expect, String, 0, 2, 1, 0, 0, 0);
      expect["KeyCounts"]["count"] = 1;
      SETSTAT(expect["KeyCounts"]["keys"]["key1"], 0, 1, 1, 0, 4, 4);
      assertDynoCmp(stats, expect);
   }
   tr.passIfNoException();
#else
   tr.test("[stats disabled]");
   tr.passIfNoException();
#endif

   tr.ungroup();
}

class RunnableDelegateClass
{
public:
   int counter;
   RunnableDelegateClass() {};
   virtual ~RunnableDelegateClass() {};

   virtual void runFunction()
   {
      ++counter;
   };

   virtual void runParamFunction(void* param)
   {
      int* counter = (int*)param;
      ++(*counter);
   };

   virtual void freeParamFunction(void* param)
   {
      int* counter = (int*)param;
      delete counter;
   };

   virtual void runDynoFunction(DynamicObject& dyno)
   {
      dyno["counter"] = dyno["counter"]->getUInt32() + 1;
   };
};

static int gCounter;
static void _runFunction()
{
   ++gCounter;
};

static void _runParamFunction(void* param)
{
   int* counter = (int*)param;
   ++(*counter);
};

static void _freeParamFunction(void* param)
{
   int* counter = (int*)param;
   delete counter;
};

static void _runDynoFunction(DynamicObject& dyno)
{
   dyno["counter"] = dyno["counter"]->getUInt32() + 1;
};

static void runRunnableDelegateTest(TestRunner& tr)
{
   tr.group("RunnableDelegate");

   tr.test("RunnableDelegate Object()");
   {
      RunnableDelegateClass drc;
      drc.counter = 0;

      RunnableRef r = new RunnableDelegate<RunnableDelegateClass>(
         &drc, &RunnableDelegateClass::runFunction);
      Thread* t = new Thread(r);
      t->start();
      t->join();
      delete t;

      assert(drc.counter == 1);
   }
   tr.passIfNoException();

   tr.test("RunnableDelegate f()");
   {
      gCounter = 0;
      RunnableRef r = new RunnableDelegate<void>(_runFunction);
      Thread* t = new Thread(r);
      t->start();
      t->join();
      delete t;

      assert(gCounter == 1);
   }
   tr.passIfNoException();

   tr.test("RunnableDelegate Object(void*)");
   {
      int* counter = new int(0);

      RunnableDelegateClass drc;
      RunnableRef r = new RunnableDelegate<RunnableDelegateClass>(
         &drc, &RunnableDelegateClass::runParamFunction, counter,
         &RunnableDelegateClass::freeParamFunction);
      Thread* t = new Thread(r);
      t->start();
      t->join();
      delete t;

      assert(*counter == 1);
   }
   tr.passIfNoException();

   tr.test("RunnableDelegate f(void*)");
   {
      int* counter = new int(0);

      RunnableRef r = new RunnableDelegate<void>(
         _runParamFunction, counter, _freeParamFunction);
      Thread* t = new Thread(r);
      t->start();
      t->join();
      delete t;

      assert(*counter == 1);
   }
   tr.passIfNoException();

   tr.test("RunnableDelegate Object(DynamicObject)");
   {
      DynamicObject d;
      d["counter"] = 0;

      RunnableDelegateClass drc;
      RunnableRef r = new RunnableDelegate<RunnableDelegateClass>(
         &drc, &RunnableDelegateClass::runDynoFunction, d);
      Thread* t = new Thread(r);
      t->start();
      t->join();
      delete t;

      assert(d["counter"]->getUInt32() == 1);
   }
   tr.passIfNoException();

   tr.test("RunnableDelegate f(DynamicObject)");
   {
      DynamicObject d;
      d["counter"] = 0;

      RunnableRef r = new RunnableDelegate<void>(_runDynoFunction, d);
      Thread* t = new Thread(r);
      t->start();
      t->join();
      delete t;

      assert(d["counter"]->getUInt32() == 1);
   }
   tr.passIfNoException();

   tr.ungroup();
}

static void runExceptionTest(TestRunner& tr)
{
   tr.group("Exceptions");

   tr.test("getCauseOfType");
   {
      ExceptionRef current;

      // setup root
      Exception::clear();
      ExceptionRef root = new Exception("root", "root-type");
      Exception::push(root);

      // check basic type
      current = Exception::get();
      assert(!current->hasType("bogus-type"));
      assert(current->hasType("root-type"));

      // add child
      ExceptionRef child = new Exception("child", "child-type");
      Exception::push(child);

      // check children types
      current = Exception::get();
      assert(!current->hasType("bogus-type"));
      assert(current->hasType("root-type"));
      assert(current->hasType("child-type"));

      // check starting part
      assert(current->hasType("root", true));
      assert(current->hasType("child", true));

      // check starting n part
      assert(current->hasType("chiX", true, 3));

      Exception::clear();
   }
   tr.pass();

   tr.ungroup();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runThreadTest(tr);
      runThreadPoolTest(tr);
      runJobDispatcherTest(tr);
      runExclusiveLockTest(tr);
      runSharedLockTest(tr);
      runCollectableTest(tr);
      runDynamicObjectTest(tr);
      runDynoClearTest(tr);
      runDynoConversionTest(tr);
      runDynoCastTest(tr);
      runDynoRemoveTest(tr);
      runDynoIndexTest(tr);
      runDynoTypeTest(tr);
      runDynoAppendTest(tr);
      runDynoMergeTest(tr);
      runDynoDiffTest(tr);
      runDynoCopyTest(tr);
      runDynoReverseTest(tr);
      runDynoSortTest(tr);
      runDynoStatsTest(tr);
      runRunnableDelegateTest(tr);
      runExceptionTest(tr);
   }
   if(tr.isTestEnabled("cpu-info"))
   {
      runCpuInfoTest(tr);
   }
   if(tr.isTestEnabled("time"))
   {
      runTimeTest(tr);
   }
   if(tr.isTestEnabled("slow-shared-lock"))
   {
      runInteractiveSharedLockTest(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.rt.test", "1.0", mo_test_rt::run)
