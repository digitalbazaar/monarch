/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Semaphore.h"

#include <cstdlib>
#include <ctime>

using namespace std;
using namespace db::rt;

Semaphore::Semaphore(int permits, bool fair)
{
   // set permits
   mPermits = permits;
   mPermitsLeft = permits;
   
   // set fair/not fair
   mFair = fair;
   
   // seed random
   srand((unsigned int)time(0));
}

Semaphore::~Semaphore()
{
}

int Semaphore::increasePermitsLeft(int increase)
{
   lock();
   {
      // only increase, at most, by the difference between the
      // max permit count and the permits left
      int permits = mPermits - mPermitsLeft;
      increase = (permits < increase) ? permits : increase;
      
      mPermitsLeft += increase;
   }
   unlock();
   
   return increase;
}

void Semaphore::decreasePermitsLeft(int decrease)
{
   lock();
   {
      mPermitsLeft -= decrease;
   }
   unlock();
}

InterruptedException* Semaphore::waitThread()
{
   InterruptedException* rval = NULL;
   
   // get the current thread
   Thread* t = Thread::currentThread();
   
   // add thread to waiting threads
   addWaitingThread(t);
   
   // synchronize on lock object
   mLockObject.lock();
   {
      // wait while in the list of waiting threads
      while(rval == NULL && mustWait(t))
      {
         if((rval = mLockObject.wait()) != NULL)
         {
            // thread has been interrupted, so notify other waiting
            // threads and remove this thread from the wait list
            notifyThreads();
            removeWaitingThread(t);
         }
      }
   }
   mLockObject.unlock();
   
   return rval;
}

void Semaphore::notifyThreads()
{
   lock();
   {
      if(isFair())
      {
         // remove the first waiting thread
         removeFirstWaitingThread();
      }
      else
      {
         // remove a random waiting thread
         removeRandomWaitingThread();
      }
      
      // notify all threads to wake up
      mLockObject.lock();
      {
         mLockObject.notifyAll();
      }
      mLockObject.unlock();
   }
   unlock();
}

void Semaphore::addWaitingThread(Thread* thread)
{
   lock();
   {
      // if this thread isn't in the wait queue, add it
      list<Thread*>::iterator i =
         find(mWaitingThreads.begin(), mWaitingThreads.end(), thread);
      if(i == mWaitingThreads.end())
      {
         mWaitingThreads.push_back(thread);
      }
   }
   unlock();
}

void Semaphore::removeWaitingThread(Thread* thread)
{
   lock();
   {
      // remove thread
      mWaitingThreads.remove(thread);
   }
   unlock();
}

void Semaphore::removeFirstWaitingThread()
{
   lock();
   {
      if(mWaitingThreads.size() > 0)
      {
         // remove first thread
         mWaitingThreads.pop_front();
      }
   }
   unlock();
}

void Semaphore::removeRandomWaitingThread()
{
   lock();
   {
      if(mWaitingThreads.size() > 0)
      {
         // get a random index
         unsigned int index = rand() % mWaitingThreads.size();
         list<Thread*>::iterator i = mWaitingThreads.begin();
         for(unsigned int count = 0;
             count < index && i != mWaitingThreads.end(); i++, count++);
         
         // remove thread
         mWaitingThreads.erase(i);
      }
   }
   unlock();
}

bool Semaphore::mustWait(Thread* thread)
{
   bool rval = false;
   
   lock();
   {
      list<Thread*>::iterator i =
         find(mWaitingThreads.begin(), mWaitingThreads.end(), thread);
      if(i != mWaitingThreads.end())
      {
         // thread is in the wait list
         rval = true;
      }
   }
   unlock();
   
   return rval;
}

InterruptedException* Semaphore::acquire()
{
   return acquire(1);
}

InterruptedException* Semaphore::acquire(int permits)
{
   InterruptedException* rval = NULL;
   
   // while there are not enough permits, wait for them
   while(rval == NULL && availablePermits() - permits < 0)
   {
      // wait thread
      rval = waitThread();
   }
   
   if(rval == NULL)
   {
      // permits have been granted, decrease permits left
      decreasePermitsLeft(permits);
   }
   
   return rval;
}

bool Semaphore::tryAcquire()
{
   bool rval = false;
   
   lock();
   {
      rval = tryAcquire(1);
   }
   unlock();
   
   return rval;
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
   
      // if fair, notify in order, otherwise notify all
      for(int i = 0; i < permits; i++)
      {
         // notify threads
         notifyThreads();
      }
   }
   unlock();
   
   return rval;
}

int Semaphore::availablePermits()
{
   return mPermitsLeft;
}

int Semaphore::usedPermits()
{
   return mPermits - mPermitsLeft;
}

bool Semaphore::isFair()
{
   return mFair;
}

const list<Thread*>& Semaphore::getQueuedThreads()
{
   return mWaitingThreads;
}

int Semaphore::getQueueLength()
{
   int rval = 0;
   
   lock();
   {
      rval = mWaitingThreads.size();
   }
   unlock();
   
   return rval;
}

void Semaphore::setMaxPermitCount(int max)
{
   // store old permit count
   int oldPermitCount = getMaxPermitCount();
   
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

int Semaphore::getMaxPermitCount()
{
   return mPermits;
}
