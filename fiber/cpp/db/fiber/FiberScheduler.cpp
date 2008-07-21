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
                  fiber->setState(i->state);
               }
               break;
            case Fiber::Running:
               // illegal, ignore message
               break;
            case Fiber::Exiting:
               // exiting fiber always permitted
               fiber->setState(i->state);
               break;
            case Fiber::Sleeping:
               // only set sleeping if not exiting
               if(fiber->getState() == Fiber::Exiting)
               {
                  fiber->setState(i->state);
               }
               break;
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

void FiberScheduler::runNextFiber(bool yield)
{
   // lock to schedule next fiber
   mScheduleLock.lock();
   {
      // process messages
      processMessages();
      
      // cycle through the fibers looking for an idle one to run, do not
      // do more than one cycle:
      
      // restart fiber iterator as necessary
      if(mFiberItr == mFiberList.end())
      {
         mFiberItr = mFiberList.begin();
      }
      
      // initialize cycleEnd as invalid
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
            case Fiber::Exiting:
               // FIXME: uncomment (race condition bug seems to disappear
               // with this code uncommented)
               /*
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
               */
               // FIXME: remove nextFiber() when uncommenting above code
               nextFiber();
               
               // notify that a fiber has exited
               mScheduleLock.notifyAll();
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
         
         // notify that a fiber has gone idle
         mScheduleLock.notifyAll();
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

void FiberScheduler::sendStateMessage(FiberId id, Fiber::State state)
{
   // create FiberMessage to change state
   FiberMessage fm;
   fm.id = id;
   fm.state = state;
   fm.data.setNull();
   
   // lock to queue message
   mMessageQueueLock.lock();
   {
      // add message to the first queue
      mMessageQueues.front()->push_back(fm);
   }
   mMessageQueueLock.unlock();
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
   
   mScheduleLock.lock();
   {
      // wait on the schedule lock until there are no more fibers
      while(rval && !mFiberList.empty())
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
