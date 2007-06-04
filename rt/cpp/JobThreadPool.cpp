/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "JobThreadPool.h"

using namespace std;
using namespace db::rt;

JobThreadPool::JobThreadPool(unsigned int poolSize) :
   mThreadSemaphore(poolSize, true)
{
   // default JobThread expire time to 0 (no expiration)
   setJobThreadExpireTime(0);
   
   // set the pool size (number of threads)
   setPoolSize(poolSize);
}

JobThreadPool::~JobThreadPool()
{
   // terminate all threads
   terminateAllThreads(0);
}

void JobThreadPool::acquireThreadPermit() throw(InterruptedException)
{
   // If this pool allows an infinite number of threads, then
   // the number of permits will be zero -- since threads are
   // always permitted. Therefore, only try to acquire a
   // permit if there are more than 0 permits -- otherwise a
   // permit is automatically granted.
   if(mThreadSemaphore.getMaxPermitCount() != 0)
   {
      mThreadSemaphore.acquire();
   }
}

void JobThreadPool::releaseThreadPermit()
{
   mThreadSemaphore.release();
}

JobThread* JobThreadPool::createJobThread()
{
   // create job thrad
   return new JobThread(getJobThreadExpireTime());
}

// FIXME: remove iostream
#include <iostream>
JobThread* JobThreadPool::getIdleThread()
{
   JobThread* rval = NULL;
   
   // synchronize
   lock();
   {
      // get the number of extra threads
      int extraThreads = 0;
      if(mThreadSemaphore.getMaxPermitCount() > 0)
      {
         extraThreads = mThreads.size() - mThreadSemaphore.getMaxPermitCount();
      }
      
      // iterate through threads, find one that is idle
      for(vector<JobThread*>::iterator i = mThreads.begin();
          i != mThreads.end(); i++)
      {
         JobThread* thread = *i;
         if(thread->isIdle())
         {
            // if the thread is not alive, remove it and continue on
            if(!thread->isAlive())
            {
               cout << "removing expired thread." << endl;
               
               // remove thread
               mThreads.erase(i);
               delete thread;
               
               // decrement extra threads
               extraThreads--;
            }
            else
            {
               // if there are extra idle threads, interrupt and remove thread
               if(extraThreads > 0)
               {
                  cout << "interrupting extra thread." << endl;
                  
                  // interrupt thread
                  thread->interrupt();
                  
                  // detach thread
                  thread->detach();
                  
                  // remove thread
                  mThreads.erase(i);
                  delete thread;
                  
                  // decrement extra threads
                  extraThreads--;
               }
               else if(rval == NULL)
               {
                  // return this thread
                  rval = thread;
               }
            }
         }
      }
      
      // if no idle thread was found
      if(rval == NULL)
      {
         // create new job thread
         rval = createJobThread();
         
         cout << "adding new thread." << endl;
         
         // add thread to pool
         mThreads.push_back(rval);
         
         // start the thread
         rval->start();
      }
   }
   unlock();
   
   return rval;
}

/**
 * Runs the passed Runnable job on an idle JobThread.
 * 
 * @param job the Runnable job to run.
 */
void JobThreadPool::runJobOnIdleThread(Runnable* job)
{
   // synchronize
   lock();
   {
      // get an idle thread
      JobThread* thread = getIdleThread();
      
      // set job
      thread->setJob(job);
   }
   unlock();
}

/**
 * Sets the number of threads in this thread pool. If a size of
 * 0 is specified, than there will be no limit to the number of
 * threads in this pool.
 * 
 * @param size the number of threads in this thread pool. A size
 *             of 0 specifies an unlimited number of threads. Size
 *             must be a non-negative number.
 */
void JobThreadPool::setPoolSize(unsigned int size)
{
   // synchronize
   lock();
   {
      // Note: threads are created lazily so if the thread pool size
      // here is greater than the number of threads currently in
      // the pool, those threads will be created as they are needed
      // hence, we do not need to adjust for increases in the pool size
      // only for decreases
      
      // remove threads as necessary
      if(mThreads.size() > size && size != 0)
      {
         // get number of threads to remove
         int removeCount = mThreads.size() - size;
         
         // iterate through threads, remove idle threads
         for(vector<JobThread*>::iterator i = mThreads.begin();
             i != mThreads.end() && removeCount > 0; i++)
         {
            JobThread* thread = *i;
            
            // if thread is idle, terminate it and remove it
            if(thread->isIdle())
            {
               // interrupt thread
               thread->interrupt();
               
               // detach thread
               thread->detach();
               
               // remove it from the pool
               mThreads.erase(i);
               delete thread;
               
               // decrement remove count
               removeCount--;
            }
         }
      }
      
      // set semaphore permits
      mThreadSemaphore.setMaxPermitCount(size);
   }
   unlock();
}

