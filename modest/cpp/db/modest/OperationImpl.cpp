/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/OperationImpl.h"
#include "db/modest/OperationDispatcher.h"

using namespace db::modest;
using namespace db::rt;

OperationImpl::OperationImpl(Runnable& r)
{
   mRunnable = &r;
   mThread = NULL;
   mStarted = false;
   mInterrupted = false;
   mStopped = false;
   mFinished = false;
   mCanceled = false;
}

OperationImpl::OperationImpl(CollectableRunnable& r)
{
   mRunnable = &(*r);
   mRunnableReference = r;
   mThread = NULL;
   mStarted = false;
   mInterrupted = false;
   mStopped = false;
   mFinished = false;
   mCanceled = false;
}

OperationImpl::~OperationImpl()
{
}

void OperationImpl::run()
{
   lock();
   {
      // operation started on the current thread
      mThread = Thread::currentThread();
      mStarted = true;
   }
   unlock();
   
   // run the operation's runnable
   if(!isInterrupted())
   {
      mRunnable->run();
   }
   
   lock();
   {
      // determine if the operation was finished or canceled
      if(isInterrupted())
      {
         mCanceled = true;
      }
      else
      {
         mFinished = true;
      }
      
      // clear thread from Operation
      mThread = NULL;
   }
   unlock();
}

void OperationImpl::stop()
{
   lock();
   {
      // if operation did not finish, then it was canceled
      if(!finished())
      {
         mCanceled = true;
      }
      
      // mark operation stopped and wake up all waiting threads
      mStopped = true;
      notifyAll();
   }
   unlock();
}

InterruptedException* OperationImpl::waitFor(bool interruptible)
{
   InterruptedException* rval = NULL;
   
   lock();
   {
      // wait until Operation is stopped
      InterruptedException* e = NULL;
      while(!stopped())
      {
         if((e = wait()) != NULL)
         {
            // thread was interrupted
            e = rval;
            
            if(interruptible)
            {
               // interruptible, so break out
               break;
            }
            else
            {
               // clear thread interruption
               Thread::interrupted(true);
            }
         }
      }
   }
   unlock();
   
   // ensure thread remains interrupted
   if(rval != NULL)
   {
      Thread::currentThread()->interrupt();
   }
   
   return rval;
}

bool OperationImpl::started()
{
   return mStarted;
}

void OperationImpl::interrupt()
{
   lock();
   {
      if(!mInterrupted)
      {
         mInterrupted = true;
         if(mThread != NULL)
         {
            mThread->interrupt();
         }
      }
   }
   unlock();
}

bool OperationImpl::isInterrupted()
{
   if(!mInterrupted)
   {
      lock();
      {
         if(mThread != NULL)
         {
            mInterrupted = mThread->isInterrupted();
         }
      }
      unlock();
   }
   
   return mInterrupted;
}

bool OperationImpl::stopped()
{
   return mStopped;
}

bool OperationImpl::finished()
{
   return mFinished;
}

bool OperationImpl::canceled()
{
   return mCanceled;
}

Runnable* OperationImpl::getRunnable()
{
   return mRunnable;
}

void OperationImpl::addGuard(OperationGuard* g, bool front)
{
   // create new OperationGuardChain and add guard in order
   if(mGuard == NULL)
   {
      mGuard = new OperationGuardChain(g, NULL);
   }
   else if(front)
   {
      mGuard = new OperationGuardChain(g, mGuard);
   }
   else
   {
      mGuard = new OperationGuardChain(mGuard, g);
   }
}

void OperationImpl::addGuard(CollectableOperationGuard& g, bool front)
{
   // create new OperationGuardChain and add guard in order
   if(mGuard == NULL)
   {
      mGuard = new OperationGuardChain(g, NULL);
   }
   else if(front)
   {
      mGuard = new OperationGuardChain(g, mGuard);
   }
   else
   {
      mGuard = new OperationGuardChain(mGuard, g);
   }
}

OperationGuard* OperationImpl::getGuard()
{
   return mGuard.isNull() ? NULL : &(*mGuard);
}

void OperationImpl::addStateMutator(StateMutator* m, bool front)
{
   // create new StateMutatorChain and add mutator in order
   if(mMutator == NULL)
   {
      mMutator = new StateMutatorChain(m, NULL);
   }
   else if(front)
   {
      mMutator = new StateMutatorChain(m, mMutator);
   }
   else
   {
      mMutator = new StateMutatorChain(mMutator, m);
   }
}

void OperationImpl::addStateMutator(CollectableStateMutator& m, bool front)
{
   // create new StateMutatorChain and add mutator in order
   if(mMutator == NULL)
   {
      mMutator = new StateMutatorChain(m, NULL);
   }
   else if(front)
   {
      mMutator = new StateMutatorChain(m, mMutator);
   }
   else
   {
      mMutator = new StateMutatorChain(mMutator, m);
   }
}

StateMutator* OperationImpl::getStateMutator()
{
   return mMutator.isNull() ? NULL : &(*mMutator);
}

bool OperationImpl::interrupted()
{
   return Thread::interrupted(false);
}
