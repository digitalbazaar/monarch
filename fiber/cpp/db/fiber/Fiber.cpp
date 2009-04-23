/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/Fiber.h"

#include "db/fiber/FiberScheduler.h"

using namespace db::fiber;
using namespace db::rt;

#define DEFAULT_STACK_SIZE 0x2000 // 8k

Fiber::Fiber(size_t stackSize) :
   mId(0),
   mState(Fiber::New)
{
   mStackSize = (stackSize == 0 ? DEFAULT_STACK_SIZE : stackSize);
}

Fiber::~Fiber()
{
}

void Fiber::start()
{
   // run custom fiber code
   run();
   
   // fiber now exited
   mScheduler->exit(this);
}

inline void Fiber::yield()
{
   mScheduler->yield(this);
}

inline void Fiber::sleep()
{
   mScheduler->sleep(this);
}

inline void Fiber::wakeup()
{
   mScheduler->wakeupSelf(this);
}

void Fiber::setScheduler(FiberId id, FiberScheduler* scheduler)
{
   mId = id;
   mScheduler = scheduler;
}

inline FiberId Fiber::getId()
{
   return mId;
}

inline void Fiber::setState(State state)
{
   mState = state;
}

inline Fiber::State Fiber::getState()
{
   return mState;
}

inline size_t Fiber::getStackSize()
{
   return mStackSize;
}

inline FiberContext* Fiber::getContext()
{
   return &mContext;
}

inline bool Fiber::canSleep()
{
   return true;
}
