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
   
   // create message queues
   mSystemMessages = new MessageQueue;
   mCustomMessages = new MessageQueue;
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
   
   // delete unprocessed custom message data
   for(MessageQueue::iterator i = mCustomMessages->begin();
       i != mCustomMessages->end(); i++)
   {
      delete i->data;
   }
   
   // delete message queues
   delete mSystemMessages;
   delete mCustomMessages;
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

void FiberScheduler::noFibersAvailable()
{
   mNoFibersWaitLock.lock();
   mNoFibersWaitLock.notifyAll();
   mNoFibersWaitLock.unlock();
}

void FiberScheduler::queueMessage(FiberMessage& fm, bool system)
{
   // lock to queue message
   mMessageQueueLock.lock();
   {
      // add message to the appropriate queue
      system ? mSystemMessages->push_back(fm) : mCustomMessages->push_back(fm);
   }
   mMessageQueueLock.unlock();
   
   // notify that a message can be processed
   workAvailable();
}

void FiberScheduler::sendSystemMessage(FiberId id, Fiber::State state)
{
   // create FiberMessage to change state
   FiberMessage fm;
   fm.id = id;
   fm.state = state;
   queueMessage(fm, true);
}

void FiberScheduler::processCustomMessages()
{
   MessageQueue* mq;
   
   // lock to acquire custom message queue to process
   mMessageQueueLock.lock();
   {
      // point to custom messages, create new queue
      mq = mCustomMessages;
      mCustomMessages = new MessageQueue;
   }
   mMessageQueueLock.unlock();
   
   // process custom messages
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
         // defer the message if the fiber is running
         if(fiber->getState() & Fiber::Running)
         {
            fiber->addDeferredMessage(i->data);
         }
         else
         {
            // process the message and clean up
            fiber->processMessage(*i->data);
            delete i->data;
         }
      }
      else
      {
         // delete message data
         delete i->data;
      }
   }
   
   // delete old message queue
   delete mq;
}

bool FiberScheduler::processSystemMessages()
{
   bool rval = false;
   
   MessageQueue* mq;
   
   // lock to acquire system message queue to process
   mMessageQueueLock.lock();
   {
      // point to system messages, create new queue
      mq = mSystemMessages;
      mSystemMessages = new MessageQueue;
   }
   mMessageQueueLock.unlock();
   
   // process system messages, keep track of whether or not new work is found
   Fiber* fiber = NULL;
   Fiber::State state;
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
         // update the fiber's state
         state = fiber->getState();
         switch(i->state)
         {
            case Fiber::Exiting:
               // add state, fiber thread work may be available
               fiber->setState(state | i->state);
               rval = true;
               break;
            case Fiber::Sleeping:
               // add sleeping state, fiber thread work unchanged
               fiber->setState(state | i->state);
               break;
            case Fiber::Interrupted:
               // remove sleeping state, add interrupted state,
               // fiber thread work may be available
               fiber->setState((state & ~Fiber::Sleeping) | i->state);
               rval = true;
               break;
            case Fiber::Wakeup:
               // remove sleeping state, fiber thread work may be available
               fiber->setState(state & ~Fiber::Sleeping);
               rval = true;
               break;
            case Fiber::Resume:
               // remove interrupted state, fiber thread work unchanged
               fiber->setState(state & ~Fiber::Interrupted);
               break;
            case Fiber::Running:
               // illegal, ignore message
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
   
   // process custom messages
   processCustomMessages();
   
   // now process system messages which will include any system messages
   // sent when custom messages were processed (ensuring state changes
   // take place before run() is scheduled again for fibers)
   if(processSystemMessages())
   {
      // notify that work is available
      workAvailable();
   }
   
   // cycle through the fibers looking for an idle one to run, do not
   // do more than one cycle:
   
   // restart fiber iterator as necessary
   if(mFiberItr == mFiberList.end())
   {
      mFiberItr = mFiberList.begin();
   }
   
   // initialize cycleEnd as invalid
   Fiber* fiber = NULL;
   Fiber::State state;
   FiberId cycleEnd = 0;
   while(fiber == NULL && mFiberItr != mFiberList.end() &&
         (*mFiberItr)->getId() != cycleEnd)
   {
      // check fiber state (ignoring interrupted status)
      state = (*mFiberItr)->getState() & ~Fiber::Interrupted;
      
      // if fiber is NOT running and NOT sleeping and NOT exiting
      if(state == Fiber::None)
      {
         // idle fiber found, iterate for next cycle
         fiber = *mFiberItr;
         nextFiber();
      }
      // if fiber is NOT running and IS exiting
      else if(!(state & Fiber::Running) && (state & Fiber::Exiting))
      {
         // remove it
         removeFiber();
      }
      // else fiber must be running OR sleeping and not exiting
      // and is therefore not available to do work or exit
      else
      {
         if(cycleEnd == 0)
         {
            // set cycleEnd (do not check this fiber again this cycle)
            cycleEnd = (*mFiberItr)->getId();
         }
         
         // iterate to next fiber
         nextFiber();
      }
   }
   
   // see if a fiber was available
   if(fiber != NULL)
   {
      // determine if interrupting() or run() should be executed
      state = fiber->getState();
      bool interrupted = state & Fiber::Interrupted;
      
      // add running state to fiber
      fiber->setState(state | Fiber::Running);
      
      // unlock scheduler, run fiber, relock
      mScheduleLock.unlock();
      interrupted ? fiber->interrupted() : fiber->run();
      mScheduleLock.lock();
      
      // process any deferred messages
      fiber->processDeferredMessages();
      
      // remove running state from fiber
      fiber->setState(fiber->getState() & ~Fiber::Running);
      
      // notify that work is available
      workAvailable();
      
      // unlock scheduler
      mScheduleLock.unlock();
   }
   else
   {
      if(mFiberList.empty())
      {
         // notify that no fibers are available
         noFibersAvailable();
      }
      
      // unlock scheduler
      mScheduleLock.unlock();
      
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

bool FiberScheduler::waitForLastFiberExit(bool stop)
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
      // get available FiberId
      id = mFiberIdFreeList.front();
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
   
   return id;
}

void FiberScheduler::sendMessage(FiberId id, DynamicObject& msg)
{
   // create FiberMessage
   FiberMessage fm;
   fm.id = id;
   fm.data = new DynamicObject(msg);
   queueMessage(fm, false);
}

inline void FiberScheduler::yield(FiberId id)
{
   runNextFiber(true);
}

inline void FiberScheduler::exit(FiberId id)
{
   sendSystemMessage(id, Fiber::Exiting);
}

inline void FiberScheduler::sleep(FiberId id)
{
   sendSystemMessage(id, Fiber::Sleeping);
}

inline void FiberScheduler::wakeup(FiberId id)
{
   sendSystemMessage(id, Fiber::Wakeup);
}

inline void FiberScheduler::interrupt(FiberId id)
{
   sendSystemMessage(id, Fiber::Interrupted);
}

inline void FiberScheduler::resume(FiberId id)
{
   sendSystemMessage(id, Fiber::Resume);
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
