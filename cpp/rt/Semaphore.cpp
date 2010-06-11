/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/rt/Semaphore.h"

#include "monarch/rt/Thread.h"

using namespace std;
using namespace monarch::rt;

Semaphore::Semaphore(int permits, bool fair)
{
   // set permits
   mPermits = permits;
   mPermitsLeft = permits;

   // set fair/not fair
   mFair = fair;
}

Semaphore::~Semaphore()
{
}

int Semaphore::increasePermitsLeft(int increase)
{
   // only increase, at most, by the difference between the
   // max permit count and the permits left
   int permits = mPermits - mPermitsLeft;
   increase = (permits < increase) ? permits : increase;
   mPermitsLeft += increase;

   return increase;
}

inline void Semaphore::decreasePermitsLeft(int decrease)
{
   mPermitsLeft -= decrease;
}

bool Semaphore::waitThread(Thread* t)
{
   bool rval = true;

   // add thread to wait queue
   mWaitMap.insert(make_pair(t, true));
   mWaitList.push_back(t);

   // wait while not interrupted and in the list of waiting threads
   while(rval && mustWait(t))
   {
      if(!(rval = wait()))
      {
         // thread has been interrupted, so notify other waiting
         // threads and remove this thread from the wait list
         notifyThreads(1);

         // remove thread from wait queue
         mWaitList.remove(t);
         mWaitMap.erase(t);
      }
   }

   return rval;
}

void Semaphore::notifyThreads(int count)
{
   if(isFair())
   {
      for(int i = 0; !mWaitList.empty() && i < count; ++i)
      {
         // remove first thread
         Thread* thread = mWaitList.front();
         mWaitList.pop_front();
         mWaitMap.erase(thread);
      }
   }
   else
   {
      // erase according to map entry
      WaitMap::iterator i = mWaitMap.begin();
      WaitMap::iterator prev;
      for(int n = 0; i != mWaitMap.end() && n < count; n++)
      {
         // store iterator and advance it
         prev = i++;

         // remove thread
         mWaitList.remove(prev->first);
         mWaitMap.erase(prev);
      }
   }

   // notify all threads to wake up
   notifyAll();
}

inline bool Semaphore::mustWait(Thread* thread)
{
   // return true if thread is in the wait map
   return mWaitMap.find(thread) != mWaitMap.end();
}

bool Semaphore::acquire()
{
   return acquire(1);
}

bool Semaphore::acquire(int permits)
{
   bool rval = true;

   lock();
   {
      // see if enough permits are available
      Thread* t = Thread::currentThread();
      while(rval && availablePermits() - permits < 0)
      {
         // must wait for permits
         rval = waitThread(t);
      }

      if(rval)
      {
         // permits have been granted, decrease permits left
         decreasePermitsLeft(permits);
      }
   }
   unlock();

   return rval;
}

bool Semaphore::tryAcquire()
{
   return tryAcquire(1);
}

bool Semaphore::tryAcquire(int permits)
{
   bool rval = false;

   lock();
   {
      if(availablePermits() - permits >= 0)
      {
         // decrease permits left
         decreasePermitsLeft(permits);

         // permits granted
         rval = true;
      }
   }
   unlock();

   return rval;
}

void Semaphore::release()
{
   release(1);
}

int Semaphore::release(int permits)
{
   int rval = 0;

   lock();
   {
      // increase the number of permits left
      rval = increasePermitsLeft(permits);

      // notify threads for number of permits
      notifyThreads(permits);
   }
   unlock();

   return rval;
}

inline int Semaphore::availablePermits()
{
   return mPermitsLeft;
}

int Semaphore::usedPermits()
{
   int rval = 0;

   lock();
   {
      rval = mPermits - mPermitsLeft;
   }
   unlock();

   return rval;
}

inline bool Semaphore::isFair()
{
   return mFair;
}

inline const list<Thread*>& Semaphore::getQueuedThreads()
{
   return mWaitList;
}

inline int Semaphore::getQueueLength()
{
   return mWaitList.size();
}

void Semaphore::setMaxPermitCount(int max)
{
   lock();
   {
      // store old permit count
      int oldPermitCount = mPermits;

      // set new permit count
      mPermits = max;

      // release more permits or decrease permits left
      if(max > oldPermitCount)
      {
         // release used permits
         release(max - oldPermitCount);
      }
      else
      {
         // decrease permits left
         decreasePermitsLeft(oldPermitCount - max);
      }
   }
   unlock();
}

int Semaphore::getMaxPermitCount()
{
   return mPermits;
}
