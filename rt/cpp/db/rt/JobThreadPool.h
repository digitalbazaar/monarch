/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_JobThreadPool_H
#define db_rt_JobThreadPool_H

#include "db/rt/Semaphore.h"
#include "db/rt/JobThread.h"

#include <list>
#include <vector>

namespace db
{
namespace rt
{

/**
 * This class is a JobThread pool.
 * 
 * This pool maintains a set of N JobThreads that can be used to run jobs
 * without having to tear down the threads and create new ones.
 * 
 * @author Dave Longley
 */
class JobThreadPool : public virtual Object
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
   typedef std::list<JobThread*> ThreadList;
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
    * A lock for modifying the thread lists.
    */
   Object mListLock;
   
   /**
    * The expire time for JobThreads (in milliseconds).
    */
   unsigned long long mJobThreadExpireTime;
   
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
   virtual JobThread* getIdleThread();
   
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
    * Runs the passed Runnable job on an idle JobThread.
    * 
    * @param job the Runnable job to run.
    */
   virtual void runJobOnIdleThread(Runnable& job);
   virtual void runJobOnIdleThread(CollectableRunnable& job);
   
public:
   /**
    * Creates a new JobThreadPool with the specified number of threads
    * available for running jobs.
    * 
    * @param poolSize the size of the pool (number of threads), 0 specifies
    *                 an unlimited number of threads.
    */
   JobThreadPool(unsigned int poolSize = 10);
   
   /**
    * Destructs this JobThreadPool.
    */
   virtual ~JobThreadPool();
   
   /**
    * Tries to run the passed Runnable job on an available JobThread. If a
    * thread is available, this method will return true. If not, this method
    * will not block, but will instead return false.
    * 
    * @param job the Runnable job to run.
    * 
    * @return true if the job could run, false if not.
    */
   virtual bool tryRunJob(Runnable& job);
   virtual bool tryRunJob(CollectableRunnable& job);
   
   /**
    * Runs the passed Runnable job on an available JobThread.
    * 
    * This method will lock until an available thread is acquired or
    * the current thread is interrupted.
    * 
    * @param job the Runnable job to run.
    */
   virtual void runJob(Runnable& job);
   virtual void runJob(CollectableRunnable& job);
   
   /**
    * Called by a JobThread when it completes its job.
    * 
    * @param t the JobThread that completed its job.
    */
   virtual void jobCompleted(JobThread* t);
   
   /**
    * Interrupts all threads in this pool.
    */
   virtual void interruptAllThreads();
   
   /**
    * Interrupts all threads in this pool, joins, and removes them.
    */
   virtual void terminateAllThreads();
   
   /**
    * Sets the number of threads in this thread pool. If a size of
    * 0 is specified, than there will be no limit to the number of
    * threads in this pool.
    * 
    * @param size the number of threads in this thread pool. A size
    *             of 0 specifies an unlimited number of threads.
    */
   virtual void setPoolSize(unsigned int size);
   
   /**
    * Gets the number of threads in this thread pool. If a size of
    * 0 is returned, than there is no limit to the number of threads
    * in this pool.
    * 
    * @return the number of threads in this thread pool. A size
    *         of 0 specifies an unlimited number of threads.
    */
   virtual unsigned int getPoolSize();
   
   /**
    * Sets the expire time for all JobThreads.
    * 
    * @param expireTime the amount of time that must pass while JobThreads
    *                   are idle in order for them to expire -- if 0 is passed
    *                   then JobThreads will never expire.
    */
   virtual void setJobThreadExpireTime(unsigned long long expireTime);
   
   /**
    * Gets the expire time for all JobThreads.
    * 
    * @return the expire time for all JobThreads.
    */
   virtual unsigned long long getJobThreadExpireTime();
   
   /**
    * Gets the current number of JobThreads in the pool.
    * 
    * @return the current number of JobThreads in the pool.
    */
   virtual unsigned int getJobThreadCount();
   
   /**
    * Gets the current number of running JobThreads.
    * 
    * Returns getJobThreadCount() - getIdleJobThreadCount().
    * 
    * @return the current number of running JobThreads.
    */
   virtual unsigned int getRunningJobThreadCount();
   
   /**
    * Gets the current number of idle JobThreads.
    * 
    * @return the current number of idle JobThreads.
    */
   virtual unsigned int getIdleJobThreadCount();
};

} // end namespace rt
} // end namespace db
#endif
