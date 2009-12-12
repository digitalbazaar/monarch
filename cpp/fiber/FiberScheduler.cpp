/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/fiber/FiberScheduler.h"

#include <algorithm>
#include <cstdio>
#include <setjmp.h>

using namespace std;
using namespace db::fiber;
using namespace db::modest;
using namespace db::rt;

// max fiber ID is MAX(uint32)
#define MAX_FIBER_ID 0xFFFFFFFF

FiberScheduler::FiberScheduler() :
   mNextFiberId(1),
   mCheckFiberMap(false)
{
}

FiberScheduler::~FiberScheduler()
{
   // ensure stopped
   FiberScheduler::stop();

   // delete all fibers
   for(FiberMap::iterator i = mFiberMap.begin(); i != mFiberMap.end(); i++)
   {
      i->second->setState(Fiber::Dead);
      delete i->second;
   }
   mFiberMap.clear();
}

void FiberScheduler::start(OperationRunner* opRunner, int numOps)
{
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

inline void FiberScheduler::stop()
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

bool FiberScheduler::waitForLastFiberExit(bool stop)
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

FiberId FiberScheduler::addFiber(Fiber* fiber)
{
   FiberId id;

   // lock scheduler to add fiber
   mScheduleLock.lock();
   {
      // get next available FiberId
      id = mNextFiberId++;
      if(mCheckFiberMap)
      {
         // Note: If there are MAX_FIBER_ID fibers concurrently in the system
         // then this code will lock forever... so don't do that.
         FiberMap::iterator i = mFiberMap.find(id);
         while(i != mFiberMap.end())
         {
            id = mNextFiberId++;
            if(id == MAX_FIBER_ID)
            {
               mNextFiberId = 1;
            }
            i = mFiberMap.find(id);
         }
      }
      if(id == MAX_FIBER_ID)
      {
         mNextFiberId = 1;
         mCheckFiberMap = true;
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

void FiberScheduler::run()
{
   // get and store scheduler context for this thread
   FiberContext* scheduler = new FiberContext();
   mScheduleLock.lock();
   {
      mContextList.push_back(scheduler);
   }
   mScheduleLock.unlock();

   // continue scheduling fibers while this thread is not interrupted
   bool tryInit = true;
   Thread* t = Thread::currentThread();
   while(!t->isInterrupted())
   {
      // synchronously get the next fiber to schedule
      Fiber* fiber = nextFiber();

      // if there is no fiber to schedule
      if(fiber == NULL)
      {
         // wait until one is available
         waitForFiber();
      }
      // else a fiber has been found
      else
      {
         // fiber's state can only be set to New if it
         // hasn't ever been run yet, so no need to lock here
         if(fiber->getState() == Fiber::New)
         {
            // initialize the fiber's context
            if(tryInit && fiber->getContext()->init(fiber))
            {
               // set fiber state to running
               fiber->setState(Fiber::Running);
            }
            else
            {
               // do not try init again until a fiber's stack is reclaimed
               tryInit = false;
            }
         }

         /*
         Note: The fiber's state *must* have been New or Running if it was
         selected via nextFiber. The fiber cannot be running currently.
         The only state the fiber can be set to outside of its own context
         is Waking. It could only have had its state set to Waking if its
         state was Sleeping, which is impossible given the first sentence.
         Therefore, the fiber's state will definitely be New if it failed
         to init.
         */
         if(fiber->getState() == Fiber::New)
         {
            // failed to init fiber, not enough memory, lock to re-queue it
            mScheduleLock.lock();
            {
               mFiberQueue.push_back(fiber);
            }
            mScheduleLock.unlock();
         }
         // fiber's state is Running
         else
         {
            // swap in the fiber's context
            scheduler->swap(fiber->getContext());

            // Note: Here the fiber's state could be changed externally
            // from Sleeping to Waking, so we must lock first to ensure
            // ensure we don't cause any evil race conditions.

            // lock scheduling while adding fiber back to queue
            mScheduleLock.lock();
            {
               if(fiber->getState() == Fiber::Sleeping)
               {
                  /*
                  Note: We must check the canSleep() method here and not
                  in the above sleep() call. We must do this because there
                  only two ways for a fiber to keep running after a call
                  to put it to sleep. The first is to wakeup the fiber.
                  However, if a fiber was going to sleep but hadn't been
                  swapped out yet, then a wakeup call will fail to wake
                  up the fiber (because while the fiber's state will have
                  been set to sleeping, it will not have entered the map
                  of sleeping fibers yet). Since a wakeup call may fail, the
                  only other way to keep a fiber running is for it to fail
                  its canSleep() call. A fiber-extending class can specify
                  the conditions underwhich a fiber can sleep in this method
                  to prevent it from sleeping if it may have missing that
                  critical wakeup() call. We make that call here, because
                  if we did it in the sleep() call it would have the same
                  potential problem as the wakeup() call: it could be missed
                  and cause the fiber to sleep indefinitely.

                  With this implementation, the user can safely implement
                  a canSleep() method that checks a condition (within a
                  mutex) that will be set elsewhere (within the mutex)
                  *followed by* a mutex-free wake up call. This
                  implementation prevents a deadlock scenario.
                  */

                  // only *actually* sleep fiber if it can be sleeped at
                  // the moment
                  if(fiber->canSleep())
                  {
                     // do not re-queue the fiber, instead add it to a map
                     // of sleeping fibers
                     mSleepingFibers.insert(make_pair(fiber->getId(), fiber));
                  }
                  else
                  {
                     // change fiber state back to running
                     fiber->setState(Fiber::Running);
                  }
               }

               if(fiber->getState() != Fiber::Sleeping)
               {
                  // if fiber is running, put it in the back of the queue
                  if(fiber->getState() == Fiber::Running)
                  {
                     mFiberQueue.push_back(fiber);
                  }
                  // fiber is dying, so put it in front for quicker cleanup
                  else if(fiber->getState() == Fiber::Exited)
                  {
                     mFiberQueue.push_front(fiber);

                     // because fiber's stack memory will be reclaimed, it
                     // is safe to try init on new fibers again
                     tryInit = true;
                  }

                  /*
                  Note: If the fiber's state is Waking, we don't add it
                  to the queue as this was done when its state was changed
                  to Waking. Also, it is worth noting that its state cannot
                  be changed back to Sleeping until the fiber runs again
                  in its own context, so there are no race conditions
                  concerning the fiber queue.
                  */

                  // notify that a fiber is available
                  fiberAvailable();
               }
            }
            mScheduleLock.unlock();
         }
      }
   }
}

void FiberScheduler::yield(Fiber* fiber)
{
   // swap scheduler back in
   fiber->getContext()->swapBack();
}

void FiberScheduler::sleep(Fiber* fiber)
{
   // lock scheduling to insert sleeping fiber entry
   mScheduleLock.lock();
   {
      /*
      Note: Simply set the fiber's state here. The fiber will be added
      to a map of sleeping fibers, if it "canSleep()" once it is swapped
      out. That map insert must occur after swapping the fiber out to
      prevent a race condition where the fiber can be double-scheduled --
      which could occur if the fiber was insert into the sleep map here,
      then wakeup() was called immediately causing the fiber to be queued
      for scheduling before it was swapped out -- and then another
      scheduling thread could run the fiber concurrently causing evil
      havok.
      */
      fiber->setState(Fiber::Sleeping);
   }
   mScheduleLock.unlock();

   // swap scheduler back in
   fiber->getContext()->swapBack();
}

void FiberScheduler::wakeupSelf(Fiber* fiber)
{
   // lock scheduling while waking up sleeping fiber
   mScheduleLock.lock();
   {
      // only *actually* wake up self if sleeping
      if(fiber->getState() == Fiber::Sleeping)
      {
         wakeup(fiber->getId());
      }
   }
   mScheduleLock.unlock();
}

void FiberScheduler::wakeup(FiberId id)
{
   // lock scheduling while waking up sleeping fiber
   mScheduleLock.lock();
   {
      // find the sleeping fiber
      FiberMap::iterator i = mSleepingFibers.find(id);
      if(i != mSleepingFibers.end())
      {
         /*
         Note: Here we must set a special Waking state for the fiber. This
         is because a fiber may have just put itself to sleep and the
         scheduler's context may have been swapped in in another thread.
         Since the scheduler will be blocked until we return here, we will
         have already added the fiber back into the fiber queue -- and if
         we were to simply set a state of Running, then once the scheduler
         gets unblocked it would also add the fiber into the fiber queue,
         causing some serious evil. Instead, we set a Waking state and
         allow the scheduler to convert Waking state fibers back into
         Running state fibers once they are found during the scheduling
         process. Also, it is worth noting that a fiber's state can only
         be set to Sleeping when we are inside of a fiber's context, so
         we needn't worry about a similar (but reverse) situation occurring
         there.
         */

         // update fiber state, add to queue, remove from sleeping fibers map
         i->second->setState(Fiber::Waking);
         mFiberQueue.push_back(i->second);
         mSleepingFibers.erase(i);

         // notify that a fiber is available
         fiberAvailable();
      }
   }
   mScheduleLock.unlock();
}

void FiberScheduler::exit(Fiber* fiber)
{
   /*
   Note: No need to lock to modify state here, it could only be changed to
   Waking and only if the state was Sleeping, which it can't be since
   the state *had* to be Running since we are exiting from within the
   fiber's context.
   */
   fiber->setState(Fiber::Exited);

   // load scheduler back in
   fiber->getContext()->loadBack();
}

Fiber* FiberScheduler::nextFiber()
{
   Fiber* rval = NULL;

   // lock scheduling while iterating over and modifying fiber queue
   mScheduleLock.lock();
   {
      // look for a fiber that can be scheduled:

      // cycle through fibers not more than once
      FiberId cycleEnd = 0;
      for(FiberQueue::iterator i = mFiberQueue.begin();
          rval == NULL && i != mFiberQueue.end() && (*i)->getId() != cycleEnd;)
      {
         // pop fiber off queue
         Fiber* fiber = *i;
         i = mFiberQueue.erase(i);

         // check state of fiber
         switch(fiber->getState())
         {
            // if a fiber is waking, set it to running and schedule it
            case Fiber::Waking:
               fiber->setState(Fiber::Running);
            // if fiber is new or running, it can be scheduled
            case Fiber::New:
            case Fiber::Running:
               rval = fiber;
               break;
            // exited or dead fibers must be deleted
            case Fiber::Exited:
               fiber->setState(Fiber::Dead);
            case Fiber::Dead:
               // remove fiber from scheduler
               mFiberMap.erase(fiber->getId());
               delete fiber;
               fiber = NULL;
               break;
            // a sleeping fiber should *NEVER* be in the queue
            case Fiber::Sleeping:
               fprintf(stderr,
                  "A sleeping fiber was scheduled to run, which should "
                  "never *ever* happen. The fiber code is broken.\n");
               ::exit(1);
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

inline void FiberScheduler::fiberAvailable()
{
   mFiberWaitLock.lock();
   mFiberWaitLock.notifyAll();
   mFiberWaitLock.unlock();
}

inline void FiberScheduler::waitForFiber()
{
   mFiberWaitLock.lock();
   mFiberWaitLock.wait();
   mFiberWaitLock.unlock();
}

inline void FiberScheduler::noFibersAvailable()
{
   mNoFibersWaitLock.lock();
   mNoFibersWaitLock.notifyAll();
   mNoFibersWaitLock.unlock();
}
