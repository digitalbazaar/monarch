/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "OperationExecutor.h"
#include "OperationDispatcher.h"
#include "Thread.h"

using namespace std;
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
#include <iostream>
void OperationExecutor::run()
{
   cout << ".......STARTING OPERATIONEXECUTOR" << endl;
   
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
   
   cout << ".......FINISHED OPERATIONEXECUTOR" << endl;
   
   // executor now expired
   mDispatcher->addExpiredExecutor(this);
}

void OperationExecutor::execute()
{
   // do pre-execution state mutation
   if(mOperation->getStateMutator() != NULL)
   {
      mOperation->getStateMutator()->mutatePreExecutionState(
         mState, mOperation);
   }
   
   // unlock state, it will have been locked by dispatcher
   mState->unlock();
   
   // run using dispatcher's thread pool
   mDispatcher->getThreadPool()->runJob(this);
}

int OperationExecutor::checkGuard()
{
   int rval = 0;
   
   cout << "***********CHECKING GUARD" << endl;
   
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
   
   cout << "***********GUARD CHECKED" << endl;
   
   return rval;
}

void OperationExecutor::cleanup()
{
   cout << ".........................OPERATION EXECUTOR CLEANUP!" << endl;
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
   
   cout << ".........................OPERATION EXECUTOR CLEANUP COMPLETE!" << endl;
}

string& OperationExecutor::toString(string& str)
{
   if(mOperation->getRunnable() != NULL)
   {
      str = "Operation: " + mOperation->getRunnable()->toString(str);
   }
   else
   {
      str = "Operation: NULL";
   }
   
   return str;
}
