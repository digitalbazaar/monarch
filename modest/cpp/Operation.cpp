/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Operation.h"

using namespace std;
using namespace db::modest;
using namespace db::rt;

Operation::Operation(Runnable* r, OperationEnvironment* e, StateMutator* m)
{
   mRunnable = r;
   mEnvironment = e;
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
   // synchronize
   lock();
   {
      mInterrupted = true;
      if(mThread != NULL)
      {
         mThread->interrupt();
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

Runnable* Operation::getRunnable()
{
   return mRunnable;
}

OperationEnvironment* Operation::getEnvironment()
{
   return mEnvironment;
}

StateMutator* Operation::getStateMutator()
{
   return mStateMutator;
}

bool Operation::interrupted()
{
   return Thread::interrupted();
}
