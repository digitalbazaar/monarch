/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/Semaphore.h"
#include "db/rt/Thread.h"

#include <cstdlib>

using namespace std;
using namespace db::rt;

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

void Semaphore::decreasePermitsLeft(int decrease)
{
   mPermitsLeft -= decrease;
}

InterruptedException* Semaphore::waitThread(Thread* t)
{
   InterruptedException* rval = NULL;
   
   // add waiting thread
   addWaitingThread(t);
   
   // wait while not interrupted and in the list of waiting threads
   while(rval == NULL && mustWait(t))
   {
      if((rval = wait()) != NULL)
      {
         // thread has been interrupted, so notify other waiting
         // threads and remove this thread from the wait list
         notifyThreads();
         removeWaitingThread(t);
      }
   }
   
   return rval;
}

void Semaphore::notifyThreads()
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
   notifyAll();
}

void Semaphore::addWaitingThread(Thread* thread)
{
   // if this thread isn't in the wait queue, add it
   list<Thread*>::iterator i =
      find(mWaitingThreads.begin(), mWaitingThreads.end(), thread);
   if(i == mWaitingThreads.end())
   {
      mWaitingThreads.push_back(thread);
   }
}

void Semaphore::removeWaitingThread(Thread* thread)
{
   // remove thread
   mWaitingThreads.remove(thread);
}

void Semaphore::removeFirstWaitingThread()
{
   if(!mWaitingThreads.empty())
   {
      // remove first thread
      mWaitingThreads.pop_front();
   }
}

void Semaphore::removeRandomWaitingThread()
{
   if(!mWaitingThreads.empty())
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

bool Semaphore::mustWait(Thread* thread)
{
   bool rval = false;
   
   list<Thread*>::iterator i =
      find(mWaitingThreads.begin(), mWaitingThreads.end(), thread);
   if(i != mWaitingThreads.end())
   {
      // thread is in the wait list
      rval = true;
   }
   
   return rval;
}

InterruptedException* Semaphore::acquire()
{
   return acquire(1);
}

InterruptedException* Semaphore::acquire(int permits)
{
   InterruptedException* rval = NULL;
   
   lock();
   {
      // see if enough permits are available
      Thread* t = Thread::currentThread();
      while(rval == NULL && availablePermits() - permits < 0)
      {
         // must wait for permits
         rval = waitThread(t);
      }
      
      if(rval == NULL)
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
      
      // notify threads for number of permits
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
   return mWaitingThreads.size();
}

void Semaphore::setMaxPermitCount(int max)
{
   lock();
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
   unlock();
}

int Semaphore::getMaxPermitCount()
{
   return mPermits;
}
