/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Operation.h"

using namespace db::modest;
using namespace db::rt;

Operation::Operation(Runnable* r, OperationGuard* g, StateMutator* m)
{
   mRunnable = r;
   mGuard = g;
   mStateMutator = m;
   
   mStarted = false;
   mInterrupted = false;
   mStopped = false;
   mFinished = false;
   mCanceled = false;
}

Operation::~Operation()
{
}

bool Operation::waitFor(bool interruptible)
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

bool Operation::started()
{
   return mStarted;
}

void Operation::interrupt()
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

bool Operation::isInterrupted()
{
   return mInterrupted;
}

bool Operation::stopped()
{
   return mStopped;
}

bool Operation::finished()
{
   return mFinished;
}

bool Operation::canceled()
{
   return mCanceled;
}

Runnable* Operation::getRunnable()
{
   return mRunnable;
}

OperationGuard* Operation::getGuard()
{
   return mGuard;
}

StateMutator* Operation::getStateMutator()
{
   return mStateMutator;
}

bool Operation::interrupted()
{
   return Thread::interrupted(false);
}
