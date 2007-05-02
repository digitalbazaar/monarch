/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Thread_H
#define Thread_H

#include <pthread.h>
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
class Thread : public virtual Object, public Runnable
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
    * Stores whether or not this Thread has been interrupted.
    */
   bool mInterrupted;
   
   /**
    * Stores whether or not this Thread has been started.
    */
   bool mStarted;
   
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
    * run() method will be executed.
    *
    * @return true if the Thread started successfully, false if not.
    */
   virtual bool start();
   
   /**
    * Causes the currently executing Thread to wait until this Thread
    * completes or until the specified time expires.
    *
    * @param time the number of milliseconds to wait for this Thread to
    *             complete (0 to wait indefinitely).
    */
   virtual void join(unsigned long time = 0);
   
   /**
    * Detaches this Thread. This means that this Thread can no longer be
    * waited for via a join() call. This allows for the thread to be
    * automatically cleaned up by the OS once it terminates. Otherwise
    * a join() call must be made to appropriately clean up the thread.
    */
   virtual void detach();
   
   /**
    * Returns true if this Thread is still alive, false if not.
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
    * Runs the Runnable associated with this Thread.
    */
   virtual void run();
   
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
    * Returns true if the currently executing Thread has been interrupted,
    * false if not. The interrupted status of the currently executing Thread
    * is cleared when this method is called. This means that if this method
    * is called twice in immediate succession, the second call will always
    * return false.
    *
    * @return true if this Thread has been interrupted, false if not.
    */
   static bool interrupted();
   
   /**
    * Causes the currently executing Thread to sleep for the specified
    * number of milliseconds.
    *
    * @param time the number of milliseconds to sleep for.
    * 
    * @exception InterruptedException thrown if this Thread is interrupted
    *            while sleeping.
    */
   static void sleep(unsigned long time) throw(InterruptedException);
};

} // end namespace rt
} // end namespace db
#endif
