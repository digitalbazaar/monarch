/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_Thread_H
#define db_rt_Thread_H

#include <pthread.h>
#include <sys/types.h>
#include <sched.h>
#include <signal.h>
#include <string>

#include "Object.h"
#include "InterruptedException.h"
#include "Runnable.h"

namespace db
{
namespace rt
{

/**
 * A Thread executes the run() method of a Runnable object on its own thread
 * of execution.
 *
 * This class wraps a POSIX thread (pthread) to provide an object-oriented
 * foundation for multithreaded applications.
 *
 * @author Dave Longley
 */
class Thread : public virtual Object, protected Runnable
{
protected:
   /**
    * The POSIX thread wrapped by this class.
    */
   pthread_t mPThread;
   
   /**
    * The attributes for the POSIX thread.
    */
   pthread_attr_t mPThreadAttributes;
   
   /**
    * The Monitor this Thread is waiting to enter.
    */
   Monitor* mWaitMonitor;
   
   /**
    * The Runnable associated with this Thread.
    */
   Runnable* mRunnable;
   
   /**
    * The name for this thread.
    */
   std::string mName;
   
   /**
    * Stores whether or not this Thread is alive.
    */
   bool mAlive;
   
   /**
    * Stores whether or not this Thread has been detached.
    */
   bool mDetached;
   
   /**
    * Stores whether or not this Thread has been interrupted.
    */
   bool mInterrupted;
   
   /**
    * Stores whether or not this Thread has joined.
    */
   bool mJoined;
   
   /**
    * Stores whether or not this Thread has been started.
    */
   bool mStarted;
   
   /**
    * This method is called when a new POSIX thread is created (on that
    * POSIX thread). It runs the Runnable associated with this Thread.
    */
   virtual void run();
   
   /**
    * Used to ensure that the current thread key is initialized only once.
    */
   static pthread_once_t CURRENT_THREAD_KEY_INIT;
   
   /**
    * A thread key for obtaining the current thread.
    */
   static pthread_key_t CURRENT_THREAD_KEY;
   
   /**
    * Creates the current thread key.
    */
   static void createCurrentThreadKey();
   
   /**
    * Used to ensure that the exception key is initialized only once.
    */
   static pthread_once_t EXCEPTION_KEY_INIT;
   
   /**
    * A thread key for obtaining the last thread-local exception.
    */
   static pthread_key_t EXCEPTION_KEY;
   
// Note: disabled due to a lack of support in windows
//   /**
//    * Used to ensure that the SIGINT handler is initialized only once.
//    */
//   static pthread_once_t SIGINT_HANDLER_INIT;
   
   /**
    * Creates the exception key.
    */
   static void createExceptionKey();
   
// Note: disabled due to a lack of support in windows
//   /**
//    * Installs the SIGINT handler that can interrupt the current thread.
//    */
//   static void installSigIntHandler();
//   
//   /**
//    * The SIGINT handler that ensures threads get interrupted.
//    * 
//    * @param signum the signal to handle.
//    */
//   static void handleSigInt(int signum);
   
   /**
    * The method used to execute the POSIX thread. The passed Thread object
    * will be deleted once execution completes.
    *
    * @param thread the Thread that is executing (to be cast to a Thread*).
    */
   static void* execute(void* thread);
   
public:
   /**
    * Creates a new Thread that uses the given Runnable.
    *
    * @param runnable the Runnable to use.
    * @param name a name for the Thread.
    */
   Thread(Runnable* runnable, std::string name = "");
   
   /**
    * Destructs this Thread and deletes its Runnable.
    */
   virtual ~Thread();
   
   /**
    * Starts this Thread. A POSIX thread will be created and this Thread's
    * run() method will be executed. This thread can only be started once.
    *
    * @return true if the Thread started successfully, false if not.
    */
   virtual bool start();

// Note: disabled due to a lack of support in windows   
//   /**
//    * Sends a signal to this Thread.
//    * 
//    * @param signum the signal to send.
//    */
//   virtual void sendSignal(int signum);
   
   /**
    * Causes the currently executing Thread to wait until this Thread
    * completes.
    */
   virtual void join();
   
   /**
    * Detaches this Thread. This means that this Thread can no longer be
    * waited for via a join() call. This allows for the thread to be
    * automatically cleaned up by the OS once it terminates. Otherwise
    * a join() call must be made to appropriately clean up the thread.
    */
   virtual void detach();
   
   /**
    * Returns true if this Thread is still alive, false if not. A Thread
    * is alive if it has been started and is running.
    * 
    * @return true if this Thread is still alive, false if not.
    */
   virtual bool isAlive();
   
   /**
    * Interrupts this Thread.
    */
   virtual void interrupt();
   
   /**
    * Returns true if this Thread has been interrupted, false if not. The
    * interrupted status of this Thread is not affected by this method.
    *
    * @return true if this Thread has been interrupted, false if not.
    */
   virtual bool isInterrupted();
   
   /**
    * Returns true if this Thread has been started, false if not.
    *
    * @return true if this Thread has been started, false if not.
    */
   virtual bool hasStarted();
   
   /**
    * Sets the name of this Thread.
    *
    * @param name the name for this Thread.
    */
   virtual void setName(std::string name);
   
