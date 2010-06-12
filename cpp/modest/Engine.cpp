/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/Engine.h"

#include "monarch/rt/RunnableDelegate.h"

using namespace std;
using namespace monarch::rt;
using namespace monarch::modest;

typedef RunnableDelegate<Engine, Operation*> Runner;

Engine::Engine() :
   JobDispatcher(new ThreadPool(100), true),
   mDispatch(false)
{
   // set thread expire time to 2 minutes (120000 milliseconds) by default
   mThreadPool->setThreadExpireTime(120000);
}

Engine::~Engine()
{
   // ensure engine is stopped
   stop();
}

void Engine::start()
{
   mStartLock.lock();
   {
      mDispatch = true;
      startDispatching();
   }
   mStartLock.unlock();
}

void Engine::stop()
{
   mStartLock.lock();
   {
      // stop dispatching
      stopDispatching();

      // terminate all running operations
      terminateRunningOperations();

      // clear all queued operations
      clearQueuedOperations();
   }
   mStartLock.unlock();
}

void Engine::queue(Operation& op)
{
   // create runnable
   RunnableRef r = new Runner(this, &Engine::runOperation, new Operation(op));

   mLock.lock();
   {
      // enable dispatching and queue runnable
      mDispatch = true;
      queueJob(r);
   }
   mLock.unlock();
}

void Engine::clearQueuedOperations()
{
   clearQueuedJobs();
}

void Engine::terminateRunningOperations()
{
   JobDispatcher::terminateAllRunningJobs();

   // wake up dispatcher, don't care if dispatch flag changes no need to lock
   wakeup();
}

Operation Engine::getCurrentOperation()
{
   Operation rval(NULL);

   // get the current thread's operation reference
   Thread* thread = Thread::currentThread();
   Operation* op = static_cast<Operation*>(thread->getUserData());
   if(op != NULL)
   {
      rval = *op;
   }

   return rval;
}

inline ThreadPool* Engine::getThreadPool()
{
   return mThreadPool;
}

unsigned int Engine::getQueuedOperationCount()
{
   return JobDispatcher::getQueuedJobCount();
}

unsigned int Engine::getTotalOperationCount()
{
   return JobDispatcher::getTotalJobCount();
}

bool Engine::canDispatch()
{
   return mDispatch;
}

void Engine::dispatchJobs()
{
   // turn off dispatching (if an operation starts or completes it will be
   // turned back on)
   mDispatch = false;

   // keep track of whether or not dispatch loop should break:
   // 1. There are no more queued jobs.
   // 2. If an operation would block due to lack of idle threads.
   // 3. The queue has been cycled.
   bool breakLoop = false;
   Operation* first = NULL;
   JobList::iterator i;

   do
   {
      // lock while trying to dispatch the next operation
      mLock.lock();

      // Note: Here we can't just iterate through the job queue because
      // we unlock to allow more jobs to queue and for the job queue to be
      // cleared if requested. Since the queue could be cleared, we can't
      // rely on iterators to stay valid.
      breakLoop = mJobQueue.empty();
      if(!breakLoop)
      {
         // get next job and operation
         JobDispatcher::Job job = mJobQueue.front();
         Runner* runner = static_cast<Runner*>(&(*(*job.runnableRef)));
         Operation* op = runner->getParam();
         if(first == op)
         {
            // queue cycle detected
            breakLoop = true;
         }
         else
         {
            // pop job
            mJobQueue.pop_front();

            // get operation guard
            OperationGuard* og = (*op)->getGuard();

            // lock state while guards/mutators are used, operation will unlock
            // state if it can be started
            mStateLock.lock();

            // check the operation's guard restrictions
            bool opStarted = false;
            if(og == NULL || og->canExecuteOperation(*op))
            {
               // try to run the operation without blocking
               if(getThreadPool()->tryRunJob(*job.runnableRef))
               {
                  // operation executed, turn on dispatching because running
                  // an op could change the state and allow other ops that
                  // were previously unable to run to run
                  delete job.runnableRef;
                  mDispatch = true;
                  opStarted = true;
               }
               else
               {
                  // operation would block, set flag to break out of loop
                  // and requeue job
                  breakLoop = true;
                  mJobQueue.push_back(job);
               }
            }
            // operation can wait
            else if(!(*op)->isInterrupted() && !og->mustCancelOperation(*op))
            {
               // requeue job, mark as first in cycle if not yet set
               mJobQueue.push_back(job);
               if(first == NULL)
               {
                  first = op;
               }
            }
            // operation must be canceled
            else
            {
               // stop operation, do not requeue it
               (*op)->stop();
               delete job.runnableRef;
            }

            if(!opStarted)
            {
               // op didn't start, release state lock
               mStateLock.unlock();
            }
         }
      }

      // dispatch job complete
      mLock.unlock();
   }
   while(!breakLoop);
}

void Engine::runOperation(Operation* op)
{
   // do pre-execution state mutation
   StateMutator* sm = (*op)->getStateMutator();
   if(sm != NULL)
   {
      sm->mutatePreExecutionState(*op);
   }

   // unlock state
   mStateLock.unlock();

   // set thread user data to allow operation lookup by thread
   Thread* thread = Thread::currentThread();
   thread->setUserData(op);

   // run operation
   (*op)->run();

   // do post-execution state mutation
   if(sm != NULL)
   {
      mStateLock.lock();
      sm->mutatePostExecutionState(*op);
      mStateLock.unlock();
   }

   // stop operation
   (*op)->stop();

   // clear thread user data
   thread->setUserData(NULL);

   // clean up operation
   delete op;

   // resume dispatching
   mLock.lock();
   mDispatch = true;
   wakeup();
   mLock.unlock();
}
