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
   mFinished = false;
   mCanceled = false;
   mStopped = false;
   mCollectable = false;
}

Operation::~Operation()
{
}
#include <iostream>
bool Operation::waitFor(bool interruptible)
{
   bool rval = false;
   
   std::cout << "Operation waitFor() grabbing lock" << std::endl;
   lock();
   {
      // wait until Operation has stopped
      while(!mStopped)
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
   std::cout << "Operation waitFor() released lock" << std::endl;
   
   // ensure thread remains interrupted
   if(rval)
   {
      std::cout << "Operation waitFor() HAD TO CALL INTERRUPT" << std::endl;
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

bool Operation::finished()
{
   return mFinished;
}

bool Operation::canceled()
{
   return mCanceled;
}

bool Operation::collectable()
{
   return mCollectable;
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
