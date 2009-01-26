/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/MessagableFiber.h"

#include "db/fiber/FiberMessageCenter.h"

using namespace db::fiber;
using namespace db::rt;

MessagableFiber::MessagableFiber(FiberMessageCenter* fmc, size_t stackSize) :
   Fiber2(stackSize)
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
   
   bool quit = false;
   while(!quit)
   {
      // lock to swap message queues
      mMessageLock.lock();
      {
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
      
      // process messages
      quit = !processMessages(mProcessingMessageQueue);
      
      // clear processing message queue
      mProcessingMessageQueue->clear();
      
      // yield to allow other fibers to run
      yield();
   }
   
   // unregister with message center
   mMessageCenter->registerFiber(this);
}

void MessagableFiber::addMessage(db::rt::DynamicObject& msg)
{
   // lock to enqueue message
   mMessageLock.lock();
   {
      mIncomingMessageQueue->push_back(msg);
   }
   mMessageLock.unlock();
}

bool MessagableFiber::sendMessage(FiberId2 id, DynamicObject& msg)
{
   return mMessageCenter->sendMessage(id, msg);
}
