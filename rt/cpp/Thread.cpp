/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Thread.h"
#include "System.h"

using namespace std;
using namespace db::rt;

// initialize current thread key parameters
pthread_once_t Thread::CURRENT_THREAD_KEY_INIT = PTHREAD_ONCE_INIT;
pthread_key_t Thread::CURRENT_THREAD_KEY;

// initialize exception key parameters
pthread_once_t Thread::EXCEPTION_KEY_INIT = PTHREAD_ONCE_INIT;
pthread_key_t Thread::EXCEPTION_KEY;

Thread::Thread(Runnable* runnable, std::string name)
{
   // initialize POSIX thread attributes
   pthread_attr_init(&mPThreadAttributes);
   
   // make thread joinable
   pthread_attr_setdetachstate(&mPThreadAttributes, PTHREAD_CREATE_JOINABLE);
   
   // make thread cancelable upon joins/waits/etc
   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
   
   // thread not waiting to enter a Monitor yet
   mWaitMonitor = NULL;
   
   // store runnable
   mRunnable = runnable;
   
   // set name
   mName = name;
   
   // thread is not alive yet
   mAlive = false;
   
   // thread is not interrupted yet
   mInterrupted = false;
   
   // thread is not started yet
   mStarted = false;
}

Thread::~Thread()
{
   // destroy the POSIX thread attributes
   pthread_attr_destroy(&mPThreadAttributes);
}

void Thread::run()
{
   // create the current thread key if it hasn't been created yet
   pthread_once(&CURRENT_THREAD_KEY_INIT, Thread::createCurrentThreadKey);
   
   // set thread specific data for current thread to "this" pointer
   pthread_setspecific(CURRENT_THREAD_KEY, this);
   
   // create the exception key if it hasn't been created yet
   pthread_once(&EXCEPTION_KEY_INIT, Thread::createExceptionKey);
   
   // set thread specific data for exception to NULL (no exception yet)
   pthread_setspecific(EXCEPTION_KEY, NULL);
   
   // thread is alive
   mAlive = true;
   
   // if a Runnable if available, use it
   if(mRunnable != NULL)
   {
      mRunnable->run();
   }
   
   // thread is no longer alive
   mAlive = false;
   
   // clean up any exception
   setException(NULL);
}

void Thread::createCurrentThreadKey()
{
   // create the thread key for obtaining the current thread
   pthread_key_create(&CURRENT_THREAD_KEY, NULL);
}

void Thread::createExceptionKey()
{
   // create the thread key for obtaining the last thread-local exception
   pthread_key_create(&EXCEPTION_KEY, NULL);
}

void* Thread::execute(void* thread)
{
   // run the passed thread's run() method
   Thread* t = (Thread*)thread;
   t->run();
   
   // exit thread
   pthread_exit(NULL);
   return NULL;
}

bool Thread::start()
{
   bool rval = false;
   
   // create the POSIX thread
   int rc = pthread_create(
      &mPThread, &mPThreadAttributes, execute, (void*)this);
      
   // if the thread was created successfully, return true
   if(rc == 0)
   {
      // thread has started
      mStarted = true;
      rval = true;
   }
   
   return rval;
}

bool Thread::isAlive()
{
   return mAlive;
}

void Thread::interrupt()
{
   // synchronize
   lock();
   {
      // only interrupt if not already interrupted
      if(!isInterrupted())
      {
         // set interrupted flag
         mInterrupted = true;
         
         // wake up thread if necessary
         if(mWaitMonitor != NULL)
         {
            mWaitMonitor->signalAll();
         }
      }
   }
   unlock();
}

bool Thread::isInterrupted()
{
   return mInterrupted;
}

bool Thread::hasStarted()
{
   return mStarted;
}

void Thread::join()
{
   // synchronize
   lock();
   {
      // join thread, wait for it to detach/terminate indefinitely
      int status;
      pthread_join(mPThread, (void **)&status);
   }
   unlock();
}

void Thread::detach()
{
   // synchronize
   lock();
   {
      // detach thread
      pthread_detach(mPThread);
   }
   unlock();
}

void Thread::setName(string name)
{
   mName = name;
}

const string& Thread::getName()
{
   return mName;
}

Thread* Thread::currentThread()
{
   // get a pointer to the current thread
   return (Thread*)pthread_getspecific(CURRENT_THREAD_KEY);
}

bool Thread::interrupted()
{
   bool rval = false;
   
   // get the current thread's interrupted status
   Thread* thread = Thread::currentThread();
   rval = thread->isInterrupted();
   
   // clear interrupted flag
   thread->mInterrupted = false;
   
   return rval;
}

void Thread::sleep(unsigned long time) throw(InterruptedException)
{
   // FIXME: make thread return some kind of interrupted code instead
   // of using an exception
   
   // create a lock object
   Object lock;
   
   lock.lock();
   {
      try
      {
         // FIXME: get interrupted code from the wait method instead of
         // using an exception
         
         // wait on the lock object for the specified time
         lock.wait(time);
      }
      catch(InterruptedException& e)
      {
         lock.unlock();
         throw e;
      }
   }
   lock.unlock();
}

void Thread::yield() throw(InterruptedException)
{
   pthread_testcancel();
   sched_yield();
}

void Thread::setException(Exception* e)
{
   // store the existing exception for the current thread, if any
   Exception* existing = getException();
   
   // replace the existing exception
   pthread_setspecific(EXCEPTION_KEY, e);
   
   // delete the old exception
   delete existing;
}

Exception* Thread::getException()
{
   // get the exception for the current thread, if any
   return (Exception*)pthread_getspecific(EXCEPTION_KEY);
}

void Thread::waitToEnter(Monitor* m, unsigned long timeout)
{
   // get the current thread and set its wait monitor
   Thread* t = currentThread();
   t->mWaitMonitor = m;
   
   // get the current time and determine if wait should be indefinite
   unsigned long long time = System::getCurrentMilliseconds();
   bool indefinite = timeout == 0;
   
   // wait while not interrupted, must wait, and timeout not exhausted
   while(!t->isInterrupted() && m->mustWait() && (indefinite || timeout > 0))
   {
      m->wait(timeout);
      timeout -= (System::getCurrentMilliseconds() - time);
   }
   
   // FIXME: create interrupted exception if interrupted
}
