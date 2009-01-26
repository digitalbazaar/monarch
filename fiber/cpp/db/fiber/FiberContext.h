/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_fiber_FiberContext_H
#define db_fiber_FiberContext_H

#include "db/fiber/WindowsSupport.h"

namespace db
{
namespace fiber
{

class Fiber2;

/**
 * A FiberContext is a snapshot of the minimal information required to continue
 * a paused process of execution. It is used in this fiber library to store
 * the stack and other information for fibers and fiber scheduler threads.
 * 
 * A FiberScheduler is used to swap different fiber contexts in and out to
 * accomplish cooperative multitasking. Whenever a fiber yields, its current
 * context is saved and the FiberScheduler then swaps another fiber's context
 * in so that it may continue executing where it left off. This swapping is
 * done in user-space, thus saving any kernel-level overhead.
 * 
 * @author Dave Longley
 */
class FiberContext
{
protected:
   /**
    * The user context object.
    */
   ucontext_t mUserContext;
   
   /**
    * Set to true if this context has an allocated stack.
    */
   bool mAllocatedStack;
   
   /**
    * Set to context that was last swapped out.
    */
   FiberContext* mBack;
   
public:
   /**
    * Creates a new uninitialized FiberContext.
    */
   FiberContext();
   
   /**
    * Destructs this FiberContext.
    */
   virtual ~FiberContext();
   
   /**
    * Initializes this context by setting up a stack for the passed fiber.
    * 
    * @param fiber the fiber to create a stack for.
    * 
    * @return true if successful, false if there was not enough memory to
    *         allocate the fiber at this time.
    */
   virtual bool init(Fiber2* fiber);
   
   /**
    * Saves this context and swaps another one in. This context will be
    * stored in the passed one so that it can swap it back in if appropriate.
    * 
    * @param in the new context to swap in.
    */
   virtual void swap(FiberContext* in);
   
   /**
    * Swaps this context back to the one that last swapped it out.
    */
   virtual void swapBack();
   
   /**
    * Loads the context that swapped this one out.
    */
   virtual void loadBack();
};

} // end namespace fiber
} // end namespace db
#endif
