/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/FiberScheduler.h"

using namespace db::fiber;
using namespace db::modest;
using namespace db::rt;

FiberScheduler::FiberScheduler()
{
   // add first FiberId
   mFiberIdFreeList.push_back(1);
   
   // create message queue
   mMessageQueue = new MessageQueue;
}

FiberScheduler::~FiberScheduler()
{
   // ensure stopped
   stop();
   
   // delete all fibers
   for(FiberList::iterator i = mFiberList.begin(); i != mFiberList.end(); i++)
   {
      delete *i;
   }
   
   // delete message queue
   delete mMessageQueue;
}

void FiberScheduler::workAvailable()
{
   mWorkWaitLock.lock();   
   mWorkWaitLock.notifyAll();
   mWorkWaitLock.unlock();
}

void FiberScheduler::waitForWork()
{
   mWorkWaitLock.lock();   
   mWorkWaitLock.wait();
   mWorkWaitLock.unlock();
}

void FiberScheduler::queueMessage(FiberMessage& fm)
{
   // lock to queue message
   mMessageQueueLock.lock();
   {
      // add message to the queue
      mMessageQueue->push_back(fm);
   }
   mMessageQueueLock.unlock();
   
   // notify that a message can be processed
   workAvailable();
}

void FiberScheduler::sendStateMessage(FiberId id, Fiber::State state)
{
   // create FiberMessage to change state
   FiberMessage fm;
   fm.id = id;
   fm.state = state;
   fm.data.setNull();
   queueMessage(fm);
}

bool FiberScheduler::processMessages()
{
   bool rval = false;
   
   MessageQueue* mq;
   
   // lock to acquire a message queue to process
   mMessageQueueLock.lock();
   {
      // point to messages, create new queue
      mq = mMessageQueue;
      mMessageQueue = new MessageQueue;
   }
   mMessageQueueLock.unlock();
   
   // process messages, keep track of whether or not new work is found
   Fiber* fiber = NULL;
   FiberMap::iterator fi;
   for(MessageQueue::iterator i = mq->begin(); i != mq->end(); i++)
   {
      // find the fiber the message is for
      if(fiber == NULL || fiber->getId() != i->id)
      {
         fi = mFiberMap.find(i->id);
         fiber = (fi != mFiberMap.end() ? fi->second : NULL);
      }
      
      if(fiber != NULL)
      {
         // process the message OR update the fiber's state
         switch(i->state)
         {
            case Fiber::None:
               // no state change, so process the message 
               fiber->processMessage(i->data);
               break;
            case Fiber::Idle:
               // only set to idle if sleeping
               if(fiber->getState() == Fiber::Sleeping)
               {
                  fiber->setState(Fiber::Idle);
                  rval = true;
               }
               break;
            case Fiber::Running:
               // illegal, ignore message
               break;
            case Fiber::Exiting:
            case Fiber::Exited:
               // change to exiting if running, else change to exited
               fiber->setState(fiber->getState() == Fiber::Running ?
                  Fiber::Exiting : Fiber::Exited);
               rval = true;
               break;
            case Fiber::Sleeping:
               // only set sleeping if not exiting or exited
               if(fiber->getState() != Fiber::Exiting &&
                  fiber->getState() != Fiber::Exited)
               {
                  fiber->setState(Fiber::Sleeping);
               }
               break;
         }
      }
   }
   
   // delete old message queue
   delete mq;
   
   return rval;
}

void FiberScheduler::nextFiber()
{
   if(mFiberItr == mFiberList.end())
   {
      mFiberItr = mFiberList.begin();
   }
   else
   {
      mFiberItr++;
   }
}

void FiberScheduler::removeFiber()
{
   // call fiber's exiting function
   (*mFiberItr)->exiting();
   
   // add fiber ID to free list
   mFiberIdFreeList.push_front((*mFiberItr)->getId());
   
   // erase map entry, delete fiber, erase list entry
   mFiberMap.erase((*mFiberItr)->getId());
   delete *mFiberItr;
   mFiberItr = mFiberList.erase(mFiberItr);
   
   // restart fiber iterator as necessary
   if(mFiberItr == mFiberList.end())
   {
      mFiberItr = mFiberList.begin();
   }
}

