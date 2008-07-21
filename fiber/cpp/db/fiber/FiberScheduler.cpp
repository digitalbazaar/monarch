/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/FiberScheduler.h"

using namespace db::fiber;
using namespace db::modest;
using namespace db::rt;

FiberScheduler::FiberScheduler()
{
}

FiberScheduler::~FiberScheduler()
{
   // ensure stopped
   stop();
   
   // delete all fibers
   for(FiberMap::iterator i = mFibers.begin(); i != mFibers.end(); i++)
   {
      delete i->second;
   }
   
   // delete all message queues
   for(MessageQueueList::iterator i = mMessageQueues.begin();
       i != mMessageQueues.end(); i++)
   {
      delete *i;
   }
}

void FiberScheduler::processMessages()
{
   MessageQueue* mq;
   
   // lock to acquire a message queue to process
   mMessageQueueLock.lock();
   {
      mq = mMessageQueues.front();
      mMessageQueues.pop_front();
   }
   mMessageQueueLock.unlock();
   
   // process messages
   FiberMap::iterator fi;
   for(MessageQueue::iterator i = mq->begin(); i != mq->end(); i++)
   {
      // get the Fiber that will process the message
      fi = mFibers.find(i->id);
      if(fi != mFibers.end())
      {
         if(i->state != Fiber::None)
         {
            // update the fiber's state
            fi->second->setState(i->state);
         }
         else
         {
            // process the message
            fi->second->processMessage(i->data);
         }
      }
   }
   
   // clear message queue
   mq->clear();
   
   // lock to put message queue back
   mMessageQueueLock.lock();
   {
      mMessageQueues.push_back(mq);
   }
   mMessageQueueLock.unlock();
}

void FiberScheduler::runNextFiber(bool yield)
{
   // lock to schedule next fiber
   mScheduleLock.lock();
   {
      // process messages
      processMessages();
      
      // iterate until an idle fiber is found or until 1 full cycle
      Fiber* fiber = NULL;
      FiberMap::iterator curr;
      FiberMap::iterator end = mFiberItr;
      do
      {
         // save current iterator
         curr = mFiberItr;
         
         // iterate to next fiber
         if(mFiberItr == mFibers.end())
         {
            mFiberItr = mFibers.begin();
         }
         else
         {
            mFiberItr++;
         }
         
         // check current iterator
         if(curr != mFibers.end())
         {
            if(curr->second->getState() == Fiber::Idle)
            {
               // idle fiber found
               fiber = curr->second;
            }
            // erase any exiting fiber
            else if(curr->second->getState() == Fiber::Exiting)
            {
               // add now-unused fiber ID to free list
               mFiberIdFreeList.push_front(curr->second->getId());
               
               // delete fiber and erase map entry
               delete curr->second;
               mFibers.erase(curr);
               
               // notify that a fiber has exited
               mScheduleLock.notifyAll();
            }
         }
      }
      while(mFiberItr != end && fiber == NULL);
      
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
      }
      else
      {
         // wait on schedule lock if not yielding
         if(!yield)
         {
            mScheduleLock.wait();
         }
      }
   }
   mScheduleLock.unlock();
}

void FiberScheduler::start(OperationRunner* opRunner, int numOps)
{
   // create "numOps" Operations and message queues
   for(int i = 0; i < numOps; i++)
   {
      // create Operation
      Operation op(*this);
      mOpList.add(op);
      
      if((int)mMessageQueues.size() < numOps)
      {
         // create message queue
         MessageQueue* mq = new MessageQueue();
         mMessageQueues.push_back(mq);
      }
   }
   
   // point to the first fiber
   mFiberItr = mFibers.begin();
   
   // add new id if list is empty
   if(mFiberIdFreeList.empty())
   {
      mFiberIdFreeList.push_back(1);
   }
   
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
   
   mScheduleLock.lock();
   {
      // wait on the schedule lock until there are no more fibers
      while(rval && !mFibers.empty())
      {
         rval = mScheduleLock.wait();
      }
   }
   mScheduleLock.unlock();
   
   if(rval)
   {
      // stop fiber scheduler
      stop();
   }
   
   return rval;
}

inline void FiberScheduler::next()
{
   runNextFiber(false);
}

inline void FiberScheduler::yield(FiberId id)
{
   runNextFiber(true);
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
      
      // add fiber to map
      mFibers[id] = fiber;
      
      // notify waiting threads of a new fiber
      mScheduleLock.notifyAll();
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
   
   // lock to queue message
   mMessageQueueLock.lock();
   {
      // add message to the first queue
      mMessageQueues.front()->push_back(fm);
   }
   mMessageQueueLock.unlock();
}

void FiberScheduler::wakeupFiber(FiberId id)
{
   // create FiberMessage to change state
   FiberMessage fm;
   fm.id = id;
   fm.state = Fiber::Idle;
   fm.data.setNull();
   
   // lock to queue message
   mMessageQueueLock.lock();
   {
      // add message to the first queue
      mMessageQueues.front()->push_back(fm);
   }
   mMessageQueueLock.unlock();
}

void FiberScheduler::run()
{
   // run scheduled fibers
   Thread* t = Thread::currentThread();
   while(!t->isInterrupted())
   {
      next();
   }
}