/**
 * Gets the number of threads in this thread pool. If a size of
 * 0 is returned, than there is no limit to the number of threads
 * in this pool.
 * 
 * @return the number of threads in this thread pool. A size
 *         of 0 specifies an unlimited number of threads.
 */
unsigned int JobThreadPool::getPoolSize()
{
   return mThreadSemaphore.getMaxPermitCount();
}

/**
 * Runs the passed Runnable job on an available JobThread.
 * 
 * This method will lock until an available thread is acquired or
 * the current thread is interrupted.
 * 
 * @param job the Runnable job to run.
 */
void JobThreadPool::runJob(Runnable* job)
{
   bool permitAcquired = false;
   
   try
   {
      // acquire a thread permit
      acquireThreadPermit();
      
      // permit acquired
      permitAcquired = true;
      
      // run the job on an idle thread
      runJobOnIdleThread(job);
   }
   catch(InterruptedException& e)
   {
      // toggle interrupt thread to true
      Thread::currentThread()->interrupt();
      
      cout << "thread acquisition interrupted." << endl;
   }
   
   // if a permit was acquired, release it
   if(permitAcquired)
   {
      releaseThreadPermit();
   }
}

/**
 * Interrupts all threads in this pool.
 */
void JobThreadPool::interruptAllThreads()
{
   // synchronize
   lock();
   {
      cout << "interrupting all threads." << endl;
      
      // iterate through all threads, interrupt each
      for(vector<JobThread*>::iterator i = mThreads.begin();
          i != mThreads.end(); i++)
      {
         // interrupt thread
         JobThread* thread = *i;
         thread->interrupt();
      }
      
      cout << "all threads interrupted." << endl;      
   }
   unlock();
}

void JobThreadPool::terminateAllThreads(unsigned long long joinTime)
{
   // interrupt all the threads
   interruptAllThreads();
   
   cout << "terminating all threads." << endl;
   
   // synchronize
   lock();
   {
      try
      {
         // iterate through all threads, join and remove them
         for(vector<JobThread*>::iterator i = mThreads.begin();
             i != mThreads.end(); i++)
         {
            JobThread* thread = *i;
            
            // join thread
            thread->join(joinTime);
            
            // remove thread
            mThreads.erase(i);
            delete thread;
         }
      }
      catch(InterruptedException e)
      {
         // ensure current thread is still interrupted
         Thread::currentThread()->interrupt();
      }
      
      // clear threads
      mThreads.clear();
   }
   unlock();
   
   cout << "all threads terminated." << endl;
}

/**
 * Sets the expire time for all JobThreads.
 * 
 * @param expireTime the amount of time that must pass while JobThreads
 *                   are idle in order for them to expire -- if 0 is passed
 *                   then JobThreads will never expire.
 */
void JobThreadPool::setJobThreadExpireTime(unsigned long long expireTime)
{
   // synchronize
   lock();
   {
      mJobThreadExpireTime = expireTime;
      
      // update all existing job threads
      for(vector<JobThread*>::iterator i = mThreads.begin();
          i != mThreads.end(); i++)
      {
         JobThread* thread = *i;
         thread->setExpireTime(expireTime);
      }
   }
   unlock();
}

/**
 * Gets the expire time for all JobThreads.
 * 
 * @return the expire time for all JobThreads.
 */
unsigned long long JobThreadPool::getJobThreadExpireTime()
{
   return mJobThreadExpireTime;
}

/**
 * Gets the current number of JobThreads in the pool.
 * 
 * @return the current number of JobThreads in the pool.
 */
unsigned int JobThreadPool::getJobThreadCount()
{
   unsigned int rval = 0;
   
   // synchronize
   lock();
   {
      rval = mThreads.size();
   }
   unlock();
   
   return rval;
}

/**
 * Gets the current number of running JobThreads.
 * 
 * Returns getJobThreadCount() - getIdleJobThreadCount().
 * 
 * @return the current number of running JobThreads.
 */
unsigned int JobThreadPool::getRunningJobThreadCount()
{
   unsigned int rval = 0;
   
   // subtract idle threads from total threads
   rval = getJobThreadCount() - getIdleJobThreadCount();
   
   return rval;
}

/**
 * Gets the current number of idle JobThreads.
 * 
 * @return the current number of idle JobThreads.
 */
unsigned int JobThreadPool::getIdleJobThreadCount()
{
   unsigned int rval = 0;
   
   // synchronize
   lock();
   {
      // iterate through all threads, add up idle threads
      for(vector<JobThread*>::iterator i = mThreads.begin();
          i != mThreads.end(); i++)
      {
         JobThread* thread = *i;
         if(thread->isIdle())
         {
            rval++;
         }
      }
   }
   unlock();
   
   return rval;
}
