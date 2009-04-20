/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 * 
 * This test file is used to test deadlock issues with the SharedLock
 * class absent any use of the dbcore App Tester framework.
 */

#include <cstdio>

#include "db/rt/Thread.h"
#include "db/rt/ExclusiveLock.h"
#include "db/rt/SharedLock.h"

using namespace std;
using namespace db::rt;

class DeadlockRunnable : public Runnable
{
public:
   SharedLock* mLock;
   ExclusiveLock* mSignalLock;
   bool* mSignal;
   bool mWrite;
   
   DeadlockRunnable(
      SharedLock* lock, ExclusiveLock* signalLock, bool* signal, bool write) :
      mLock(lock),
      mSignalLock(signalLock),
      mSignal(signal),
      mWrite(write)
   {
   }
   
   virtual ~DeadlockRunnable()
   {
   }
   
   virtual void run()
   {
      if(mWrite)
      {
         // wait for signal to get exclusive lock
         mSignalLock->lock();
         while(!(*mSignal))
         {
            mSignalLock->wait();
         }
         mSignalLock->unlock();
         
         // get exclusive lock
         mLock->lockExclusive();
         
         // should block forever if test fails
         
         mLock->unlockExclusive();
      }
      else
      {
         // get shared lock
         mLock->lockShared();
         {
            // set signal
            mSignalLock->lock();
            *mSignal = true;
            mSignalLock->notifyAll();
            mSignalLock->unlock();
            
            // wait to allow lock exclusive to occur in write thread
            Thread::sleep(250);
            
            // try to get shared lock
            mLock->lockShared();
            
            // should block here for ever if test fails
            
            // recursive unlock shared lock
            mLock->unlockShared();
         }
         mLock->unlockShared();
      }
   }
};

int main()
{
   printf("Testing SharedLock deadlock, will pass if no deadlock...");
   
   // this test checks to see if thread 1 can get a read lock,
   // wait for thread 2 to get a write lock, and then see if
   // thread 1 can recurse its read lock (it should be able to)
   
   SharedLock lock;
   ExclusiveLock signalLock;
   bool signal = false;
   
   DeadlockRunnable r1(&lock, &signalLock, &signal, false);
   DeadlockRunnable r2(&lock, &signalLock, &signal, true);
   
   Thread t1(&r1);
   Thread t2(&r2);
   
   t2.start();
   t1.start();
   
   t1.join();
   t2.join();
   
   printf("PASS.\n");
   
   Thread::exit();
   return 0;
}
