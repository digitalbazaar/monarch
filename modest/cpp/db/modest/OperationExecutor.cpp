/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/OperationExecutor.h"
#include "db/modest/OperationDispatcher.h"
#include "db/modest/OperationGuard.h"
#include "db/modest/StateMutator.h"

using namespace db::modest;
using namespace db::rt;

OperationExecutor::OperationExecutor(
   State* s, Operation& op, OperationDispatcher* od)
{
   mState = s;
   mOperation = op;
   mDispatcher = od;
}

OperationExecutor::~OperationExecutor()
{
}

void OperationExecutor::run()
{
   mOperation->lock();
   {
      // operation started on the current thread
      mOperation->mThread = Thread::currentThread();
      mOperation->mStarted = true;
   }
   mOperation->unlock();
   
   // run the operation's runnable
   if(mOperation->getRunnable() != NULL && !mOperation->isInterrupted())
   {
      mOperation->getRunnable()->run();
   }
   
   mOperation->lock();
   {
      // clear thread from Operation
      mOperation->mThread = NULL;
   }
   mOperation->unlock();
   
   // determine if the operation was finished or canceled
   if(mOperation->isInterrupted())
   {
      mOperation->mCanceled = true;
   }
   else
   {
      mOperation->mFinished = true;
   }
   
   // do post-execution state mutation
   if(mOperation->getStateMutator() != NULL)
   {
      mState->lock();
      {
         mOperation->getStateMutator()->mutatePostExecutionState(
            mState, mOperation);
      }
      mState->unlock();
   }
   
   // executor now expired
   mDispatcher->addExpiredExecutor(this);
}

void OperationExecutor::doPreExecutionStateMutation()
{
   // do pre-execution state mutation (state already locked)
   if(mOperation->getStateMutator() != NULL)
   {
      mOperation->getStateMutator()->mutatePreExecutionState(
         mState, mOperation);
   }
}

int OperationExecutor::checkGuard()
{
   int rval = 0;
   
   // check the Operation' guard restrictions, if a guard exists
   OperationGuard* g = mOperation->getGuard();
   if(g != NULL)
   {
      ImmutableState* s = (ImmutableState*)mState;
      if(!g->canExecuteOperation(s))
      {
         if(!mOperation->isInterrupted() && !g->mustCancelOperation(s))
         {
            // operation can wait
            rval = 1;
         }
         else
         {
            // operation must be canceled
            rval = 2;
         }
      }
   }
   
   return rval;
}

void OperationExecutor::cleanup()
{
   // if operation did not finish, then it was canceled
   if(!mOperation->finished())
   {
      mOperation->mCanceled = true;
   }
   
   mOperation->lock();
   {
      // wake up all waiting threads
      mOperation->mStopped = true;
      mOperation->notifyAll();
   }
   mOperation->unlock();
}
