/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/FiberContext.h"

#include "db/fiber/Fiber.h"
#include <cstring>

using namespace db::fiber;

FiberContext::FiberContext() :
   mAllocatedStack(false),
   mBack(NULL)
{
}

FiberContext::~FiberContext()
{
   if(mAllocatedStack)
   {
#ifdef WIN32
      // clean up stack
      free(mUserContext.uc_stack.ss_sp);
#else
      // clean up stack
      munmap(mUserContext.uc_stack.ss_sp, mUserContext.uc_stack.ss_size);
#endif
   }
}

/**
 * The function at the top of a fiber's stack. This function will start
 * a new fiber.
 * 
 * @param fiber the new fiber to start.
 */
static void startFiber(Fiber* fiber)
{
   // start new fiber
   fiber->start();
}

bool FiberContext::init(Fiber* fiber)
{
#ifdef WIN32
   // FIXME: win32 requires malloc to be used for the stack because mmap
   // has issues at present
   void* stack = malloc(fiber->getStackSize());
   mAllocatedStack = (stack != NULL);
#else
   // allocate memory for the context's stack using mmap so the memory
   // is executable and can expand to use available system resources
   // as necessary:
   
   // 0: let mmap pick the memory address
   // stackSize: enough memory for new stack
   // PROT_READ | PROT_WRITE | PROT_EXEC: can be read/written/executed
   // MAP_PRIVATE | MAP_ANONYMOUS: process private with no file descriptor
   // -1: no file descriptor associated
   // 0: start at offset 0
   void* stack = mmap(
      0, fiber->getStackSize(),
      PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   mAllocatedStack = (stack != MAP_FAILED);
#endif
   
   if(mAllocatedStack)
   {
      // get the current context
      getcontext(&mUserContext);
      
      // set the new stack location and size
      mUserContext.uc_stack.ss_sp = stack;
      mUserContext.uc_stack.ss_size = fiber->getStackSize();
      mUserContext.uc_stack.ss_flags = 0;
      mUserContext.uc_link = NULL;
      
      // write the new stack location, etc. to this context
      makecontext(&mUserContext, (void (*)())startFiber, 1, fiber);
   }
   
   return mAllocatedStack;
}

inline void FiberContext::swap(FiberContext* in)
{
   in->mBack = this;
   swapcontext(&mUserContext, &in->mUserContext);
}

inline void FiberContext::swapBack()
{
   swap(mBack);
}

inline void FiberContext::loadBack()
{
   setcontext(&mBack->mUserContext);
}
