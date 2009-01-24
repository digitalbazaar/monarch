/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_fiber_Context_H
#define db_fiber_Context_H

#include <ucontext.h>

#include "db/rt/Thread.h"

namespace db
{
namespace fiber
{

class Fiber2;

/**
 * A context is a snapshot of the minimal information required to continue
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
class Context
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
   Context* mBack;
   
public:
   /**
    * Creates a new uninitialized Context.
    */
   Context();
   
   /**
    * Destructs this Context.
    */
   virtual ~Context();
   
   /**
    * Initializes this context by setting up a stack for the passed fiber.
    * 
    * @param fiber the fiber to create a stack for.
    * @param stackSize the size of the stack to create.
    */
   virtual void init(Fiber2* fiber, size_t stackSize);
   
   /**
    * Saves this context and swaps another one in. This context will be
    * stored in the passed one so that it can swap it back in if appropriate.
    * 
    * @param in the new context to swap in.
    */
   virtual void swap(Context* in);
   
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
