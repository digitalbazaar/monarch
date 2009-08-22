/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_fiber_Fiber_H
#define db_fiber_Fiber_H

#include "db/fiber/FiberContext.h"

#include <inttypes.h>

namespace db
{
namespace fiber
{

// type definitions
typedef uint32_t FiberId;

// forward declarations
class FiberScheduler;

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
      New         = 0,
      Running     = 1 << 0,
      Sleeping    = 1 << 1,
      Waking      = 1 << 2,
      Exited      = 1 << 3,

      // a deleted fiber should be in this state
      Dead        = 1 << 7
   };

protected:
   /**
    * The ID of this Fiber.
    */
   FiberId mId;

   /**
    * The FiberScheduler in charge of scheduling and running this Fiber.
    */
   FiberScheduler* mScheduler;

   /**
    * The current state of this Fiber.
    */
   State mState;

   /**
    * The size of this fiber's stack.
    */
   size_t mStackSize;

   /**
    * This fiber's execution context.
    */
   FiberContext mContext;

public:
   /**
    * Creates a new Fiber with the specified stack size.
    *
    * @param stackSize the stack size to use in bytes, 0 for the default.
    */
   Fiber(size_t stackSize = 0);

   /**
    * Destructs this Fiber.
    */
   virtual ~Fiber();

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
   virtual void setScheduler(FiberId id, FiberScheduler* scheduler);

   /**
    * Gets this Fiber's ID, as assigned by its FiberScheduler.
    *
    * @return this Fiber's ID.
    */
   virtual FiberId getId();

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
    * Gets this fiber's stack size.
    *
    * @return this fiber's stack size.
    */
   virtual size_t getStackSize();

   /**
    * Gets this fiber's execution context.
    *
    * @return this fiber's execution context.
    */
   virtual FiberContext* getContext();

   /**
    * Determines whether or not a fiber is capable of sleeping when a
    * particular call to sleep is made. This method will be called by this
    * fiber's scheduler, inside of its scheduler lock, right before sleeping
    * this fiber. The fiber is guaranteed not to change state while this call
    * is being made. To create deadlock free code, whatever condition would
    * cause this fiber to wake up should be checked (within a mutex) in this
    * call. The condition should be set within the same mutex elsewhere --
    * followed by a call to wakeup().
    *
    * If this method returns false, the fiber will not be sleeped, but its
    * context will still be swapped out.
    *
    * This method is essential for preventing race conditions in extending
    * fiber classes that need to sleep. The base fiber class is never capable
    * of sleeping.
    *
    * Note: One way to implement deadlock-free and reliable sleep-conditional
    * fibers:
    *
    * class MyFiber : public Fiber
    * {
    *    volatile bool mCondition;
    *
    *    bool canSleep()
    *    {
    *       bool rval;
    *       mutex.lock();
    *       rval = mCondition;
    *       mutex.unlock();
    *       return rval;
    *    }
    *
    *    void myWakeup()
    *    {
    *       mutex.lock();
    *       mCondition = false;
    *       mutex.unlock();
    *       wakeup();
    *    }
    * }
    *
    * @return true if this fiber can be put to sleep at the moment, false
    *         if not.
    */
   virtual bool canSleep();

   /**
    * Yields this fiber temporarily to allow another fiber to run.
    *
    * This method *must* only be called inside run().
    */
   virtual void yield();

   /**
    * Causes this fiber to sleep, if canSleep() returns true when this
    * fiber's scheduler attempts to put it to sleep. The fiber may be awakened
    * only be a wakeup() call to its FiberScheduler using this fiber's ID.
    *
    * Proper use of this method may involve calling it inside of a loop that
    * depends on a particular condition that will be modified by some external
    * code that will then call wakeup().
    *
    * This method *must* only be called inside run().
    */
   virtual void sleep();

   /**
    * Causes this fiber to wakeup, if this fiber was asleep.
    *
    * This method may be called outside of run to wake up this fiber, but
    * *must* be done with appropriate protections. This method must only
    * be called when it is not possible for this fiber to be exiting.
    */
   virtual void wakeup();
};

} // end namespace fiber
} // end namespace db
#endif