void FiberScheduler::runNextFiber(bool yield)
{
   // lock to schedule next fiber
   mScheduleLock.lock();
   
   // process messages
   bool workFound = processMessages();
   
   // cycle through the fibers looking for an idle one to run, do not
   // do more than one cycle:
   
   // restart fiber iterator as necessary
   if(mFiberItr == mFiberList.end())
   {
      mFiberItr = mFiberList.begin();
   }
   
   // initialize cycleEnd as invalid, keep track of whether or not new work
   // is found
   Fiber* fiber = NULL;
   FiberId cycleEnd = 0;
   while(fiber == NULL && mFiberItr != mFiberList.end() &&
         (*mFiberItr)->getId() != cycleEnd)
   {
      // check fiber state
      switch((*mFiberItr)->getState())
      {
         case Fiber::Idle:
            // idle fiber found, iterate for next cycle
            fiber = *mFiberItr;
            nextFiber();
            break;
         case Fiber::Exited:
            // remove fiber
            removeFiber();
            workFound = true;
            break;
         default:
            if(cycleEnd == 0)
            {
               // set cycleEnd (do not check this fiber again this cycle)
               cycleEnd = (*mFiberItr)->getId();
            }
            
            // iterate to next fiber
            nextFiber();
            break;
      }
   }
   
   if(workFound)
   {
      // notify that work is available
      workAvailable();
   }
   
   // see if a fiber was available
   if(fiber != NULL)
   {
      // update fiber state
      fiber->setState(Fiber::Running);
      
      // unlock scheduler, run fiber, relock
      mScheduleLock.unlock();
      fiber->run();
      mScheduleLock.lock();
      
      // mark fiber as idle if its state is Running
      if(fiber->getState() == Fiber::Running)
      {
         fiber->setState(Fiber::Idle);
      }
      // mark fiber as exited if it is exiting
      else if(fiber->getState() == Fiber::Exiting)
      {
         fiber->setState(Fiber::Exited);
      }
      
      // notify that work is available
      workAvailable();
      
      // unlock scheduler
      mScheduleLock.unlock();
   }
   else
   {
      // unlock scheduler
      mScheduleLock.unlock();
      
      if(mFiberList.empty())
      {
         mNoFibersWaitLock.lock();
         {
            // notify that no fibers are available
            mNoFibersWaitLock.notifyAll();
         }
         mNoFibersWaitLock.unlock();
      }
      
      if(!yield)
      {
         // wait for work if not yielding
         waitForWork();
      }
   }
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
   
   // initialize fiber iterator as invalid
   mFiberItr = mFiberList.end();
   
   // queue Operations
   mOpList.queue(opRunner);
}

inline void FiberScheduler::stop()
{
   // terminate all Operations
   mOpList.terminate();
}

bool FiberScheduler::stopOnLastFiberExit()
{
   bool rval = true;
   
   mNoFibersWaitLock.lock();
   {
      // wait on the no fibers lock until there are no more fibers
      while(rval && !mFiberList.empty())
      {
         rval = mNoFibersWaitLock.wait();
      }
   }
   mNoFibersWaitLock.unlock();
   
   if(rval)
   {
      // stop fiber scheduler
      stop();
   }
   
   return rval;
}

void FiberScheduler::addFiber(Fiber* fiber)
{
   // lock scheduler to add fiber
   mScheduleLock.lock();
   {
      // get available FiberId
      FiberId id = mFiberIdFreeList.front();
      mFiberIdFreeList.pop_front();
      
      // add new id if list is empty
      if(mFiberIdFreeList.empty())
      {
         mFiberIdFreeList.push_back(id + 1);
      }
      
      // assign scheduler and id to fiber
      fiber->setScheduler(this, id);
      
      // add fiber to map and list
      mFiberMap.insert(std::make_pair(id, fiber));
      mFiberList.push_back(fiber);
      
      // notify that work is available
      workAvailable();
   }
   mScheduleLock.unlock();
}

void FiberScheduler::sendMessage(FiberId id, DynamicObject& msg)
{
   // create FiberMessage
   FiberMessage fm;
   fm.id = id;
   fm.state = Fiber::None;
   fm.data = msg;
   queueMessage(fm);
}

inline void FiberScheduler::yield(FiberId id)
{
   runNextFiber(true);
}

inline void FiberScheduler::exit(FiberId id)
{
   // send state message
   sendStateMessage(id, Fiber::Exiting);
}

inline void FiberScheduler::sleep(FiberId id)
{
   // send state message
   sendStateMessage(id, Fiber::Sleeping);
}

inline void FiberScheduler::wakeup(FiberId id)
{
   // send state message
   sendStateMessage(id, Fiber::Idle);
}

void FiberScheduler::run()
{
   // run scheduled fibers
   Thread* t = Thread::currentThread();
   while(!t->isInterrupted())
   {
      runNextFiber(false);
   }
}
