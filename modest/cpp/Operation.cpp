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
}

Operation::~Operation()
{
}

void Operation::waitFor()
{
   lock();
   {
      // wait until Operation finishes or is canceled
      while(!finished() && !canceled())
      {
         wait();
      }
   }
   unlock();
}

bool Operation::started()
{
   return mStarted;
}

void Operation::interrupt()
{
   mInterrupted = true;
   if(mThread != NULL)
   {
      mThread->interrupt();
   }
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
   return Thread::interrupted();
}
