/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/SharedLock.h"

using namespace db::rt;

SharedLock::SharedLock()
{
   mShared = 0;
   mExclusive = 0;
   mThread = NULL;
}

SharedLock::~SharedLock()
{
}

void SharedLock::lockShared()
{
   // check status
   mStatusLock.lock();
   {
      // wait on status lock until exclusive lock not engaged
      // or until exclusive lock is held by this thread
      while(mExclusive > 0 || mThread == Thread::currentThread())
      {
         mStatusLock.wait();
      }
      
      // acquire shared lock
      mShared++;
   }
   mStatusLock.unlock();
}

void SharedLock::unlockShared()
{
   mStatusLock.lock();
   {
      // decrement shared lock
      mShared--;
      
      if(mShared == 0)
      {
         // notify waiting locks
         mStatusLock.notifyAll();
      }
   }
   mStatusLock.unlock();
}

void SharedLock::lockExclusive()
{
   mStatusLock.lock();
   {
      // see if this thread already holds the exclusive lock
      Thread* t = Thread::currentThread();
      if(mThread != t)
      {
         // wait on status lock until no lock is engaged
         while(mExclusive > 0 || mShared > 0)
         {
            mStatusLock.wait();
         }
      }
      
      // acquire exclusive lock
      mThread = t;
      mExclusive++;
   }
   mStatusLock.unlock();
}

void SharedLock::unlockExclusive()
{
   mStatusLock.lock();
   {
      // decrement exclusive lock
      mExclusive--;
      
      if(mExclusive == 0)
      {
         // notify waiting locks
         mStatusLock.notifyAll();
      }
   }
   mStatusLock.unlock();
}
