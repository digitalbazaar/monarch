/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/FiberScheduler2.h"

#include <algorithm>
#include <setjmp.h>

using namespace std;
using namespace db::fiber;
using namespace db::modest;
using namespace db::rt;

#define DEFAULT_STACK_SIZE 0x1000 // 32k

FiberScheduler2::FiberScheduler2()
{
   // add first FiberId
   mFiberIdFreeList.push_back(1);
}

FiberScheduler2::~FiberScheduler2()
{
   // ensure stopped
   FiberScheduler2::stop();
   
   // delete all fibers
   for(FiberMap::iterator i = mFiberMap.begin(); i != mFiberMap.end(); i++)
   {
      i->second->setState(Fiber2::Dead);
      delete i->second;
   }
   mFiberMap.clear();
}

void FiberScheduler2::start(
   OperationRunner* opRunner, int numOps, size_t fiberStackSize)
{
   // save stack size
   mFiberStackSize =
      (fiberStackSize == 0 ? DEFAULT_STACK_SIZE : fiberStackSize);
   
   // create "numOps" Operations
   for(int i = 0; i < numOps; i++)
   {
      // create Operation
      Operation op(*this);
      mOpList.add(op);
   }
   
   // queue Operations
   mOpList.queue(opRunner);
}

inline void FiberScheduler2::stop()
{
   // terminate all operations
   mOpList.terminate();
   
   // delete all thread contexts
   for(ContextList::iterator i = mContextList.begin();
       i != mContextList.end(); i++)
   {
      delete *i;
   }
   mContextList.clear();
}

bool FiberScheduler2::waitForLastFiberExit(bool stop)
{
   bool rval = true;
   
   mNoFibersWaitLock.lock();
   {
      // wait on the no fibers lock until there are no more fibers
      while(rval && !mFiberMap.empty())
      {
         rval = mNoFibersWaitLock.wait();
      }
   }
   mNoFibersWaitLock.unlock();
   
   if(rval && stop)
   {
      // stop fiber scheduler
      this->stop();
   }
   
   return rval;
}

FiberId2 FiberScheduler2::addFiber(Fiber2* fiber)
{
   FiberId2 id;
   
   // lock scheduler to add fiber
   mScheduleLock.lock();
   {
      // get available FiberId
      id = mFiberIdFreeList.front();
      mFiberIdFreeList.pop_front();
      
      // add new id if list is empty
      if(mFiberIdFreeList.empty())
      {
         mFiberIdFreeList.push_back(id + 1);
      }
      
      // assign id and scheduler to fiber
      fiber->setScheduler(id, this);
      
      // add fiber to map and queue
      mFiberMap.insert(make_pair(id, fiber));
      mFiberQueue.push_back(fiber);
      
      // notify that a fiber is available for scheduling
      fiberAvailable();
   }
   mScheduleLock.unlock();
   
   return id;
}

void FiberScheduler2::run()
{
   // get and store scheduler context for this thread
   FiberContext* scheduler = new FiberContext();
   mScheduleLock.lock();
   {
      mContextList.push_back(scheduler);
   }
   mScheduleLock.unlock();
   
   // continue scheduling fibers while this thread is not interrupted
   Thread* t = Thread::currentThread();
   while(!t->isInterrupted())
   {
      // synchronously get the next fiber to schedule
      Fiber2* fiber = nextFiber();
      
      // if there is no fiber to schedule
      if(fiber == NULL)
      {
         // wait until one is available
         waitForFiber();
      }
      // else a fiber has been found
      else
      {
         if(fiber->getState() == Fiber2::New)
         {
            // initialize the fiber's context
            if(fiber->getContext()->init(fiber, mFiberStackSize))
            {
               // set fiber state to running
               fiber->setState(Fiber2::Running);
            }
            else
            {
               // failed to init fiber, not enough memory, lock to re-queue it
               mScheduleLock.lock();
               {
                  mFiberQueue.push_back(fiber);
               }
               mScheduleLock.unlock();
            }
         }
         
         if(fiber->getState() == Fiber2::Running)
         {
            // swap in the fiber's context
            scheduler->swap(fiber->getContext());
            
            if(fiber->getState() != Fiber2::Sleeping)
            {
               // lock scheduling while adding fiber back to queue
               mScheduleLock.lock();
               {
                  mFiberQueue.push_back(fiber);
               }
               mScheduleLock.unlock();
               
               // notify that a fiber is available
               fiberAvailable();
            }
         }
      }
   }
}

