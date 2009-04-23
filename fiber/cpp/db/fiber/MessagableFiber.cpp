/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/MessagableFiber.h"

#include "db/fiber/FiberScheduler.h"
#include "db/fiber/FiberMessageCenter.h"

using namespace db::fiber;
using namespace db::rt;

MessagableFiber::MessagableFiber(FiberMessageCenter* fmc, size_t stackSize) :
   Fiber(stackSize)
{
   mMessageCenter = fmc;
   mProcessingMessageQueue = &mMessageQueue1;
   mIncomingMessageQueue = &mMessageQueue2;
}

MessagableFiber::~MessagableFiber()
{
}

void MessagableFiber::run()
{
   // register with message center
   mMessageCenter->registerFiber(this);
   
   // process messages
   processMessages();
   
   // unregister with message center
   mMessageCenter->unregisterFiber(this);
}

void MessagableFiber::addMessage(db::rt::DynamicObject& msg)
{
   // lock to enqueue message
   mMessageLock.lock();
   {
      mIncomingMessageQueue->push_back(msg);
   }
   mMessageLock.unlock();
   
   // wake up self if sleeping
   mScheduler->wakeupSelf(this);
}

bool MessagableFiber::canSleep()
{
   bool rval;
   
   // lock to see if there are incoming messages
   mMessageLock.lock();
   {
      rval = mIncomingMessageQueue->empty();
   }
   mMessageLock.unlock();
   
   return rval;
}

FiberMessageQueue* MessagableFiber::getMessages()
{
   // lock to swap message queues
   mMessageLock.lock();
   {
      // clear previous processing message queue
      mProcessingMessageQueue->clear();
      
      if(mProcessingMessageQueue == &mMessageQueue1)
      {
         mProcessingMessageQueue = &mMessageQueue2;
         mIncomingMessageQueue = &mMessageQueue1;
      }
      else
      {
         mProcessingMessageQueue = &mMessageQueue1;
         mIncomingMessageQueue = &mMessageQueue2;
      }
   }
   mMessageLock.unlock();
   
   return mProcessingMessageQueue;
}

bool MessagableFiber::sendMessage(FiberId id, DynamicObject& msg)
{
   return mMessageCenter->sendMessage(id, msg);
}
