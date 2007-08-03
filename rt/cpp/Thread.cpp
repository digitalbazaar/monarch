/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Thread.h"
#include "System.h"

using namespace db::rt;

// initialize main thread
Thread Thread::MAIN_THREAD(NULL, "main");

// initialize current thread key parameters
pthread_once_t Thread::CURRENT_THREAD_KEY_INIT = PTHREAD_ONCE_INIT;
pthread_key_t Thread::CURRENT_THREAD_KEY;

// initialize exception key parameters
pthread_once_t Thread::EXCEPTION_KEY_INIT = PTHREAD_ONCE_INIT;
pthread_key_t Thread::EXCEPTION_KEY;

// Note: disabled due to a lack of support in windows
// initialize signal handler parameters
//pthread_once_t Thread::SIGINT_HANDLER_INIT = PTHREAD_ONCE_INIT;

Thread::Thread(Runnable* runnable, const char* name)
{
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
   
   if(this == &MAIN_THREAD)
   {
      // create the current thread key
      pthread_once(&CURRENT_THREAD_KEY_INIT, Thread::createCurrentThreadKey);
      
      // create the exception key
      pthread_once(&EXCEPTION_KEY_INIT, Thread::createExceptionKey);
      
      // Note: disabled due to a lack of support in windows
      // if support becomes available in the future, make the main
      // thread handle all signals
      // install signal handler
      //pthread_once(&SIGINT_HANDLER_INIT, Thread::installSigIntHandler);
      
      // on the main thread, so initialize main thread data
      mThreadId = pthread_self();
      mAlive = true;
      mDetached = true;
      mStarted = true;
      pthread_setspecific(CURRENT_THREAD_KEY, this);
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
   }
   else
   {
      // thread is not alive, detached, or started yet
      mAlive = false;
      mDetached = false;
      mStarted = false;
   }
}

Thread::~Thread()
{
   // delete name
   if(mName != NULL)
   {
      delete [] mName;
   }
   
   if(this == &MAIN_THREAD)
   {
      // exit main thread
      pthread_exit(NULL);
   }
}

void Thread::run()
{
   // if a Runnable if available, use it
   if(mRunnable != NULL)
   {
      mRunnable->run();
   }
}

void Thread::assignName(const char* name)
{
   if(name == NULL)
   {
      if(mName == NULL)
      {
         mName = new char[1];
      }
      else if(strlen(mName) > 0)
      {
         delete mName;
         mName = new char[1];
      }
      
      mName[0] = 0;
   }
   else
   {
      if(mName == NULL)
      {
         mName = new char[100];
      }
      else if(strlen(mName) == 0)
      {
         delete mName;
         mName = new char[100];
      }
      
      unsigned int length = strlen(name);
      length = (length > 99 ? 99 : length);
      memcpy(mName, name, length);
      memset(mName + length, 0, 1);
   }
}

void Thread::createCurrentThreadKey()
{
   // create the thread key for obtaining the current thread
   pthread_key_create(&CURRENT_THREAD_KEY, &cleanupCurrentThreadKeyValue);
}

void Thread::cleanupCurrentThreadKeyValue(void* thread)
{
   // no action is necessary, key is automatically set to NULL
}

void Thread::createExceptionKey()
{
   // create the thread key for obtaining the last thread-local exception
   pthread_key_create(&EXCEPTION_KEY, &cleanupExceptionKeyValue);
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
   pthread_setspecific(CURRENT_THREAD_KEY, t);
   
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
         &mThreadId, &attributes, execute, (void*)this);
      
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
   // synchronize
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
   
   // synchronize
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
   
   // synchronize
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
   
   char* msg = new char[8 + strlen(getName()) + 13 + 1];
   sprintf(msg, "Thread '%s' interrupted", getName());
   
   rval = new InterruptedException(msg);
   delete msg;
   
   return rval;
}

Thread* Thread::currentThread()
{
   // get a pointer to the current thread
   return (Thread*)pthread_getspecific(CURRENT_THREAD_KEY);
}

bool Thread::interrupted(bool clear)
{
   bool rval = false;
   
   // get the current thread's interrupted status
   Thread* t = Thread::currentThread();
   
   // synchronize
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

void Thread::setException(Exception* e)
{
   // get the existing exception for the current thread, if any
   Exception* existing = getException();
   if(existing != e)
   {
      // replace the existing exception
      pthread_setspecific(EXCEPTION_KEY, e);
      
      if(existing != NULL)
      {
         // delete the old exception
         delete existing;
      }
   }
}

Exception* Thread::getException()
{
   // get the exception for the current thread, if any
   return (Exception*)pthread_getspecific(EXCEPTION_KEY);
}

bool Thread::hasException()
{
   return getException() != NULL;
}

void Thread::clearException()
{
   setException(NULL);
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
