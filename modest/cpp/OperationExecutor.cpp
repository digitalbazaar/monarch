/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "OperationExecutor.h"
#include "OperationDispatcher.h"
#include "Thread.h"

using namespace db::modest;
using namespace db::rt;

OperationExecutor::OperationExecutor(
   State* s, Operation* op, OperationDispatcher* od)
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
   // operation started on the current thread
   mOperation->mThread = Thread::currentThread();
   mOperation->mStarted = true;
   
   // run the operation's runnable
   if(mOperation->getRunnable() != NULL && !mOperation->isInterrupted())
   {
      mOperation->getRunnable()->run();
   }
   
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
   
   mOperation->lock();
   {
      // wake up all waiting threads
      mOperation->notifyAll();
   }
   mOperation->unlock();
}

void OperationExecutor::execute()
{
   // do pre-execution state mutation
   if(mOperation->getStateMutator() != NULL)
   {
      mState->lock();
      {
         mOperation->getStateMutator()->mutatePreExecutionState(
            mState, mOperation);
      }
      mState->unlock();
   }
   
   // run using dispatcher's thread pool
   mDispatcher->getThreadPool()->runJob(this);
}

int OperationExecutor::checkEnvironment()
{
   int rval = 0;
   
   // check the Operation's environment restrictions
   OperationEnvironment* e = mOperation->getEnvironment();
   if(e != NULL)
   {
      ImmutableState* s = (ImmutableState*)mState;
      if(!e->canExecuteOperation(s))
      {
         if(!mOperation->isInterrupted() && !e->mustCancelOperation(s))
         {
            // operation can wait
            rval = 1;
         }
         else
         {
            // operation must be canceled
            rval = 2;
            mOperation->mCanceled = true;
            mDispatcher->addExpiredExecutor(this);
         }
      }
   }
   
   return rval;
}
