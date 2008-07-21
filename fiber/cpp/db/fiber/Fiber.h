/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_fiber_Fiber_H
#define db_fiber_Fiber_H

#include "db/rt/DynamicObject.h"

namespace db
{
namespace fiber
{

// define FiberId type
typedef uint32_t FiberId;

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
 * processMessage method.
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
      None, Idle, Running, Sleeping, Exiting
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
    * Yields this Fiber temporarily to allow another Fiber to run.
    * 
    * This method *must* only be called inside run().
    */
   virtual void yield();
   
   /**
    * Causes this Fiber to exit the next time it is scheduled to be run.
    * 
    * This method *must* only be called inside run() or processMessage().
    */
   virtual void exit();
   
   /**
    * Causes this Fiber to sleep until it is woken up.
    * 
    * This method *must* only be called inside run() or processMessage().
    */
   virtual void sleep();
   
   /**
    * Causes this Fiber to wakeup if it was sleeping.
    * 
    * This method *must* only be called inside processMessage().
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
    * with itself, though it could be run while processMessage() is being
    * executed.
    */
   virtual void run() = 0;
   
   /**
    * Called by a FiberScheduler to have this Fiber process the passed
    * message. This method is guaranteed to be run non-concurrently with
    * itself, though it could be run while run() is being executed.
    * 
    * @param msg the message to process.
    */
   virtual void processMessage(db::rt::DynamicObject& msg) {};
   
   /**
    * Called by a FiberScheduler to claim ownership of this Fiber.
    * 
    * @param scheduler the FiberScheduler that is claiming this Fiber.
    * @param id the FiberId assigned to this Fiber.
    */
   virtual void setScheduler(FiberScheduler* scheduler, FiberId id);
   
   /**
    * Gets this Fiber's ID, as assigned by its FiberScheduler.
    * 
    * @return this Fiber's ID.
    */
   virtual FiberId getId();
   
   /**
    * Called by a FiberScheduler to set this Fiber's current state.
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
};

} // end namespace fiber
} // end namespace db
#endif
