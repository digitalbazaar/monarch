/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_PooledThread_H
#define db_rt_PooledThread_H

#include "db/rt/Thread.h"
#include "db/rt/Semaphore.h"

namespace db
{
namespace rt
{

// forward declare ThreadPool
class ThreadPool;

/**
 * A PooledThread is a thread that is a member of ThreadPool. It runs Runnable
 * jobs and goes idle while it is not running a job.
 * 
 * @author Dave Longley
 */
class PooledThread : public Thread
{
protected:
   /**
    * The Runnable job to run.
    */
   Runnable* mJob;
   RunnableRef mJobReference;
   
   /**
    * The ThreadPool to notify when a job completes.
    */
   ThreadPool* mThreadPool;
   
   /**
    * A lock for changing this thread's job/idle status.
    */
   Object mJobLock;
   
   /**
    * The amount of idle time (in milliseconds) that must pass before this
    * thread automatically expires.
    */
   unsigned long long mExpireTime;
   
   /**
    * True if this thread has expired, false if not.
    */
   bool mExpired;
   
   /**
    * Makes this thread idle.
    */
   virtual void goIdle();
   
public:
   /**
    * Creates a new PooledThread that expires if it sits idle (never processes
    * a single job) for the passed time interval. If an expire time of 0
    * is passed then this thread will never expire.
    * 
    * @param pool the ThreadPool this PooledThread is a member of.
    * @param expireTime the amount of time (in milliseconds) that must pass
    *                   while this thread is idle in order for it to expire --
    *                   if 0 is passed then the thread will never expire.
    */
   PooledThread(ThreadPool* pool, unsigned long long expireTime = 0);
   
   /**
    * Destructs this PooledThread.
    */
   virtual ~PooledThread();
   
   /**
    * Sets the Runnable job for this thread. If null is passed then this
    * thread will be considered idle (with no job).
    * 
    * @param job the job for this thread or null if this thread has no job.
    */
   virtual void setJob(Runnable* job);
   virtual void setJob(RunnableRef& job);
   
   /**
    * Gets the Runnable job for this thread.
    */
   virtual Runnable* getJob();
   
   /**
    * Gets the lock for changing this thread's job/idle status.
    * 
    * @return the lock for changing this thread's job/idle status.
    */
   virtual Object* getJobLock();
   
   /**
    * Runs this thread.
    */
   virtual void run();
   
   /**
    * Sets the expire time for this thread.
    * 
    * @param expireTime the amount of time that must pass while this thread
    *                   is idle in order for it to expire -- if 0 is passed
    *                   then this thread will never expire.
    */
   virtual void setExpireTime(unsigned long long expireTime);
   
   /**
    * Gets the expire time for this thread.
    * 
    * @return the expire time for this thread.
    */
   virtual unsigned long long getExpireTime();
   
   /**
    * Returns true if this thread is expired, false if not.
    * 
    * @return true if this thread is expired, false if not.
    */
   virtual bool isExpired();
};

} // end namespace rt
} // end namespace db
#endif
