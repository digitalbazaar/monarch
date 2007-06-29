/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Thread.h"
#include "System.h"
#include "Math.h"

using namespace std;
using namespace db::rt;

// FIXME: REMOVE ME!
#include <iostream>

// initialize current thread key parameters
pthread_once_t Thread::CURRENT_THREAD_KEY_INIT = PTHREAD_ONCE_INIT;
pthread_key_t Thread::CURRENT_THREAD_KEY;

// initialize exception key parameters
pthread_once_t Thread::EXCEPTION_KEY_INIT = PTHREAD_ONCE_INIT;
pthread_key_t Thread::EXCEPTION_KEY;

// initialize signal handler parameters
pthread_once_t Thread::SIGINT_HANDLER_INIT = PTHREAD_ONCE_INIT;

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
   
   // thread not detached yet
   mDetached = false;
   
   // thread is not interrupted yet
   mInterrupted = false;
   
   // thread has not joined yet
   mJoined = false;
   
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
   // if a Runnable if available, use it
   if(mRunnable != NULL)
   {
      mRunnable->run();
   }
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

void Thread::installSigIntHandler()
{
   // create the SIGINT handler
   struct sigaction newsa;
   newsa.sa_handler = handleSigInt;
   newsa.sa_flags = 0;
   sigemptyset(&newsa.sa_mask);
   
   // set the SIGINT handler
   sigaction(SIGINT, &newsa, NULL);
}

void Thread::handleSigInt(int signum)
{
   // no action is necessary, thread already interrupted
}

void* Thread::execute(void* thread)
{
   // get the Thread object
   Thread* t = (Thread*)thread;
   
   // create the current thread key, if not created
   pthread_once(&CURRENT_THREAD_KEY_INIT, Thread::createCurrentThreadKey);
   
   // create the exception key, if not created
   pthread_once(&EXCEPTION_KEY_INIT, Thread::createExceptionKey);
   
   // set thread specific data for current thread to the Thread
   pthread_setspecific(CURRENT_THREAD_KEY, t);
   
   // set thread specific data for exception to NULL (no exception yet)
   pthread_setspecific(EXCEPTION_KEY, NULL);
   
   // install signal handler
   pthread_once(&SIGINT_HANDLER_INIT, Thread::installSigIntHandler);
   
   // thread is alive
   t->mAlive = true;
   
   // run the passed thread's run() method
   t->run();
   
   // thread is no longer alive
   t->mAlive = false;
   
   // clean up any exception
   setException(NULL);   
   
   // detach thread
   t->detach();
   
   // exit thread
   pthread_exit(NULL);
   return NULL;
}

bool Thread::start()
{
   bool rval = false;
   
   if(!hasStarted())
   {
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
   }
   
   return rval;}

void Thread::sendSignal(int signum)
{
   lock();
   {
      if(hasStarted() && isAlive())
      {
         pthread_kill(mPThread, signum);
      }
   }
   unlock();
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
         
         // send SIGINT to thread
         sendSignal(SIGINT);
         
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
      pthread_join(mPThread, (void **)&status);
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
      pthread_detach(mPThread);
   }
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

bool Thread::interrupted(bool clear)
{
   bool rval = false;
   
   // get the current thread's interrupted status
   Thread* t = Thread::currentThread();
   if(t != NULL)
   {
      // synchronize
      t->lock();
      {
         if(t->isInterrupted())
         {
            rval = true;
         }
         else if(clear)
         {
            // clear interrupted flag
            t->mInterrupted = false;
         }
      }
      t->unlock();
   }
   
   return rval;
}

InterruptedException* Thread::sleep(unsigned long time)
{
   InterruptedException* rval = NULL;
   
   // create a lock object
   Object lock;
   
   lock.lock();
   {
      // wait on the lock object for the specified time
      rval = lock.wait(time);
   }
   lock.unlock();
   
   return rval;
}

void Thread::yield()
{
   sched_yield();
}

int Thread::select(
   int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
   unsigned long long timeout, const sigset_t* sigmask)
{
   // create timeout
   struct timeval* tv = NULL;
   struct timeval to;
   if(timeout > 0)
   {
      // set timeout (1 millisecond is 1000 microseconds) 
      to.tv_sec = timeout / 1000LL;
      to.tv_usec = (timeout % 1000LL) * 1000LL;
      tv = &to;
   }
   
   return select(nfds, readfds, writefds, exceptfds, tv, sigmask);
}

