/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/Fiber.h"

#include "db/fiber/FiberScheduler.h"

using namespace db::fiber;

Fiber::Fiber()
{
}

Fiber::~Fiber()
{
}

void Fiber::yield()
{
   mScheduler->yield(getId());
}

void Fiber::exit()
{
   setState(Exiting);
}

void Fiber::sleep()
{
   setState(Sleeping);
}

void Fiber::wakeup()
{
   setState(Idle);
}

void Fiber::setScheduler(FiberScheduler* scheduler, FiberId id)
{
   mScheduler = scheduler;
   mId = id;
   mState = Idle;
}

FiberId Fiber::getId()
{
   return mId;
}

void Fiber::setState(Fiber::State state)
{
   mState = state;
}

Fiber::State Fiber::getState()
{
   return mState;
}
