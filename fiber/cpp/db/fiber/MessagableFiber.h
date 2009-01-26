/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_fiber_MessagableFiber_H
#define db_fiber_MessagableFiber_H

#include "db/fiber/Fiber2.h"
#include "db/rt/DynamicObject.h"

#include <list>

namespace db
{
namespace fiber
{

class FiberMessageCenter;

// typedef for a queue of fiber messages
typedef std::list<db::rt::DynamicObject> FiberMessageQueue;

/**
 * A MessagableFiber is a Fiber that can receive messages. The Fiber runs on
 * in a loop, receiving its latest messages in a queue on each iteration. If
 * the loop returns false, then the Fiber will exit.
 * 
 * @author Dave Longley
 */
class MessagableFiber : public Fiber2
{
protected:
   /**
    * The FiberMessageCenter this MessageFiber is registered with.
    */
   FiberMessageCenter* mMessageCenter;
   
   /**
    * A lock for manipulating this fiber's message queues.
    */
   db::rt::ExclusiveLock mMessageLock;
   
   /**
    * Two message queues that are swapped with one another appropriately. At
    * any given time, one queue holds messages that are to be processed while
    * the other stores new incoming messages.
    */
   FiberMessageQueue mMessageQueue1;
   FiberMessageQueue mMessageQueue2;
   
   /**
    * A pointer to the current processing message queue.
    */
   FiberMessageQueue* mProcessingMessageQueue;
   
   /**
    * A pointer to the current incoming message queue.
    */
   FiberMessageQueue* mIncomingMessageQueue;
   
public:
   /**
    * Creates a new MessagableFiber.
    * 
    * @param fmc the FiberMessageCenter to register with.
    * @param stackSize the stack size to use in bytes, 0 for the default.
    */
   MessagableFiber(FiberMessageCenter* fmc, size_t stackSize = 0);
   
   /**
    * Destructs this MessagableFiber.
    */
   virtual ~MessagableFiber();
   
   /**
    * Runs the main loop for this fiber. The loop will continually call
    * processMessages, delivering any messages that have been queued up since
    * the last call. The run() method should not be overridden by extending
    * classes unless you know what you're doing, instead processMessages
    * should be implemented.
    */
   virtual void run();
   
   /**
    * Called by a MessageCenter to add a message to this fiber.
    * 
    * @param msg the message to add.
    */
   virtual void addMessage(db::rt::DynamicObject& msg);
   
protected:
   /**
    * Sends a message to another fiber.
    * 
    * @param id the ID of the fiber.
    * @param msg the message to send.
    * 
    * @return true if the message was delivered, false if no such fiber exists.
    */
   virtual bool sendMessage(FiberId2 id, db::rt::DynamicObject& msg);
   
   /**
    * Processes the passed messages and performs whatever custom work is
    * necessary. Whatever messages are not removed from the queue after
    * this method returns will be erased. 
    * 
    * @param msgs the queue of messages to process.
    * 
    * @return true to continue processing messages, false to exit the fiber.
    */
   virtual bool processMessages(FiberMessageQueue* msgs) = 0;
};

} // end namespace fiber
} // end namespace db
#endif
