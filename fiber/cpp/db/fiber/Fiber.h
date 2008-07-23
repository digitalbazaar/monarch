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
 * call to exit(). This call can be made in its run() method or in its
 * processMessage() method.
 * 
 * Fibers can have priorities that a FiberScheduler can use to determine their
 * scheduling order.
 * 
 * @author Dave Longley
 */
class Fiber
{
public:
   /**
    * A Fiber's execution state.
    */
   enum State
   {
      None, Idle, Running, Sleeping, Exiting, Exited
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
   typedef std::list<db::rt::DynamicObject> MessageQueue;
   MessageQueue mMessageQueue;
   
   /**
    * Yields this Fiber temporarily to allow another Fiber to run.
    * 
    * This method *must* only be called inside run().
    */
   virtual void yield();
   
   /**
    * Causes this Fiber to exit. If exit() is called from processMessage(),
    * then the Fiber may be scheduled again before it exits. If it is called
    * from run(), then it is guaranteed to exit before run() is called again.
    */
   virtual void exit();
   
   /**
    * Causes this Fiber to sleep. If sleep() is called from processMessage(),
    * then the Fiber may be scheduled again before it goes to sleep. If it is
    * called from run(), then it is guaranteed to be scheduled to sleep before
    * run() is called again.
    * 
    * The Fiber will remain asleep until it is woken up or exits.
    */
   virtual void sleep();
   
   /**
    * Causes this Fiber to wakeup if it was sleeping.
    */
   virtual void wakeup();
   
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
    * Runs this Fiber. This method is guaranteed to be run non-concurrently
    * with itself, processMessage(), and exiting().
    */
   virtual void run() = 0;
   
   /**
    * Called just prior to this Fiber's exit. One useful override for this
    * function is to send an event indicating that the Fiber has exited.
    * 
    * This method is guaranteed to be run non-concurrently with itself,
    * processMessage(), and run().
    */
   virtual void exiting() {};
   
   /**
    * Called *only* by a FiberScheduler to have this Fiber process the passed
    * message. This method is guaranteed to be run non-concurrently with
    * itself, run(), and exiting().
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
   virtual void addDeferredMessage(db::rt::DynamicObject& msg);
   
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
