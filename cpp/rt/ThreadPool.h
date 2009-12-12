/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_ThreadPool_H
#define monarch_rt_ThreadPool_H

#include "monarch/rt/Semaphore.h"
#include "monarch/rt/PooledThread.h"

#include <list>

namespace monarch
{
namespace rt
{

/**
 * A ThreadPool maintains a set of N PooledThreads that can be used to run jobs
 * without having to tear down the threads and create new ones.
 *
 * @author Dave Longley
 */
class ThreadPool
{
protected:
   /**
    * This semaphore is used to regulate the number of threads that
    * are available in this pool.
    */
   Semaphore mThreadSemaphore;

   /**
    * The list of all threads in this pool.
    */
   typedef std::list<PooledThread*> ThreadList;
   ThreadList mThreads;

   /**
    * The list of idle threads in this pool.
    */
   ThreadList mIdleThreads;

   /**
    * The list of expired threads in this pool.
    */
   ThreadList mExpiredThreads;

   /**
    * A lock for modifying the unexpired thread lists.
    */
   ExclusiveLock mListLock;

   /**
    * A lock for running new jobs or terminating existing ones.
    */
   ExclusiveLock mJobLock;

   /**
    * The stack size for threads (in bytes).
    */
   size_t mThreadStackSize;

   /**
    * The expire time for threads in milliseconds).
    */
   uint32_t mThreadExpireTime;

   /**
    * Gets an idle thread. This method will also clean up any extra
    * idle threads that should not exist due to a decrease in the
    * pool size. If an idle thread is not found, it will be created.
    *
    * A thread permit should always be acquired before calling this
    * method -- as to do otherwise will cause the number of threads to
    * become out of sync with the thread semaphore permit count.
    *
    * @return an idle thread.
    */
   virtual PooledThread* getIdleThread();

   /**
    * Removes up to the passed number of idle threads.
    *
    * @param count the maximum number of idle threads to remove.
    */
   virtual void removeIdleThreads(unsigned int count);

   /**
    * Cleans up any expired threads.
    */
   virtual void cleanupExpiredThreads();

   /**
    * Runs the passed Runnable job on an idle thread.
    *
    * @param job the Runnable job to run.
    * @param block true to wait until the job can be started, false to
    *              quit if it can't be started immediately.
    *
    * @return true if the job was started, false if insufficient resources
    *         prevented it from being started and it should be tried again.
    */
   virtual bool runJobOnIdleThread(Runnable& job, bool block);
   virtual bool runJobOnIdleThread(RunnableRef& job, bool block);

public:
   /**
    * Creates a new ThreadPool with the specified number of threads
    * available for running jobs.
    *
    * @param poolSize the size of the pool (number of threads).
    * @param stackSize the minimum size for each thread's stack, in bytes, 0
    *                  for the system default.
    */
   ThreadPool(unsigned int poolSize = 10, size_t stackSize = 0);

   /**
    * Destructs this ThreadPool.
    */
   virtual ~ThreadPool();

   /**
    * Tries to run the passed Runnable job on an available thread. If a
    * thread is available, this method will return true. If not, this method
    * will not block, but will instead return false.
    *
    * @param job the Runnable job to run.
    *
    * @return true if the job could run, false if not.
    */
   virtual bool tryRunJob(Runnable& job);
   virtual bool tryRunJob(RunnableRef& job);

   /**
    * Runs the passed Runnable job on an available thread.
    *
    * This method will lock until an available thread is acquired or
    * the current thread is interrupted.
    *
    * @param job the Runnable job to run.
    */
   virtual void runJob(Runnable& job);
   virtual void runJob(RunnableRef& job);

   /**
    * Called by a thread when it completes its job.
    *
    * @param t the thread that completed its job.
    */
   virtual void jobCompleted(PooledThread* t);

   /**
    * Interrupts all threads in this pool.
    */
   virtual void interruptAllThreads();

   /**
    * Interrupts all threads in this pool, joins, and removes them.
    */
   virtual void terminateAllThreads();

   /**
    * Sets the number of threads in this thread pool.
    *
    * @param size the number of threads in this thread pool.
    */
   virtual void setPoolSize(unsigned int size);

   /**
    * Gets the number of threads in this thread pool.
    *
    * @return the number of threads in this thread pool.
    */
   virtual unsigned int getPoolSize();

   /**
    * Sets the stack size for all new threads.
    *
    * @param stackSize the stack size for new threads, in bytes.
    */
   virtual void setThreadStackSize(size_t stackSize);

   /**
    * Gets the stack size for all new threads.
    *
    * @return the stack size for all new threads.
    */
   virtual size_t getThreadStackSize();

   /**
    * Sets the expire time for all threads.
    *
    * @param expireTime the amount of time that must pass while threads
    *                   are idle in order for them to expire -- if 0 is passed
    *                   then threads will never expire.
    */
   virtual void setThreadExpireTime(uint32_t expireTime);

   /**
    * Gets the expire time for all threads.
    *
    * @return the expire time for all threads.
    */
   virtual uint32_t getThreadExpireTime();

   /**
    * Gets the current number of threads in the pool.
    *
    * @return the current number of threads in the pool.
    */
   virtual unsigned int getThreadCount();

   /**
    * Gets the current number of running threads.
    *
    * Returns getThreadCount() - getIdleThreadCount().
    *
    * @return the current number of running threads.
    */
   virtual unsigned int getRunningThreadCount();

   /**
    * Gets the current number of idle threads.
    *
    * @return the current number of idle threads.
    */
   virtual unsigned int getIdleThreadCount();
};

} // end namespace rt
} // end namespace monarch
#endif
