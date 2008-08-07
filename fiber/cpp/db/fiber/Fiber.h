/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_fiber_Fiber_H
#define db_fiber_Fiber_H

#include "db/rt/DynamicObject.h"

#include <list>

namespace db
{
namespace fiber
{

// type definitions
typedef uint32_t FiberId;
typedef int16_t FiberPriority;

// forward declarations
class FiberScheduler;

/**
 * A Fiber is a single unit of execution. Fibers use cooperatively multitasking
 * to allow code to be run concurrently. Each Fiber is scheduled and run by
 * a FiberScheduler. Multiple threads can be used to run multiple Fibers, but
 * no Fiber will ever run concurrently with itself.
 * 
 * A Fiber will be continuously run by its FiberScheduler until it exits via a
 * call to exit(). This should be called from run(), processMessage(), or
 * interrupted().
 * 
 * A Fiber can be interrupted by calling interrupt() (or when it receives
 * an interrupted system message). If a Fiber is interrupted, then its
 * interrupted() call will be called each time it is scheduled, instead of
 * its run() method, even if it is in a sleep state. This is to allow fibers
 * to handle interrupts at any point. If a fiber calls resume(), its interrupted
 * state will be cleared. A fiber may also call exit() from its interrupted()
 * method.
 * 
 * Fibers can have priorities that a FiberScheduler can use to determine their
 * scheduling order.
 * 
 * There are several methods that are guaranteed to be run non-currently
 * with themselves and each other:
 * 
 * processMessage()
 * run()
 * interrupted()
 * exiting()
 * 
 * @author Dave Longley
 */
class Fiber
{
public:
   /**
    * A Fiber's execution state.
    */
   typedef uint8_t State;
   enum FiberStates
   {
      // states a fiber can be in
      None        = 0,
      Running     = 1 << 0,
      Sleeping    = 1 << 1,
      Interrupted = 1 << 2,
      Exiting     = 1 << 3,
      
      // special values to alter a fiber's state via system messages
      Wakeup      = 1 << 4,
      Resume      = 1 << 5,
      Dead        = 1 << 6
   };
   
protected:
   /**
    * The FiberScheduler in charge of scheduling and running this Fiber.
    */
   FiberScheduler* mScheduler;
   
   /**
    * The ID of this Fiber.
    */
   FiberId mId;
   
   /**
    * The current state of this Fiber.
    */
   State mState;
   
   /**
    * This Fiber's priority.
    */
   FiberPriority mPriority;
   
   /**
    * Typedef and a queue of deferred messages to process after running.
    */
   typedef std::list<db::rt::DynamicObject*> MessageQueue;
   MessageQueue mMessageQueue;
   
   /**
    * Yields this Fiber temporarily to allow another Fiber to run.
    * 
    * This method *must* only be called inside run().
    */
   virtual void yield();
   
   /**
    * Causes this Fiber to sleep. If called from a non-concurrent method, then
    * this Fiber is guaranteed to sleep() before run() is called again. If a
    * fiber is interrupted during a sleep, interrupted() will still be called.
    * 
    * Subsequent calls to wakeup() will change this.
    * 
    * The Fiber will remain asleep until it is woken up, interrupted, or exits.
    */
   virtual void sleep();
   
   /**
    * Returns true if this fiber is sleeping, false if not.
    * 
    * @return true if this fiber is sleeping, false if not.
    */
   virtual bool isSleeping();
   
   /**
    * Causes this Fiber to wakeup if it was sleeping.
    */
   virtual void wakeup();
   
   /**
    * Causes this Fiber to be interrupted. If called from a non-concurrent
    * method, then this Fiber is guaranteed to call interrupted() the next
    * time it is scheduled, rather than run(). Subsequent calls to resume()
    * will change this.
    * 
    * The Fiber will remain interrupted until it is resumed or exits.
    */
   virtual void interrupt();
   
   /**
    * Returns true if this fiber is interrupted.
    * 
    * @return true if this fiber is interrupted, false if not.
    */
   virtual bool isInterrupted();
   
   /**
    * Causes this Fiber to resume if it has been interrupted.
    */
   virtual void resume();
   
   /**
    * Causes this Fiber to exit. The Fiber is guaranteed to exit before
    * run() is called again.
    */
   virtual void exit();
   
   /**
    * Sends a message to another fiber using the same scheduler.
    * 
    * @param id the ID of the fiber to send the message to.
    * @param msg the message to send.
    */
   virtual void sendMessage(FiberId id, db::rt::DynamicObject& msg);
   
public:
   /**
    * Creates a new Fiber.
    */
   Fiber();
   
   /**
    * Destructs this Fiber.
    */
   virtual ~Fiber();
   
   /**
    * Runs this Fiber while it is not interrupted and not sleeping. If this
    * Fiber is interrupted, interrupted() will be called instead. If this
    * Fiber is sleeping, no call will be made.
    * 
    * This method is guaranteed to be a non-concurrent method as specified in
    * the description of the Fiber class.
    */
   virtual void run() = 0;
   
   /**
    * Called when this Fiber has been interrupted instead of run(). This method
    * should either cause the Fiber to exit or resume. It will be called
    * repeatedly instead of run() as long as this fiber is in an interrupted
    * state.
    * 
    * This method is guaranteed to be a non-concurrent method as specified in
    * the description of the Fiber class.
    */
   virtual void interrupted() {};
   
   /**
    * Called just prior to this Fiber's exit. One useful override for this
    * function is to send an event indicating that the Fiber has exited.
    * 
    * This method is guaranteed to be a non-concurrent method as specified in
    * the description of the Fiber class.
    */
   virtual void exiting() {};
   
   /**
    * Called *only* by a FiberScheduler to have this Fiber process the passed
    * message.
    * 
    * This method is guaranteed to be a non-concurrent method as specified in
    * the description of the Fiber class.
    * 
    * @param msg the message to process.
    */
   virtual void processMessage(db::rt::DynamicObject& msg) {};
   
   /**
    * Called *only* by a FiberScheduler to claim ownership of this Fiber.
    * 
    * @param scheduler the FiberScheduler that is claiming this Fiber.
    * @param id the FiberId assigned to this Fiber.
    */
   virtual void setScheduler(FiberScheduler* scheduler, FiberId id);
   
   /**
    * Adds a deferred message to be processed. A deferred message is a
    * custom message that could not be processed while the Fiber was
    * running, but will be once it completes.
    * 
    * @param msg the message to add.
    */
   virtual void addDeferredMessage(db::rt::DynamicObject* msg);
   
   /**
    * Processes all of this Fiber's messages that could not be processed
    * while it was running.
    */
   virtual void processDeferredMessages();
   
   /**
    * Gets this Fiber's ID, as assigned by its FiberScheduler.
    * 
    * @return this Fiber's ID.
    */
   virtual FiberId getId();
   
   /**
    * Called *only* by a FiberScheduler to set this Fiber's current state.
    * 
    * @param state the new state for this Fiber.
    */
   virtual void setState(State state);
   
   /**
    * Gets this Fiber's current state.
    * 
    * @return this Fiber's state.
    */
   virtual State getState();
   
   /**
    * Sets the priority of this Fiber.
    * 
    * @param p the new FiberPriority.
    */
   virtual void setPriority(FiberPriority p);
   
   /**
    * Gets this Fiber's priority.
    * 
    * @return this Fiber's priority.
    */
   virtual FiberPriority getPriority();
};

} // end namespace fiber
} // end namespace db
#endif
