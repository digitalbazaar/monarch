/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/Fiber.h"

#include "db/fiber/FiberScheduler.h"

using namespace db::fiber;

Fiber::Fiber()
{
   // default priority of 0
   setPriority(0);
}

Fiber::~Fiber()
{
}

inline void Fiber::yield()
{
   mScheduler->yield(getId());
}

inline void Fiber::exit()
{
   mScheduler->exit(getId());
}

inline void Fiber::sleep()
{
   mScheduler->sleep(getId());
}

inline void Fiber::wakeup()
{
   mScheduler->wakeup(getId());
}

void Fiber::setScheduler(FiberScheduler* scheduler, FiberId id)
{
   mScheduler = scheduler;
   mId = id;
   mState = Idle;
}

inline FiberId Fiber::getId()
{
   return mId;
}

inline void Fiber::setState(Fiber::State state)
{
   mState = state;
}

inline Fiber::State Fiber::getState()
{
   return mState;
}

inline void Fiber::setPriority(FiberPriority p)
{
   mPriority = p;
}

inline FiberPriority Fiber::getPriority()
{
   return mPriority;
}
