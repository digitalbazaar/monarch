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
          i != mThreads.end();)
      {
         JobThread* thread = *i;
         if(thread->isIdle())
         {
            // if the thread is not alive, remove it and continue on
            if(!thread->isAlive())
            {
               cout << "removing expired thread." << endl;
               
               // remove thread
               i = mThreads.erase(i);
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
                  i = mThreads.erase(i);
                  delete thread;
                  
                  // decrement extra threads
                  extraThreads--;
               }
               else
               {
                  if(rval == NULL)
                  {
                     // return this thread
                     rval = thread;
                  }
                  
                  // move to the next thread
                  i++;
               }
            }
         }
         else
         {
            // move to the next thread
            i++;
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
               i = mThreads.erase(i);
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

unsigned int JobThreadPool::getPoolSize()
{
   return mThreadSemaphore.getMaxPermitCount();
}

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
             i != mThreads.end();)
         {
            JobThread* thread = *i;
            
            cout << "joining thread..." << endl;
            
            // join thread
            thread->join(joinTime);
            
            // interrupt and detach thread if it is still alive
            if(thread->isAlive())
            {
               thread->interrupt();
               thread->detach();
            }
            
            cout << "thread joined." << endl;
            
            // remove thread
            i = mThreads.erase(i);
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

unsigned long long JobThreadPool::getJobThreadExpireTime()
{
   return mJobThreadExpireTime;
}

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

unsigned int JobThreadPool::getRunningJobThreadCount()
{
   unsigned int rval = 0;
   
   // subtract idle threads from total threads
   rval = getJobThreadCount() - getIdleJobThreadCount();
   
   return rval;
}

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