   /**
    * Gets the name of this Thread.
    *
    * @return the name of this Thread.
    */
   virtual const std::string& getName();
   
   /**
    * Returns the currently executing Thread.
    *
    * @return the currently executing Thread or NULL if the main process
    *         is executing.
    */
   static Thread* currentThread();
   
   /**
    * Returns true if the current thread has been interrupted, false if not.
    * The interrupted status of the currently executing Thread is cleared when
    * this method is called with the flag set to clear (the default). This
    * means that if this method is called twice in immediate succession, the
    * second call will always return false.
    *
    * @param clear true to clear the interrupted status of the current thread.
    * 
    * @return true if this Thread has been interrupted, false if not.
    */
   static bool interrupted(bool clear = true);
   
   /**
    * Causes the current thread to sleep for the specified number of
    * milliseconds.
    *
    * @param time the number of milliseconds to sleep for.
    * 
    * @return an InterruptedException if this Thread is interrupted while
    *         sleeping, NULL if not.
    */
   static InterruptedException* sleep(unsigned long time);
   
   /**
    * Causes the current thread to yield for a moment. Yielding causes the
    * current thread to relinquish use of its processor (gives up its time
    * slice) and places the thread in a wait queue. Once the other threads
    * in the queue have taken a turn using the processor the thread gets
    * rescheduled for execution.
    */
   static void yield();
   
   /**
    * Causes the current thread to wait until one or more of the given
    * file descriptors become ready for some kind of IO operation or until
    * the given timeout is reached. This method allows this thread to be
    * interrupted during the select().
    * 
    * Note: errno can be set as such:
    * 
    * EBADF  An invalid file descriptor was in the set (i.e. already closed).
    * EINTR  A signal was caught.
    * EINVAL nfds is negative or the value in the timeout is invalid.
    * ENOMEM unable to allocate memory.
    * 
    * @param nfds the highest-numbered file descriptor in any of the given
    *             three sets, plus 1.
    * @param readfds a set of file descriptors to monitor for readability.
    * @param writefds a set of file descriptors to monitor for writeability.
    * @param exceptfds a set of file descriptors to monitor for exceptions.
    * @param timeout the timeout to use (0 for indefinite wait, -1 for polling).
    * @param sigmask a signal mask to set atomically so that whatever signals
    *                are not masked will be received inside of the select()
    *                call.
    * 
    * @return the number of file descriptors contained in the three returned
    *         descriptor sets which may be 0, -1 if an error occurred and
    *         errno is set appropriately. 
    */
   static int select(
      int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
      long long timeout, const sigset_t* sigmask = NULL);
   
   /**
    * Causes the current thread to wait to enter the given Monitor until
    * that Monitor's wait condition has been satisfied.
    * 
    * @param m the Monitor to wait to enter.
    * @param timeout the number of milliseconds to wait before timing out, 
    *                0 to wait indefinitely.
    * 
    * @return an InterruptedException if this Thread is interrupted while
    *         sleeping, NULL if not.
    */
   static InterruptedException* waitToEnter(
      Monitor* m, unsigned long timeout = 0);
   
   /**
    * Sets the Exception for the current thread. This will store the passed
    * exception in thread-local memory and delete it when the current
    * thread exits or when it is replaced by another call to setException()
    * on the same thread.
    * 
    * If no current Thread object can be found, then the passed Exception
    * will be deleted immediately.
    * 
    * It is safe to call Thread::setException(Thread::getException()), no
    * memory will be mistakenly collected. 
    * 
    * @param e the Exception to set for the current thread.
    */
   static void setException(Exception* e);
   
   /**
    * Gets the Exception for the current thread. This will be the last
    * Exception that was set on this thread. It is stored in thread-local
    * memory and automatically cleaned up when the thread exits.
    * 
    * @return the last Exception for the current thread, which may be NULL.
    */
   static Exception* getException();
   
   /**
    * Returns true if the current thread has encountered an Exception that
    * can be retrieved by calling Thread::getException(), false if not.
    * 
    * @return true if the current thread an Exception, false if not.
    */
   static bool hasException();
   
   /**
    * Clears any Exception from the current thread.
    */
   static void clearException();

// Note: disabled due to a lack of support in windows
//   /**
//    * Sets the signal mask for the current thread.
//    * 
//    * @param newmask the new set of signals to mask for this thread.
//    * @param oldmask to store the old signal mask.
//    */
//   static void setSignalMask(const sigset_t* newmask, sigset_t* oldmask);
//   
//   /**
//    * Blocks a single signal for the current thread.
//    * 
//    * @param signum the signal to block.
//    */
//   static void blockSignal(int signum);
//   
//   /**
//    * Unblocks a single signal for the current thread.
//    * 
//    * @param signum the signal to unblock.
//    */
//   static void unblockSignal(int signum);
//   
//   /**
//    * Sets a signal handler for the current thread.
//    * 
//    * @param signum the signal to handle.
//    * @param newaction the signal handler to use.
//    * @param oldaction to store the old signal handler.
//    */
//   static void setSignalHandler(
//      int signum, const struct sigaction* newaction,
//      struct sigaction* oldaction);
};

} // end namespace rt
} // end namespace db
#endif
