/*
 * Copyright (c) 2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/fiber/Fiber2.h"

#include "db/fiber/FiberScheduler2.h"

using namespace db::fiber;
using namespace db::rt;

Fiber2::Fiber2() :
   mId(0),
   mState(Fiber2::New)
{
}

Fiber2::~Fiber2()
{
}

void Fiber2::start()
{
   // run custom fiber code
   run();
   
   // fiber now exited
   mScheduler->exit(this);
}

inline void Fiber2::yield()
{
   mScheduler->yield(this);
}

inline void Fiber2::sleep()
{
   mScheduler->sleep(this);
}

void Fiber2::setScheduler(FiberId2 id, FiberScheduler2* scheduler)
{
   mId = id;
   mScheduler = scheduler;
}

inline FiberId2 Fiber2::getId()
{
   return mId;
}

inline void Fiber2::setState(State state)
{
   mState = state;
}

inline Fiber2::State Fiber2::getState()
{
   return mState;
}

inline FiberContext* Fiber2::getContext()
{
   return &mContext;
}
