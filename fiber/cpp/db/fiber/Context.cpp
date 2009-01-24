/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/Context.h"

#include "db/fiber/Fiber2.h"
#include "db/fiber/WindowsSupport.h"

#include <sys/mman.h>

using namespace db::fiber;

Context::Context() :
   mAllocatedStack(false),
   mBack(NULL)
{
}

Context::~Context()
{
   if(mAllocatedStack)
   {
      // clean up stack
      munmap(mUserContext.uc_stack.ss_sp, mUserContext.uc_stack.ss_size);
   }
}

/**
 * The function at the top of a fiber's stack. This function will start
 * a new fiber.
 * 
 * @param fiber the new fiber to start.
 */
static void startFiber(Fiber2* fiber)
{
   // start new fiber
   fiber->start();
}

void Context::init(Fiber2* fiber, size_t stackSize)
{
   // context has an allocated stack
   mAllocatedStack = true;
   
   // get the current context
   getcontext(&mUserContext);
   
   // allocate memory for the context's stack using mmap so the memory
   // is executable and can expand to use available system resources
   // as necessary:
   
   // 0: let mmap pick the memory address
   // stackSize: enough memory for new stack
   // PROT_READ | PROT_WRITE | PROT_EXEC: can be read/written/executed
   // MAP_PRIVATE | MAP_ANONYMOUS: process private with no file descriptor
   // -1: no file descriptor associated
   // 0: start at offset 0
   mUserContext.uc_stack.ss_sp = mmap(
      0, stackSize,
      PROT_READ | PROT_WRITE | PROT_EXEC,
      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   mUserContext.uc_stack.ss_size = stackSize;
   mUserContext.uc_stack.ss_flags = 0;
   mUserContext.uc_link = NULL;
   
   // write the new stack location, etc. to this context
   makecontext(&mUserContext, (void (*)())startFiber, 1, fiber);
}

void Context::swap(Context* in)
{
   in->mBack = this;
   swapcontext(&mUserContext, &in->mUserContext);
}

inline void Context::swapBack()
{
   swap(mBack);
}

inline void Context::loadBack()
{
   setcontext(&mBack->mUserContext);
}
