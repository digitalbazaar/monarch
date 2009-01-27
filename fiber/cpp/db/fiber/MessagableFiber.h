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
 * A MessagableFiber is a Fiber that can receive messages. The Fiber processes
 * messages and performs whatever custom work is necessary inside of the
 * processMessages() function. Once that function returns, the Fiber will
 * exit.
 * 
 * A MessagableFiber can only sleep when it has no incoming messages. If a
 * MessagableFiber is asleep and it receives a new message, it will wake up.
 * 
 * A useful programming design for a MessagableFiber, therefore, is to do
 * something that will result in a message being sent back to the fiber at
 * a later time, and then sleep. After sleep() returns, getMessages() can
 * be called to handle any messages that accumulated while the fiber was
 * asleep.
 * 
 * For example:
 * 
 * bool processMessages()
 * {
 *    createAnotherFiberThatWillMessageThisOneWhenItFinishes();
 *    sleep();
 *    
 *    FiberMessageQueue* queue = getMessages();
 *    while(!queue->isEmpty())
 *    {
 *       DynamicObject msg = queue->front();
 *       queue->pop_front();
 *       
 *       // handle message
 *    }
 *    
 *    return false;
 * }
 * 
 * @author Dave Longley
 */
class MessagableFiber : public Fiber2
{
protected:
   /**
    * The FiberMessageCenter this MessagableFiber is registered with.
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
    * Registers this fiber with its FiberMessageCenter, runs processMessages(),
    * and then unregisters from the FiberMessageCenter. The run() method should
    * not be overridden by extending classes unless you know what you're doing,
    * instead processMessages() should be implemented.
    */
   virtual void run();
   
   /**
    * Called by a MessageCenter to add a message to this fiber.
    * 
    * @param msg the message to add.
    */
   virtual void addMessage(db::rt::DynamicObject& msg);
   
   /**
    * Determines whether or not a fiber is capable of sleeping when a
    * particular call to sleep is made. This method will be called by this
    * fiber's scheduler, inside of its scheduler lock, right before sleeping
    * this fiber. Any call to wake up this fiber will be blocked while this
    * method is running.
    * 
    * If this method returns false, the fiber will not be sleeped, but its
    * context will still be swapped out.
    * 
    * A MessagableFiber can only be put to sleep when it has no incoming
    * messages.
    * 
    * @return true if this fiber can be put to sleep at the moment, false
    *         if not.
    */
   virtual bool canSleep();
   
protected:
   /**
    * Swaps the internal message queues and returns a queue with the latest
    * messages so they can be processed. Any messages left in the previous
    * queue will be cleared. This should be called from processMessages to
    * retrieve the most recent messages. This method can be called as many
    * times as necessary from processMessages.
    * 
    * @return the processing message queue.
    */
   virtual FiberMessageQueue* getMessages();
   
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
    * Processes messages, retrieved via getMessages(), and performs whatever
    * custom work is necessary.
    */
   virtual void processMessages() = 0;
};

} // end namespace fiber
} // end namespace db
#endif
