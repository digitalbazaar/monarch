/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/OperationImpl.h"
#include "db/modest/OperationGuard.h"
#include "db/modest/StateMutator.h"

using namespace db::modest;
using namespace db::rt;

OperationImpl::OperationImpl(Runnable* r, OperationGuard* g, StateMutator* m)
{
   mRunnable = r;
   mGuard = g;
   mStateMutator = m;
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

OperationGuard* OperationImpl::getGuard()
{
   return mGuard;
}

StateMutator* OperationImpl::getStateMutator()
{
   return mStateMutator;
}

bool OperationImpl::interrupted()
{
   return Thread::interrupted(false);
}
