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
bool Thread::CURRENT_THREAD_KEY_INITIALIZED = false;
pthread_key_t Thread::CURRENT_THREAD_KEY;

// initialize exception key parameters
pthread_once_t Thread::EXCEPTION_KEY_INIT = PTHREAD_ONCE_INIT;
bool Thread::EXCEPTION_KEY_INITIALIZED = false;
pthread_key_t Thread::EXCEPTION_KEY;

// Note: disabled due to a lack of support in windows
// initialize signal handler parameters
//pthread_once_t Thread::SIGINT_HANDLER_INIT = PTHREAD_ONCE_INIT;

Thread::Thread(Runnable* runnable, const char* name)
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
   mName = NULL;
   assignName(name);
   
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
   
   if(this == &MAIN_THREAD)
   {
      // ensure keys are initialized, clear any exceptions
      currentThread();
      setException(NULL);
      
      // delete thread keys
      pthread_setspecific(CURRENT_THREAD_KEY, NULL);
      pthread_setspecific(EXCEPTION_KEY, NULL);
      pthread_key_delete(CURRENT_THREAD_KEY);
      pthread_key_delete(EXCEPTION_KEY);
   }
   
   // delete name
   if(mName != NULL)
   {
      delete [] mName;
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

InterruptedException* Thread::createInterruptedException()
{
   InterruptedException* rval = NULL;
   
   char* msg = new char[8 + strlen(getName()) + 13 + 1];
   sprintf(msg, "Thread '%s' interrupted", getName());
   
   rval = new InterruptedException(msg);
   delete msg;
   
   return rval;
}

void Thread::createCurrentThreadKey()
{
   // create the thread key for obtaining the current thread
   pthread_key_create(&CURRENT_THREAD_KEY, &cleanupCurrentThreadKeyValue);
   pthread_setspecific(CURRENT_THREAD_KEY, NULL);
}

void Thread::cleanupCurrentThreadKeyValue(void* thread)
{
   // no action is necessary, key is automatically set to NULL
}

void Thread::createExceptionKey()
{
   // create the thread key for obtaining the last thread-local exception
   pthread_key_create(&EXCEPTION_KEY, &cleanupExceptionKeyValue);
   pthread_setspecific(EXCEPTION_KEY, NULL);
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
   
   if(!CURRENT_THREAD_KEY_INITIALIZED)
   {
      // create the current thread key
      pthread_once(&CURRENT_THREAD_KEY_INIT, Thread::createCurrentThreadKey);
      CURRENT_THREAD_KEY_INITIALIZED = true;
   }
   
   // set thread specific data for current thread to the Thread
   pthread_setspecific(CURRENT_THREAD_KEY, t);
   
   // Note: disabled due to a lack of support in windows
   // install signal handler
   //pthread_once(&SIGINT_HANDLER_INIT, Thread::installSigIntHandler);
   
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

// Note: disabled due to lack of support in windows
//void Thread::sendSignal(int signum)
//{
//   lock();
//   {
//      if(hasStarted() && isAlive())
//      {
//         pthread_kill(mPThread, signum);
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

Thread* Thread::currentThread()
{
   if(!CURRENT_THREAD_KEY_INITIALIZED)
   {
      // create the current thread key
      pthread_once(&CURRENT_THREAD_KEY_INIT, Thread::createCurrentThreadKey);
      CURRENT_THREAD_KEY_INITIALIZED = true;
   }
   
   // get a pointer to the current thread
   Thread* t = (Thread*)pthread_getspecific(CURRENT_THREAD_KEY);
   if(t == NULL)
   {
      // on the main thread, so initialize main thread and set specific data
      t = &Thread::MAIN_THREAD;
      t->mPThread = pthread_self();
      t->mAlive = true;
      t->mDetached = true;
      t->mStarted = true;
      pthread_setspecific(CURRENT_THREAD_KEY, t);
   }
   
   return t;
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

int Thread::select(bool read, unsigned int fd, long long timeout)
{
   int rval = 0;
   
   // create a file descriptor set to select on
   fd_set fds;
   FD_ZERO(&fds);
   
   // add file descriptor to set
   FD_SET(fd, &fds);
   
   // "n" parameter is the highest numbered descriptor plus 1
   int n = fd + 1;
   
   // keep selecting (polling) until timeout is reached
   long long remaining = (timeout <= 0) ? 1 : timeout;
   
   struct timeval to;
   if(timeout < 0)
   {
      // create instant timeout (polling)
      to.tv_sec = 0;
      to.tv_usec = 0;
   }
   else
   {
      // create 10 millisecond timeout (1 millisecond is 1000 microseconds)
      to.tv_sec = 0;
      to.tv_usec = (10000LL < remaining ? remaining : 10000LL);
   }
   
   unsigned long long start = System::getCurrentMilliseconds();
   unsigned long long end;
   
   Thread* t = Thread::currentThread();
   while(!t->isInterrupted() && remaining > 0 && rval == 0)
   {
      // wait for file descriptors to be updated
      if(read)
      {
         // wait for readability
         rval = ::select(n, &fds, NULL, &fds, &to);
      }
      else
      {
         // wait for writability
         rval = ::select(n, NULL, &fds, &fds, &to);
      }
      
      if(rval < 0)
      {
         if(errno == EINTR)
         {
            // interrupt thread
            t->interrupt();
         }
         else if(errno == 0)
         {
            // no error, just timed out
            rval = 0;
         }
      }
      
      // select() implementation may alter sets or timeout, so reset them
      // if calling select() again
      if(remaining > 0 && rval == 0 && timeout >= 0)
      {
         // clear set and re-add file descriptor
         FD_ZERO(&fds);
         FD_SET(fd, &fds);
         
         // reset timeout
         to.tv_sec = 0;
         to.tv_usec = 1000LL;
      }
      
      if(timeout != 0)
      {
         // decrement remaining time
         end = System::getCurrentMilliseconds();
         remaining -= (end - start);
         start = end;
         to.tv_usec = (10000LL < remaining ? remaining : 10000LL);
      }
   }
   
   if(t->isInterrupted())
   {
      rval = -1;
      errno = EINTR;
      
      // set interrupted exception
      setException(t->createInterruptedException());
   }
   
   return rval;
}

int Thread::select(
   int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
   long long timeout, const sigset_t* sigmask)
{
   int rval = 0;
   
// Note: disabled due to a lack of support in windows
//   // create timeout
//   struct timeval* tv = NULL;
//   struct timeval to;
//   if(timeout > 0)
//   {
//      // set timeout (1 millisecond is 1000 microseconds) 
//      to.tv_sec = timeout / 1000LL;
//      to.tv_usec = (timeout % 1000LL) * 1000LL;
//      tv = &to;
//   }
//   
//   // FIXME: signals supposedly don't make select() return in windows
//   // this needs to be tested and potentially remedied somehow
//   
//   // FIXME: furthermore, even if we block SIGINT (interruption signal) up
//   // until we reach the select call -- and then unblock right before it
//   // the signal could still sneak in right before select() is called and
//   // control is transferred to the kernel, and therefore we'd handle the
//   // SIGINT before the select() call and select() wouldn't get interrupted
//   // (there is pselect() for doing that unblocking atomically, but
//   // it's UNIX only) -- this may be solved by writing to another file
//   // descriptor when we receive SIGINTs and checking that file descriptor
//   // as well as the one we are waiting on -- but this might not be a
//   // viable solution for windows
//   
//   // block SIGINTs
//   blockSignal(SIGINT);
//   
//   Thread* t = Thread::currentThread();
//   if(!t->isInterrupted())
//   {
//      // FIXME: pselect() required here to do atomic unblocking & selecting
//      
//      // wait for file descriptors to be updated
//      unblockSignal(SIGINT);
//      rval = ::select(nfds, readfds, writefds, exceptfds, timeout);
//      if(rval < 0)
//      {
//         if(errno == EINTR)
//         {
//            // interrupt thread
//            t->interrupt();
//         }
//      }
//   }
//   else
//   {
//      rval = -1;
//      errno = EINTR;
//   }
   
   // clone file descriptor sets
   fd_set readfds2;
   fd_set writefds2;
   fd_set exceptfds2;
   
   if(readfds != NULL)
   {
      readfds2 = *readfds;
   }
   
   if(writefds != NULL)
   {
      writefds2 = *writefds;
   }
   
   if(exceptfds != NULL)
   {
      exceptfds2 = *exceptfds;
   }
   
   // keep selecting (polling) until timeout is reached
   long long remaining = (timeout <= 0) ? 1 : timeout;
   
   struct timeval to;
   if(timeout < 0)
   {
      // create instant timeout (polling)
      to.tv_sec = 0;
      to.tv_usec = 0;
   }
   else
   {
      // create 1 millisecond timeout (1 millisecond is 1000 microseconds)
      to.tv_sec = 0;
      to.tv_usec = 1000LL;
   }
   
   unsigned long long start = System::getCurrentMilliseconds();
   unsigned long long end;
   
   Thread* t = Thread::currentThread();
   while(!t->isInterrupted() && remaining > 0 && rval == 0)
   {
      // wait for file descriptors to be updated
      rval = ::select(nfds, readfds, writefds, exceptfds, &to);
      if(rval < 0)
      {
         if(errno == EINTR)
         {
            // interrupt thread
            t->interrupt();
         }
         else if(errno == 0)
         {
            // no error, just timed out
            rval = 0;
         }
      }
      
      // select() implementation may alter sets or timeout, so reset them
      // if calling select() again
      if(remaining > 0 && rval == 0 && timeout >= 0)
      {
         // reset file descriptor sets
         if(readfds != NULL)
         {
            *readfds = readfds2;
         }
         
         if(writefds != NULL)
         {
            *writefds = writefds2;
         }
         
         if(exceptfds != NULL)
         {
            *exceptfds = exceptfds2;
         }
         
         // reset timeout
         to.tv_sec = 0;
         to.tv_usec = 1000LL;
      }
      
      if(timeout != 0)
      {
         // decrement remaining time
         end = System::getCurrentMilliseconds();
         remaining -= (end - start);
         start = end;
      }
   }
   
   if(t->isInterrupted())
   {
      rval = -1;
      errno = EINTR;
      
      // set interrupted exception
      setException(t->createInterruptedException());
   }
   
   return rval;
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
   Exception* rval = NULL;
   
   if(!EXCEPTION_KEY_INITIALIZED)
   {
      // create the exception key
      pthread_once(&EXCEPTION_KEY_INIT, Thread::createExceptionKey);
      EXCEPTION_KEY_INITIALIZED = true;
   }
   
   // get the exception for the current thread, if any
   rval = (Exception*)pthread_getspecific(EXCEPTION_KEY);
   
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