void FiberScheduler2::yield(Fiber2* fiber)
{
   // swap scheduler back in
   fiber->getContext()->swapBack();
}

void FiberScheduler2::sleep(Fiber2* fiber)
{
   fiber->setState(Fiber2::Sleeping);
   
   // lock scheduling to insert sleeping fiber entry
   mScheduleLock.lock();
   {
      mSleepingFibers.insert(make_pair(fiber->getId(), fiber));      
   }
   mScheduleLock.unlock();
   
   // swap scheduler back in
   fiber->getContext()->swapBack();
}

void FiberScheduler2::wakeup(FiberId2 id)
{
   // lock scheduling while waking up sleeping fiber
   mScheduleLock.lock();
   {
      // find the sleeping fiber
      FiberMap::iterator i = mSleepingFibers.find(id);
      if(i != mSleepingFibers.end())
      {
         // update fiber state, add to queue, remove from sleeping fibers map
         i->second->setState(Fiber2::Running);
         mFiberQueue.push_back(i->second);
         mSleepingFibers.erase(i);
         
         // notify that a fiber is available
         fiberAvailable();
      }
   }
   mScheduleLock.unlock();
}

void FiberScheduler2::exit(Fiber2* fiber)
{
   fiber->setState(Fiber2::Exited);
   
   // load scheduler back in
   fiber->getContext()->loadBack();
}

Fiber2* FiberScheduler2::nextFiber()
{
   Fiber2* rval = NULL;
   
   // lock scheduling while iterating over and modifying fiber queue
   mScheduleLock.lock();
   {
      // look for a fiber that can be scheduled:
      
      // cycle through fibers not more than once
      FiberId2 cycleEnd = 0;
      for(FiberQueue::iterator i = mFiberQueue.begin();
          rval == NULL && i != mFiberQueue.end() && (*i)->getId() != cycleEnd;)
      {
         // pop fiber off queue
         Fiber2* fiber = *i;
         i = mFiberQueue.erase(i);
         
         // check state of fiber
         switch(fiber->getState())
         {
            // if fiber is new or running, it can be scheduled
            case Fiber2::New:
            case Fiber2::Running:
               rval = fiber;
               break;
               // exited or dead fibers must be deleted
            case Fiber2::Exited:
               fiber->setState(Fiber2::Dead);
            case Fiber2::Dead:
               // add fiber ID to free list, remove fiber from scheduler
               mFiberIdFreeList.push_front(fiber->getId());
               mFiberMap.erase(fiber->getId());
               delete fiber;
               fiber = NULL;
               break;
               // sleeping fibers cannot be scheduled
            case Fiber2::Sleeping:
               break;
         }
         
         // if current fiber cannot be scheduled
         if(rval == NULL && fiber != NULL)
         {
            // push fiber onto end of queue
            mFiberQueue.push_back(fiber);
            
            // ensure current fiber is not checked again
            if(cycleEnd == 0)
            {
               cycleEnd = fiber->getId();
            }
         }
      }
      
      if(mFiberMap.empty())
      {
         // notify that no fibers are available
         noFibersAvailable();
      }
   }
   mScheduleLock.unlock();
   
   return rval;
}

inline void FiberScheduler2::fiberAvailable()
{
   mFiberWaitLock.lock();
   mFiberWaitLock.notifyAll();
   mFiberWaitLock.unlock();
}

inline void FiberScheduler2::waitForFiber()
{
   mFiberWaitLock.lock();
   mFiberWaitLock.wait();
   mFiberWaitLock.unlock();
}

inline void FiberScheduler2::noFibersAvailable()
{
   mNoFibersWaitLock.lock();
   mNoFibersWaitLock.notifyAll();
   mNoFibersWaitLock.unlock();
}
