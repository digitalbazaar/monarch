/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/rt/Thread.h"

#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/System.h"

#include <cstdlib>
#include <cstring>

using namespace monarch::rt;

// create thread initializer
pthread_once_t Thread::sThreadsInit = PTHREAD_ONCE_INIT;

// create thread specific data keys
pthread_key_t Thread::sCurrentThreadKey;
pthread_key_t Thread::sExceptionKey;

// create invalid thread ID
pthread_t Thread::sInvalidThreadId;

Thread::Thread(Runnable* runnable, const char* name, bool persistent) :
   mPersistent(persistent),
   mRunnable(runnable),
   mRunnableRef(NULL),
   mName(NULL),
   mUserData(NULL),
   mWaitMonitor(NULL)
{
   // initialize threads
   pthread_once(&sThreadsInit, &initializeThreads);

   if(name != NULL)
   {
      // set name
      Thread::assignName(name);
   }

   // thread is not interrupted or joined yet
   mInterrupted = false;
   mJoined = false;

   // thread is not alive, detached, or started yet
   mAlive = false;
   mDetached = false;
   mCleanup = false;
   mStarted = false;
}

Thread::Thread(RunnableRef& runnable, const char* name, bool persistent) :
   mPersistent(persistent),
   mRunnable(&(*runnable)),
   mRunnableRef(runnable),
   mName(NULL),
   mUserData(NULL),
   mWaitMonitor(NULL)
{
   // initialize threads
   pthread_once(&sThreadsInit, &initializeThreads);

   if(name != NULL)
   {
      // set name
      Thread::assignName(name);
   }

   // thread is not interrupted or joined yet
   mInterrupted = false;
   mJoined = false;

   // thread is not alive, detached, or started yet
   mAlive = false;
   mDetached = false;
   mCleanup = false;
   mStarted = false;
}

Thread::~Thread()
{
   free(mName);
}

