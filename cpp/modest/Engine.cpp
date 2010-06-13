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
   mDispatch(false),
   mStateSemaphore(1, true)
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
   RunnableRef r = new Runner(
      this, &Engine::runOperation, new Operation(op), &Engine::freeOperation);

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

   // keep track of whether or not dispatch loop should break because an
   // operation would block due to lack of idle threads
   bool breakLoop = false;

   // iterate over the queued jobs, jobs may be added (to the end of the
   // queue) while iterating, but not removed so there is no danger of
   // iterator invalidation, synchronously get first job
   mLock.lock();
   JobList::iterator i = mJobQueue.begin();
   JobList::iterator end = mJobQueue.end();
   mLock.unlock();
   for(; !breakLoop && i != end;)
   {
      // acquire state access permit, to be released by an operation unless
      // one cannot be started
      bool opStarted = false;
      if(!mStateSemaphore.acquire())
      {
         // dispatch thread interrupted, break out
         break;
      }

      // lock while checking deleted flag on job, updating queue, etc.
      mLock.lock();

      // get next job
      JobDispatcher::Job& job = *i;
      if(i->deleted)
      {
         // job is marked for deletion, remove from queue
         i = mJobQueue.erase(i);
      }
      else
      {
         // get operation and guard
         Runner* runner = static_cast<Runner*>(&(*(*job.runnableRef)));
         Operation* op = runner->getParam();
         OperationGuard* og = (*op)->getGuard();

         // check the operation's guard restrictions
         if(og == NULL || og->canExecuteOperation(*op))
         {
            // try to run the operation without blocking
            if(getThreadPool()->tryRunJob(*job.runnableRef))
            {
               // operation executed, remove job from queue, turn on
               // dispatching because running an op could change the state
               // and allow other ops that were previously unable to run
               delete job.runnableRef;
               i = mJobQueue.erase(i);
               --mQueuedJobs;
               mDispatch = true;
               opStarted = true;
            }
            else
            {
               // operation would block, set flag to break out of loop
               breakLoop = true;
            }
         }
         // operation can wait
         else if(!(*op)->isInterrupted() && !og->mustCancelOperation(*op))
         {
            // move to next job
            ++i;
         }
         // operation must be canceled
         else
         {
            // stop operation, remove from queue
            (*op)->stop();
            delete job.runnableRef;
            i = mJobQueue.erase(i);
         }
      }

      // dispatch job complete
      mLock.unlock();

      if(!opStarted)
      {
         // op didn't start, release state access permit
         mStateSemaphore.release();
      }
   }
}

void Engine::runOperation(Operation* op)
{
   // do pre-execution state mutation
   StateMutator* sm = (*op)->getStateMutator();
   if(sm != NULL)
   {
      sm->mutatePreExecutionState(*op);
   }

   // release state access permit
   mStateSemaphore.release();

   // set thread user data to allow operation lookup by thread
   Thread* thread = Thread::currentThread();
   thread->setUserData(op);

   // run operation
   (*op)->run();

   // do post-execution state mutation
   if(sm != NULL)
   {
      // ensure state is mutated, but save interrupted state
      bool interrupted;
      while(!mStateSemaphore.acquire())
      {
         // save interrupted, but clear thread interruption flag
         interrupted = true;
         thread->interrupted(true);
      }
      // set interrupted flag
      if(interrupted)
      {
         thread->interrupt();
      }
      sm->mutatePostExecutionState(*op);
      mStateSemaphore.release();
   }

   // stop operation
   (*op)->stop();

   // clear thread user data
   thread->setUserData(NULL);

   // resume dispatching
   mLock.lock();
   mDispatch = true;
   wakeup();
   mLock.unlock();
}

void Engine::freeOperation(Operation* op)
{
   delete op;
}
