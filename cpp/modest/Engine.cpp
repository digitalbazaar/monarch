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
   for(; !mDispatcherThread->isInterrupted() && !breakLoop && i != end;)
   {
      // get next job
      JobDispatcher::Job& job = *i;
      if(i->deleted)
      {
         // job is marked for deletion, remove from queue
         mLock.lock();
         if(job.type == Job::TypeRunnableRef)
         {
            delete job.runnableRef;
         }
         i = mJobQueue.erase(i);
         mLock.unlock();
      }
      else
      {
         // lock state to check guard/run pre-execution mutation
         mStateLock.lock();

         // get operation, guard, and mutator
         Runner* runner = static_cast<Runner*>(&(*(*job.runnableRef)));
         Operation* op = runner->getParam();
         OperationGuard* og = (*op)->getGuard();
         StateMutator* sm = (*op)->getStateMutator();

         // check the operation's guard restrictions
         if(og == NULL || og->canExecuteOperation(*op))
         {
            // do pre-execution state mutation, release state lock
            if(sm != NULL)
            {
               sm->mutatePreExecutionState(*op);
            }
            mStateLock.unlock();

            // run the operation, do not allow interruptions, but remember
            // them if they occur
            bool interrupted = false;
            while(!getThreadPool()->runJob(*job.runnableRef))
            {
               interrupted = true;
               Thread::interrupted(true);
            }
            if(interrupted)
            {
               mDispatcherThread->interrupt();
            }

            // remove job from queue, turn on dispatching because running
            // an op could change the state and allow other ops that were
            // previously unable to run to run
            mLock.lock();
            delete job.runnableRef;
            i = mJobQueue.erase(i);
            --mQueuedJobs;
            mDispatch = true;
            mLock.unlock();
         }
         // operation can wait
         else if(!(*op)->isInterrupted() && !og->mustCancelOperation(*op))
         {
            // unlock state, move to next job
            mStateLock.unlock();
            mLock.lock();
            ++i;
            mLock.unlock();
         }
         // operation must be canceled
         else
         {
            // unlock state, stop operation
            mStateLock.unlock();
            (*op)->stop();

            // remove from queue
            mLock.lock();
            delete job.runnableRef;
            i = mJobQueue.erase(i);
            mLock.unlock();
         }
      }
   }
}

void Engine::runOperation(Operation* op)
{
   // set thread user data to allow operation lookup by thread
   Thread* thread = Thread::currentThread();
   thread->setUserData(op);

   // run operation
   (*op)->run();

   // do post-execution state mutation
   StateMutator* sm = (*op)->getStateMutator();
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
