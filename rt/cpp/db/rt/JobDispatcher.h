/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_JobDispatcher_H
#define db_rt_JobDispatcher_H

#include "db/rt/ThreadPool.h"

#include <map>

namespace db
{
namespace rt
{

/**
 * A JobDispatcher is a class that maintains a queue of Runnable jobs
 * that are dispatched on a separate thread in a ThreadPool.
 * 
 * @author Dave Longley
 */
class JobDispatcher : public virtual ExclusiveLock, public Runnable
{
protected:
   /**
    * A thread pool for running jobs. This thread pool maintains
    * a set of threads that are available for running jobs.
    */
   ThreadPool* mThreadPool;
   
   /**
    * True to free the internal thread pool when destructing, false not to.
    */
   bool mCleanupThreadPool;
   
   /**
    * The internal queue that holds the Runnable jobs that are
    * waiting to be dispatched.
    */
   typedef std::list<Runnable*> RunnableList;
   RunnableList mJobQueue;
   
   /**
    * A map of Runnables to RunnableRefs.
    */
   typedef std::map<Runnable*, RunnableRef> ReferenceMap;
   ReferenceMap mJobReferenceMap;
   
   /**
    * The thread used to dispatch the Runnable jobs.
    */
   Thread* mDispatcherThread;
   
   /**
    * The wait lock for this dispatcher.
    */
   ExclusiveLock mWaitLock;
   
   /**
    * Wakes up this dispatcher if it has gone to sleep waiting for
    * jobs to become dispatchable.
    */
   virtual void wakeup();
   
   /**
    * Returns true if this dispatcher has a job it can dispatch.
    * 
    * @return true if this dispatcher has a job it can dispatch.
    */
   virtual bool canDispatch();
   
   /**
    * Gets the dispatcher thread.
    * 
    * @return the dispatcher thread.
    */
   virtual Thread* getDispatcherThread();
   
public:
   /**
    * Creates a new JobDispatcher with an unlimited number of threads that
    * have an idle expiration time of 5 minutes.
    */
   JobDispatcher();
   
   /**
    * Creates a new JobDispatcher with the given ThreadPool.
    * 
    * @param pool the ThreadPool to dispatch jobs to.
    * @param cleanupPool true to free the passed ThreadPool upon destruction,
    *                    false not to.
    */
   JobDispatcher(ThreadPool* pool, bool cleanupPool);
   
   /**
    * Destructs this JobDispatcher.
    */
   virtual ~JobDispatcher();
   
   /**
    * Queues a Runnable job for execution. The dispatcher will automatically
    * be woken up when a new job is added.
    * 
    * @param job the Runnable job to queue.
    */
   virtual void queueJob(Runnable& job);
   virtual void queueJob(RunnableRef& job);
   
   /**
    * Dequeues a Runnable job so that it will no longer be executed if it
    * has not already begun execution. The dispatcher will automatically
    * be woken up when a job is removed.
    * 
    * @param job the Runnable job to dequeue.
    */
   virtual void dequeueJob(Runnable& job);
   virtual void dequeueJob(RunnableRef& job);
   
   /**
    * Dispatches the Runnable jobs in the queue that can be dispatched.
    */
   virtual void dispatchJobs();
   
   /**
    * Returns true if the passed Runnable job is in the queue
    * to be dispatched, false if not.
    *
    * @param job the Runnable job to look for.
    *
    * @return true if the passed Runnable job is in the queue to
    *         be dispatched, false if not.
    */
   virtual bool isQueued(Runnable& job);
   virtual bool isQueued(RunnableRef& job);
   
   /**
    * Starts dispatching Runnable jobs.
    */
   virtual void startDispatching();
   
   /**
    * Stops dispatching Runnable jobs. This does not terminate the jobs
    * that are already running.
    */
   virtual void stopDispatching();
   
   /**
    * Called by startDispatching() to dispatch Runnable jobs.
    */
   virtual void run();
   
   /**
    * Returns true if this JobDispatcher is dispatching jobs, false if not.
    * 
    * @return true if this JobDispatcher is dispatching jobs, false if not.
    */
   virtual bool isDispatching();
   
   /**
    * Removes all queued jobs. Already running jobs will not be affected.
    */
   virtual void clearQueuedJobs();
   
   /**
    * Interrupts all running Jobs. Queued jobs will not be affected.
    */
   virtual void interruptAllRunningJobs();
   
   /**
    * Terminates all running Jobs. Queued jobs will not be affected. This
    * method will wait until all of the threads are joined.
    */
   virtual void terminateAllRunningJobs();
   
   /**
    * Gets the ThreadPool.
    * 
    * @return the thread pool for running jobs.
    */
   virtual ThreadPool* getThreadPool();
   
   /**
    * Gets the number of Runnable jobs that are in the queue.
    * 
    * @return the number of Runnable jobs that are queued to be dispatched.
    */
   virtual unsigned int getQueuedJobCount();
   
   /**
    * Gets the number of Runnable jobs that are in the queue and that are
    * running.
    * 
    * @return the number of Runnable jobs that are queued to be dispatched
    *         plus the Runnable jobs that are already running.
    */
   virtual unsigned int getTotalJobCount();
};

} // end namespace rt
} // end namespace db
#endif
