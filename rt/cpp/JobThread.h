/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef JobThread_H
#define JobThread_H

#include "Thread.h"
#include "IllegalArgumentException.h"

namespace db
{
namespace rt
{

/**
 * A JobThread is a thread that runs Runnable jobs and sleeps while it is
 * not running a job. This thread can be used in conjunction with a
 * JobThreadPool.
 * 
 * @author Dave Longley
 */
class JobThread : public Thread
{
protected:
   /**
    * The Runnable job to run.
    */
   Runnable* mJob;
   
   /**
    * The amount of idle time (in milliseconds) that must pass before this
    * JobThread automatically expires.
    */
   unsigned long long mExpireTime;
   
   /**
    * Gets the Runnable job to run, if any.
    * 
    * @return the Runnable job to run, if any.
    */
   virtual Runnable* getJob();
   
   /**
    * Makes this thread idle.
    */
   virtual void goIdle();
   
   /**
    * Wakes up this thread.
    */
   virtual void wakeup();
   
public:
   /**
    * Creates a new JobThread that expires if it sits idle (never processes
    * a single job) for the passed time interval. If an expire time of 0
    * is passed then this JobThread will never expire.
    * 
    * @param expireTime the amount of time (in milliseconds) that must pass
    *                   while the JobThread is idle in order for it to expire
    *                   -- if 0 is passed then the JobThread will never expire.
    */
   JobThread(unsigned long long expireTime = 0);
   
   /**
    * Destructs this JobThread.
    */
   virtual ~JobThread();
   
   /**
    * Sets the Runnable job for this thread. If null is passed then this
    * thread will be considered idle (with no job).
    * 
    * @param job the job for this thread or null if this thread has no job.
    */
   virtual void setJob(Runnable* job);
   
   /**
    * Runs this thread.
    */
   virtual void run();
   
   /**
    * Returns true if this thread has a job, false if it is idle.
    * 
    * @return true if this thread has a job, false if it is idle.
    */
   virtual bool hasJob();
   
   /**
    * Returns true if this thread has no job, false if it does.
    * 
    * @return true if this thread has no job, false if it does.
    */
   virtual bool isIdle();
   
   /**
    * Sets the expire time for this job thread.
    * 
    * @param expireTime the amount of time that must pass while this JobThread
    *                   is idle in order for it to expire -- if 0 is passed
    *                   then this JobThread will never expire.
    */
   virtual void setExpireTime(unsigned long long expireTime)
   throw(IllegalArgumentException);
   
   /**
    * Gets the expire time for this job thread.
    * 
    * @return the expire time for this job thread.
    */
   virtual unsigned long long getExpireTime();
};

} // end namespace rt
} // end namespace db
#endif
