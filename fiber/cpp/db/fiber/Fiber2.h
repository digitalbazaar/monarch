/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_fiber_Fiber2_H
#define db_fiber_Fiber2_H

#include "db/fiber/FiberContext.h"

#include <inttypes.h>

namespace db
{
namespace fiber
{

// type definitions
typedef uint32_t FiberId2;

// forward declarations
class FiberScheduler2;

/**
 * A Fiber is a single unit of execution. Fibers use cooperatively multitasking
 * to allow code to be run concurrently. Each Fiber is scheduled and run by
 * a FiberScheduler. Multiple threads can be used to run multiple Fibers, but
 * no Fiber will ever run concurrently with itself.
 * 
 * A Fiber will be continually scheduled by its FiberScheduler until it
 * returns from its run() method. It may use a call to yield() to allow
 * other Fibers to do work. A Fiber may also use a call to sleep() to prevent
 * the Fiber from being scheduled until a wakeup() call has been issued to
 * the FiberScheduler using the Fiber's ID.
 * 
 * @author Dave Longley
 */
class Fiber2
{
public:
   /**
    * A Fiber's execution state.
    */
   typedef uint8_t State;
   enum FiberStates
   {
      // states a fiber can be in
      New         = 0,
      Running     = 1 << 0,
      Sleeping    = 1 << 1,
      Exited      = 1 << 2,
      
      // a deleted fiber should be in this state
      Dead        = 1 << 7
   };
   
protected:
   /**
    * The ID of this Fiber.
    */
   FiberId2 mId;
   
   /**
    * The FiberScheduler in charge of scheduling and running this Fiber.
    */
   FiberScheduler2* mScheduler;
   
   /**
    * The current state of this Fiber.
    */
   State mState;
   
   /**
    * This fiber's execution context.
    */
   FiberContext mContext;
   
public:
   /**
    * Creates a new Fiber.
    */
   Fiber2();
   
   /**
    * Destructs this Fiber.
    */
   virtual ~Fiber2();
   
   /**
    * Called via *only* by a FiberScheduler to start this fiber.
    */
   virtual void start();
   
   /**
    * Executes the custom work for this fiber. This method is guaranteed to be
    * a non-concurrent with itself. The methods yield() and sleep() may be
    * called from run() to alter scheduling.
    */
   virtual void run() = 0;
   
   /**
    * Called *only* by a FiberScheduler to claim ownership of this fiber.
    * 
    * @param id the FiberId assigned to this fiber.
    * @param scheduler the FiberScheduler that is claiming this fiber.
    */
   virtual void setScheduler(FiberId2 id, FiberScheduler2* scheduler);
   
   /**
    * Gets this Fiber's ID, as assigned by its FiberScheduler.
    * 
    * @return this Fiber's ID.
    */
   virtual FiberId2 getId();
   
   /**
    * Called *only* by a FiberScheduler to set this fiber's current state.
    * 
    * @param state the new state for this fiber.
    */
   virtual void setState(State state);
   
   /**
    * Gets this fiber's current state.
    * 
    * @return this fiber's current state.
    */
   virtual State getState();
   
   /**
    * Gets this fiber's execution context.
    * 
    * @return this fiber's execution context.
    */
   virtual FiberContext* getContext();
   
protected:
   /**
    * Yields this fiber temporarily to allow another fiber to run.
    * 
    * This method *must* only be called inside run().
    */
   virtual void yield();
   
   /**
    * Causes this fiber to sleep. The fiber may be awakened only be a wakeup()
    * call to its FiberScheduler using this fiber's ID.
    * 
    * This method *must* only be called inside run().
    */
   virtual void sleep();
};

} // end namespace fiber
} // end namespace db
#endif
