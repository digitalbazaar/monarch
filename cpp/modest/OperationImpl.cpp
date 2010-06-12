/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/modest/OperationImpl.h"

#include "monarch/modest/Engine.h"

using namespace monarch::modest;
using namespace monarch::rt;

OperationImpl::OperationImpl(Runnable& r)
{
   mRunnable = &r;
   mThread = NULL;
   mStarted = false;
   mInterrupted = false;
   mStopped = false;
   mFinished = false;
   mCanceled = false;
   mUserData = NULL;
}

OperationImpl::OperationImpl(RunnableRef& r)
{
   mRunnable = r.isNull() ? NULL : &(*r);
   mRunnableReference = r;
   mThread = NULL;
   mStarted = false;
   mInterrupted = false;
   mStopped = false;
   mFinished = false;
   mCanceled = false;
   mUserData = NULL;
}

OperationImpl::~OperationImpl()
{
}

void OperationImpl::run()
{
   mLock.lock();
   {
      // operation started on the current thread
      mThread = Thread::currentThread();
      mThread->setUserData(this);
      mStarted = true;

      // interrupt thread if appropriate
      if(mInterrupted)
      {
         mThread->interrupt();
      }
   }
   mLock.unlock();

   if(mRunnable != NULL)
   {
      // run the operation's runnable
      mRunnable->run();
   }

   mLock.lock();
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
      mThread->setUserData(NULL);
      mThread = NULL;
   }
   mLock.unlock();
}

void OperationImpl::stop()
{
   mLock.lock();
   {
      // if operation did not finish, then it was canceled
      if(!mFinished)
      {
         mCanceled = true;
      }

      // mark operation stopped and wake up all waiting threads
      mStopped = true;
      mLock.notifyAll();
   }
   mLock.unlock();
}

bool OperationImpl::waitFor(bool interruptible, uint32_t timeout)
{
   bool rval = true;

   mLock.lock();
   {
      // wait until Operation is stopped or timed out
      bool interrupted = false;
      bool timedOut = false;
      uint32_t remaining = timeout;
      while(!mStopped && !timedOut)
      {
         if(timeout > 0)
         {
            interrupted = !mLock.wait(remaining, &mStopped, true);
         }
         else
         {
            interrupted = !mLock.wait();
         }

         if(interrupted)
         {
            // thread was interrupted
            rval = false;

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
         else if(timeout > 0 && remaining == 0)
         {
            timedOut = true;
         }
      }
   }
   mLock.unlock();

   // ensure thread remains interrupted
   if(!rval)
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
   mLock.lock();
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
   mLock.unlock();
}

bool OperationImpl::isInterrupted()
{
   if(!mInterrupted)
   {
      mLock.lock();
      {
         if(mThread != NULL)
         {
            mInterrupted = mThread->isInterrupted();
         }
      }
      mLock.unlock();
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

Thread* OperationImpl::getThread()
{
   return mThread;
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

void OperationImpl::addGuard(OperationGuardRef& g, bool front)
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

void OperationImpl::addStateMutator(StateMutatorRef& m, bool front)
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

void OperationImpl::setUserData(void* userData)
{
   mUserData = userData;
}

void* OperationImpl::getUserData()
{
   return mUserData;
}

bool OperationImpl::interrupted()
{
   return Thread::interrupted(false);
}