int Thread::select(
   int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
   struct timeval* timeout, const sigset_t* sigmask)
{
   int rval = 0;
   
   // FIXME: signals supposedly don't make select() return in windows
   // this needs to be tested and potentially remedied somehow
   
   // FIXME: furthermore, even if we block SIGINT (interruption signal) up
   // until we reach the select call -- and then unblock right before it
   // the signal could still sneak in right before select() is called and
   // control is transferred to the kernel, and therefore we'd handle the
   // SIGINT before the select() call and select() wouldn't get interrupted
   // (there is pselect() for doing that unblocking atomically, but
   // it's UNIX only) -- this may be solved by writing to another file
   // descriptor when we receive SIGINTs and checking that file descriptor
   // as well as the one we are waiting on -- but this might not be a
   // viable solution for windows
   
   // block SIGINTs
   blockSignal(SIGINT);
   
   Thread* t = Thread::currentThread();
   if(!t->isInterrupted())
   {
      // FIXME: pselect() required here to do atomic unblocking & selecting
      
      // wait for data to arrive for reading or for an exception
      unblockSignal(SIGINT);
      rval = ::select(nfds, readfds, writefds, exceptfds, timeout);
      if(rval < 0)
      {
         if(errno == EINTR)
         {
            // interrupt thread
            t->interrupt();
         }
      }
   }
   else
   {
      rval = -1;
      errno = EINTR;
   }
   
   if(t->isInterrupted())
   {
      // set interrupted exception
      setException(new InterruptedException(
         "Thread '" + t->getName() + "' interrupted"));
   }
   
   return rval;
}

void Thread::setException(Exception* e)
{
   if(currentThread() != NULL)
   {
      // get the existing exception for the current thread, if any
      Exception* existing = getException();
      if(existing != e)
      {
         // replace the existing exception
         pthread_setspecific(EXCEPTION_KEY, e);
         
         // delete the old exception
         delete existing;
      }
   }
   else if(e != NULL)
   {
      // delete passed exception, since we are not on a thread
      delete e;
   }
}

Exception* Thread::getException()
{
   Exception* rval = NULL;
   
   if(currentThread() != NULL)
   {
      // get the exception for the current thread, if any
      rval = (Exception*)pthread_getspecific(EXCEPTION_KEY);
   }
   
   return rval;
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
   
   Thread* t = currentThread();
   if(t != NULL)
   {
      // set the current thread's wait monitor
      t->mWaitMonitor = m;
      
      // get the current time and determine if wait should be indefinite
      unsigned long long time = System::getCurrentMilliseconds();
      bool indefinite = (timeout == 0);
      
      // wait while not interrupted, must wait, and timeout not exhausted
      while(!t->isInterrupted() && m->mustWait() && (indefinite || timeout > 0))
      {
         m->wait(timeout);
         time = (System::getCurrentMilliseconds() - time);
         timeout -= (time > timeout) ? timeout : time;
      }
      
      // clear the current thread's wait monitor
      t->mWaitMonitor = NULL;
      
      // create interrupted exception if interrupted
      if(t->isInterrupted())
      {
         rval = new InterruptedException(
            "Thread '" + t->getName() + "' interrupted");
      }
   }
   
   // set exception
   setException(rval);
   
   return rval;
}
void Thread::setSignalMask(const sigset_t* newmask, sigset_t* oldmask)
{
   // set signal mask for this thread
   pthread_sigmask(SIG_SETMASK, newmask, oldmask);
}

void Thread::blockSignal(int signum)
{
   // unblock signal on this thread
   sigset_t newset;
   sigemptyset(&newset);
   sigaddset(&newset, signum);
   pthread_sigmask(SIG_BLOCK, &newset, NULL);
}

void Thread::unblockSignal(int signum)
{
   // unblock signal on this thread
   sigset_t newset;
   sigemptyset(&newset);
   sigaddset(&newset, signum);
   pthread_sigmask(SIG_UNBLOCK, &newset, NULL);
}

void Thread::setSignalHandler(
   int signum, const struct sigaction* newaction, struct sigaction* oldaction)
{
   // set signal handler
   sigaction(signum, newaction, oldaction);
}
