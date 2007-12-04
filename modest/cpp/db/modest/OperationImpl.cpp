/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/OperationImpl.h"

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

bool OperationImpl::waitFor(bool interruptible)
{
   bool rval = false;
   
   lock();
   {
      // wait until Operation is stopped
      while(!stopped())
      {
         if(wait() != NULL)
         {
            // thread was interrupted
            rval = true;
            
            if(interruptible)
            {
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
   if(rval)
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
