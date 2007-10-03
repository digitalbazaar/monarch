/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/Thread.h"
#include "db/rt/System.h"

using namespace db::rt;

// create thread initializer
pthread_once_t Thread::sThreadsInit = PTHREAD_ONCE_INIT;

// create thread specific data keys
pthread_key_t Thread::sCurrentThreadKey;
pthread_key_t Thread::sExceptionKey;

Thread::Thread(Runnable* runnable, const char* name, bool persistent)
{
   // initialize threads
   pthread_once(&sThreadsInit, &initializeThreads);
   
   // store persistent setting
   mPersistent = persistent;
   
   // store runnable
   mRunnable = runnable;
   
   // set name
   mName = NULL;
   Thread::assignName(name);
   
   // thread not waiting to enter a Monitor yet
   mWaitMonitor = NULL;
   
   // thread is not interrupted or joined yet
   mInterrupted = false;
   mJoined = false;
   
   // thread is not alive, detached, or started yet
   mAlive = false;
   mDetached = false;
   mStarted = false;
}

Thread::~Thread()
{
   if(mName != NULL)
   {
      // delete name
      delete [] mName;
   }
}

void Thread::run()
{
   // if a Runnable is available, use it
   if(mRunnable != NULL)
   {
      mRunnable->run();
   }
}

void Thread::assignName(const char* name)
{
   // delete old name
   if(mName != NULL)
   {
      delete [] mName;
   }
   
   if(name == NULL)
   {
      mName = NULL;
   }
   else
   {
      mName = new char[strlen(name)];
      strcpy(mName, name);
   }
}

void Thread::initializeThreads()
{
   // create the thread specific data keys
   pthread_key_create(&sCurrentThreadKey, &cleanupCurrentThreadKeyValue);
   pthread_key_create(&sExceptionKey, &cleanupExceptionKeyValue);
   
   // Note: disabled due to a lack of support in windows
   // install signal handler
   //installSigIntHandler();
}

void Thread::cleanupCurrentThreadKeyValue(void* thread)
{
   // check for thread non-persistence
   Thread* t = (Thread*)thread;
   if(!t->mPersistent)
   {
      delete t;
   }
}

void Thread::cleanupExceptionKeyValue(void* e)
{
   if(e != NULL)
   {
      // clean up exception
      Exception* ex = (Exception*)e;
      delete ex;
   }
}

// Note: disabled due to a lack of support in windows
//void Thread::installSigIntHandler()
//{
//   // create the SIGINT handler
//   struct sigaction newsa;
//   newsa.sa_handler = handleSigInt;
//   newsa.sa_flags = 0;
//   sigemptyset(&newsa.sa_mask);
//   
//   // set the SIGINT handler
//   sigaction(SIGINT, &newsa, NULL);
//}
//
//void Thread::handleSigInt(int signum)
//{
//   // no action is necessary, thread already interrupted
//}

void* Thread::execute(void* thread)
{
   // get the Thread object
   Thread* t = (Thread*)thread;
   
   // set thread specific data for current thread to the Thread
   pthread_setspecific(sCurrentThreadKey, t);
   
   // disable thread cancelation
   pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
   
   // thread is alive
   t->mAlive = true;
   
   // run the passed thread's run() method
   t->run();
   
   // thread is no longer alive
   t->mAlive = false;
   
   // exit thread
   pthread_exit(NULL);
   return NULL;
}

bool Thread::start()
{
   bool rval = false;
   
   if(!hasStarted())
   {
      // initialize POSIX thread attributes
      pthread_attr_t attributes;
      pthread_attr_init(&attributes);
      
      // make thread joinable
      pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_JOINABLE);
      
      // create the POSIX thread
      int rc = pthread_create(
         &mThreadId, &attributes, &Thread::execute, (void*)this);
      
      // destroy POSIX thread attributes
      pthread_attr_destroy(&attributes);
      
      // if the thread was created successfully, return true
      if(rc == 0)
      {
         // thread has started
         mStarted = true;
         rval = true;
      }
   }
   
   return rval;
}

// Note: disabled due to lack of support in windows
//void Thread::sendSignal(int signum)
//{
//   lock();
//   {
//      if(hasStarted() && isAlive())
//      {
//         pthread_kill(mThreadId, signum);
//      }
//   }
//   unlock();
//}

bool Thread::isAlive()
{
   return mAlive;
}

