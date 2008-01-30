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
         InterruptedException* e = NULL;
         
         lock();
         {
            lock();
            lock();
            lock();
            e = wait();
            // thread 1 should be interrupted
            assert(e != NULL);
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
               assert(wait(5000) == NULL);
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
