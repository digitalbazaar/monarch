/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/modest/OperationImpl.h"

#include "db/modest/OperationDispatcher.h"

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
   mUserData = NULL;
}

OperationImpl::OperationImpl(RunnableRef& r)
{
   mRunnable = &(*r);
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
   lock();
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
   unlock();
   
   // run the operation's runnable
   mRunnable->run();
   
   lock();
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
   unlock();
}

void OperationImpl::stop()
{
   lock();
   {
      // if operation did not finish, then it was canceled
      if(!finished())
      {
         mCanceled = true;
      }
      
      // mark operation stopped and wake up all waiting threads
      mStopped = true;
      notifyAll();
   }
   unlock();
}

bool OperationImpl::waitFor(bool interruptible)
{
   bool rval = true;
   
   lock();
   {
      // wait until Operation is stopped
      while(!stopped())
      {
         if(!wait())
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
      }
   }
   unlock();
   
   // ensure thread remains interrupted
   if(!rval)
   {
      Thread::currentThread()->interrupt();
   }
   
   return rval;
}

inline bool OperationImpl::started()
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

inline bool OperationImpl::stopped()
{
   return mStopped;
}

inline bool OperationImpl::finished()
{
   return mFinished;
}

inline bool OperationImpl::canceled()
{
   return mCanceled;
}

inline Thread* OperationImpl::getThread()
{
   return mThread;
}

inline Runnable* OperationImpl::getRunnable()
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

inline OperationGuard* OperationImpl::getGuard()
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

inline StateMutator* OperationImpl::getStateMutator()
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