void Thread::interrupt()
{
   lock();
   {
      // only interrupt if not already interrupted
      if(!isInterrupted())
      {
         // set interrupted flag
         mInterrupted = true;
         
         // Note: disabled due to lack of support in windows
         // send SIGINT to thread
         //sendSignal(SIGINT);
         
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
   bool join = false;
   
   lock();
   {
      // check for previous detachments/joins
      if(!mDetached && !mJoined)
      {
         join = true;
         mJoined = true;
      }
   }
   unlock();
   
   if(join)
   {
      // join thread, wait for it to detach/terminate indefinitely
      int status;
      pthread_join(mThreadId, (void **)&status);
   }
}

void Thread::detach()
{
   bool detach = false;
   
   lock();
   {
      // check for previous detachments/joins
      if(!mDetached && !mJoined)
      {
         detach = true;
         mDetached = true;
      }
   }
   unlock();
   
   if(detach)
   {
      // detach thread
      pthread_detach(mThreadId);
   }
}

void Thread::setName(const char* name)
{
   lock();
   {
      assignName(name);
   }
   unlock();
}

const char* Thread::getName()
{
   const char* rval = NULL;
   
   lock();
   {
      rval = mName;
   }
   unlock();
   
   return rval;
}

InterruptedException* Thread::createInterruptedException()
{
   InterruptedException* rval = NULL;
   
   unsigned int length = (getName() == NULL) ? 0 : strlen(getName());
   char* msg = new char[8 + length + 13 + 1];
   sprintf(msg, "Thread '%s' interrupted",
      (getName() == NULL) ? "" : getName());
   
   rval = new InterruptedException(msg);
   delete msg;
   
   return rval;
}

Thread* Thread::currentThread()
{
   // initialize threads
   pthread_once(&sThreadsInit, &initializeThreads);
   
   // get a pointer to the current thread
   Thread* rval = (Thread*)pthread_getspecific(sCurrentThreadKey);
   if(rval == NULL)
   {
      // create non-persistent thread
      rval = new Thread(NULL, NULL, false);
      
      // initialize thread data
      rval->mThreadId = pthread_self();
      rval->mAlive = true;
      rval->mStarted = true;
      pthread_setspecific(sCurrentThreadKey, rval);
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
   }
   
   return rval;
}

bool Thread::interrupted(bool clear)
{
   bool rval = false;
   
   // get the current thread's interrupted status
   Thread* t = Thread::currentThread();
   
   t->lock();
   {
      if(t->isInterrupted())
      {
         rval = true;
         
         if(clear)
         {
            // clear interrupted flag
            t->mInterrupted = false;
         }
      }
   }
   t->unlock();
   
   return rval;
}

InterruptedException* Thread::sleep(unsigned long time)
{
   InterruptedException* rval = NULL;
   
   // enter an arbitrary monitor
   Monitor m;
   m.enter();
   {
      // wait to re-enter the monitor until the specified time
      rval = waitToEnter(&m, time);
   }
   m.exit();
   
   return rval;
}

void Thread::yield()
{
   sched_yield();
}

InterruptedException* Thread::waitToEnter(Monitor* m, unsigned long timeout)
{
   InterruptedException* rval = NULL;
   
   // set the current thread's wait monitor
   Thread* t = currentThread();
   t->lock();
   {
      t->mWaitMonitor = m;
   }
   t->unlock();
   
   // wait if not interrupted and timeout not exhausted
   if(!t->isInterrupted())
   {
      m->wait(timeout);
   }
   
   // clear the current thread's wait monitor
   t->lock();
   {
      t->mWaitMonitor = NULL;
   }
   t->unlock();
   
   // create interrupted exception if interrupted
   if(t->isInterrupted())
   {
      // set exception
      rval = t->createInterruptedException();
      setException(rval);
   }
   
   return rval;
}

void Thread::exit()
{
   pthread_exit(NULL);
}

void Thread::setException(Exception* e, bool cleanup)
{
   // get the existing exception for the current thread, if any
   Exception* existing = getException();
   if(existing != e)
   {
      // replace the existing exception
      pthread_setspecific(sExceptionKey, e);
      
      // clean up existing exception as appropriate
      if(existing != NULL && cleanup)
      {
         // ensure cause is not deleted
         if(e != NULL && existing != e->getCause())
         {
            // delete the old exception
            delete existing;
         }
      }
   }
}

Exception* Thread::getException()
{
   // initialize threads
   pthread_once(&sThreadsInit, &initializeThreads);
   
   // get the exception for the current thread, if any
   return (Exception*)pthread_getspecific(sExceptionKey);
}

bool Thread::hasException()
{
   return getException() != NULL;
}

void Thread::clearException(bool cleanup)
{
   setException(NULL, cleanup);
}

// Note: disabled due to a lack of support in windows
//void Thread::setSignalMask(const sigset_t* newmask, sigset_t* oldmask)
//{
//   // set signal mask for this thread
//   pthread_sigmask(SIG_SETMASK, newmask, oldmask);
//}
//
//void Thread::blockSignal(int signum)
//{
//   // unblock signal on this thread
//   sigset_t newset;
//   sigemptyset(&newset);
//   sigaddset(&newset, signum);
//   pthread_sigmask(SIG_BLOCK, &newset, NULL);
//}
//
//void Thread::unblockSignal(int signum)
//{
//   // unblock signal on this thread
//   sigset_t newset;
//   sigemptyset(&newset);
//   sigaddset(&newset, signum);
//   pthread_sigmask(SIG_UNBLOCK, &newset, NULL);
//}
//
//void Thread::setSignalHandler(
//   int signum, const struct sigaction* newaction, struct sigaction* oldaction)
//{
//   // set signal handler
//   sigaction(signum, newaction, oldaction);
//}