bool Thread::start(size_t stackSize)
{
   bool rval = false;

   if(!hasStarted())
   {
      // initialize POSIX thread attributes
      pthread_attr_t attributes;
      pthread_attr_init(&attributes);

      if(stackSize > 0)
      {
         // set thread stack size
         pthread_attr_setstacksize(&attributes, stackSize);
      }

      // make thread joinable
      pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_JOINABLE);

      // create the POSIX thread
      int rc = pthread_create(
         &mThreadId, &attributes, &Thread::execute, (void*)this);

      // destroy POSIX thread attributes
      pthread_attr_destroy(&attributes);

      // if the thread was created successfully and has a valid ID,
      // then return true
      if(rc == 0 && isThreadIdValid(mThreadId))
      {
         // thread has started
         mStarted = true;
         rval = true;
      }
      else
      {
         switch(rc)
         {
            case EAGAIN:
            {
               ExceptionRef e = new Exception(
                  "Could not start thread. Not enough system resources.",
                  "monarch.rt.Thread.InsufficientResources");
               e->getDetails()["error"] = strerror(errno);
               Exception::set(e);
               break;
            }
            case EINVAL:
            {
               ExceptionRef e = new Exception(
                  "Could not start thread. Invalid thread parameters.",
                  "monarch.rt.Thread.InvalidParameters");
               e->getDetails()["error"] = strerror(errno);
               Exception::set(e);
               break;
            }
            case EPERM:
            {
               ExceptionRef e = new Exception(
                  "Could not start thread. Not authorized.",
                  "monarch.rt.Thread.AccessDenied");
               e->getDetails()["error"] = strerror(errno);
               Exception::set(e);
               break;
            }
            case ENOMEM:
            {
               ExceptionRef e = new Exception(
                  "Could not start thread. Not enough memory.",
                  "monarch.rt.Thread.InsufficientMemory");
               e->getDetails()["error"] = strerror(errno);
               Exception::set(e);
               break;
            }
            default:
            {
               ExceptionRef e = new Exception(
                  "Could not start thread.",
                  "monarch.rt.Thread.Error");
               e->getDetails()["error"] = strerror(errno);
               Exception::set(e);
               break;
            }
         }
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

Exception* Thread::createInterruptedException()
{
   Exception* rval = NULL;

   rval = new Exception("Thread interrupted", "monarch.rt.Interrupted");
   const char* name = getName();
   rval->getDetails()["name"] = ((name == NULL) ? "" : name);

   return rval;
}

void Thread::interrupt()
{
   lock();

   // only interrupt if not already interrupted
   if(!isInterrupted())
   {
      // set interrupted flag
      mInterrupted = true;

      // Note: disabled due to lack of support in windows
      // send SIGINT to thread
      //sendSignal(SIGINT);

      // store thread's current monitor
      Monitor* m = mWaitMonitor;
      unlock();

      // wake up thread it is inside of a monitor
      if(m != NULL)
      {
         m->enter();
         m->signalAll();
         m->exit();
      }
   }
   else
   {
      unlock();
   }
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

   if(join && hasStarted())
   {
      // join thread, wait for it to detach/terminate indefinitely
      pthread_join(mThreadId, NULL);
   }
}

void Thread::detach(bool cleanup)
{
   bool detach = false;

   lock();
   {
      // check for previous detachments/joins
      if(!mDetached && !mJoined)
      {
         detach = true;
         mDetached = true;
         mCleanup = cleanup;
      }
   }
   unlock();

   if(detach && hasStarted())
   {
      // detach thread
      pthread_detach(mThreadId);
   }
}

pthread_t Thread::getId()
{
   return mThreadId;
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

void Thread::setUserData(void* userData)
{
   mUserData = userData;
}

void* Thread::getUserData()
{
   return mUserData;
}

Thread* Thread::currentThread()
{
   // initialize threads
   pthread_once(&sThreadsInit, &initializeThreads);

   // get a pointer to the current thread
   Thread* rval = static_cast<Thread*>(pthread_getspecific(sCurrentThreadKey));
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

bool Thread::sleep(unsigned int time)
{
   bool rval = true;

   // enter an arbitrary monitor
   Monitor m;
   m.enter();
   {
      // wait to re-enter the monitor until the specified time
      uint32_t remaining = time;
      uint64_t st = System::getCurrentMilliseconds();
      uint64_t et;
      uint64_t dt;
      while(rval && (time == 0 || remaining > 0))
      {
         rval = waitToEnter(&m, remaining);
         if(rval && time > 0)
         {
            // update remaining time
            et = System::getCurrentMilliseconds();
            dt = et - st;
            remaining = (dt >= remaining ? 0 : remaining - dt);
            st = et;
         }
      }
   }
   m.exit();

   return rval;
}

void Thread::yield()
{
   sched_yield();
}

bool Thread::waitToEnter(Monitor* m, uint32_t timeout)
{
   bool rval = true;

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
      ExceptionRef e = t->createInterruptedException();
      setException(e, false);
      rval = false;
   }

   return rval;
}

void Thread::exit(bool exitMain)
{
   Thread* thread = Thread::currentThread();

   if(!thread->mPersistent)
   {
      // thread is main thread, clean up its per-thread values
      cleanupCurrentThreadKeyValue(thread);
      cleanupExceptionKeyValue(
         static_cast<ExceptionRef*>(pthread_getspecific(sExceptionKey)));

      // ensure per-thread key values are NULL
      pthread_setspecific(sCurrentThreadKey, NULL);
      pthread_setspecific(sExceptionKey, NULL);

      if(exitMain)
      {
         // exit main thread
         pthread_exit(NULL);
      }
   }
   else
   {
      // exit current thread
      pthread_exit(NULL);
   }
}

void Thread::setException(ExceptionRef& e, bool caused)
{
   // initialize threads
   pthread_once(&sThreadsInit, &initializeThreads);

   // get the exception reference for the current thread
   ExceptionRef* ref =
      static_cast<ExceptionRef*>(pthread_getspecific(sExceptionKey));
   if(ref == NULL)
   {
      // create the exception reference
      ref = new ExceptionRef(NULL);
      *ref = e;
      pthread_setspecific(sExceptionKey, ref);
   }
   else
   {
      if(caused && !e.isNull())
      {
         // set cause of passed exception to previous exception
         e->setCause(*ref);
      }

      // update the reference
      *ref = e;
   }
}

ExceptionRef Thread::getException()
{
   // initialize threads
   pthread_once(&sThreadsInit, &initializeThreads);

   // get the exception reference for the current thread
   ExceptionRef* ref =
      static_cast<ExceptionRef*>(pthread_getspecific(sExceptionKey));
   if(ref == NULL)
   {
      // create the exception reference
      ref = new ExceptionRef(NULL);
      pthread_setspecific(sExceptionKey, ref);
   }

   // return the reference
   return *ref;
}

bool Thread::hasException()
{
   bool rval = false;

   // initialize threads
   pthread_once(&sThreadsInit, &initializeThreads);

   // get the exception reference for the current thread
   ExceptionRef* ref =
      static_cast<ExceptionRef*>(pthread_getspecific(sExceptionKey));
   if(ref != NULL)
   {
      // return true if the reference isn't to NULL
      rval = !ref->isNull();
   }

   return rval;
}

void Thread::clearException()
{
   // get the exception reference for the current thread
   ExceptionRef* ref =
      static_cast<ExceptionRef*>(pthread_getspecific(sExceptionKey));
   if(ref != NULL)
   {
      // clear the reference
      ref->setNull();
   }
}

bool Thread::isThreadIdValid(pthread_t id)
{
   // pthread_equal() returns non-zero when the IDs *are* equal, 0 otherwise.
   return (pthread_equal(id, sInvalidThreadId) == 0);
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
   free(mName);
   mName = (name != NULL) ? strdup(name) : NULL;
}

void Thread::initializeThreads()
{
   // create the thread specific data keys
   pthread_key_create(&sCurrentThreadKey, &cleanupCurrentThreadKeyValue);
   pthread_key_create(&sExceptionKey, &cleanupExceptionKeyValue);

   // set invalid thread ID
#ifdef WIN32
   /**
    * Windows pthreads use ptw32_handle_t for pthread_t which is:
    *
    * ptw32_handle_t
    * {
    *    void* p;        // pointer to thread object
    *    unsigned int x; // some extra information about reuse, etc.
    * };
    *
    * A value of 0 for p is considered invalid.
    */
   sInvalidThreadId.p = 0;
   sInvalidThreadId.x = 0;
#else
   /**
    * Other builds just use a number for pthread_t. 0 is considered invalid.
    */
   sInvalidThreadId = 0;
#endif

   // Note: disabled due to a lack of support in windows
   // install signal handler
   //installSigIntHandler();
}

void Thread::cleanupCurrentThreadKeyValue(void* thread)
{
   // check for thread non-persistence
   Thread* t = static_cast<Thread*>(thread);
   if(thread != NULL && !t->mPersistent)
   {
      delete t;
   }
}

void Thread::cleanupExceptionKeyValue(void* er)
{
   if(er != NULL)
   {
      // clean up exception reference
      ExceptionRef* ref = static_cast<ExceptionRef*>(er);
      delete ref;
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
   // do not allow invalid thread IDs
   pthread_t self = pthread_self();
   if(isThreadIdValid(self))
   {
      // get the Thread object
      Thread* t = static_cast<Thread*>(thread);

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

      // if thread is persistent but clean up is on, delete it
      if(t->mPersistent && t->mCleanup)
      {
         delete t;
      }
   }
   else
   {
      // detach thread and let it die, it's invalid
      pthread_detach(self);
   }

   // exit thread
   pthread_exit(NULL);
   return NULL;
}
