/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/Timer.h"

#include "db/rt/System.h"

using namespace db::rt;
using namespace db::util;

void Timer::start()
{
   // FIXME: store start time
}

void Timer::stop()
{
   // FIXME: store elasped time
}

inline uint64_t Timer::startTiming()
{
   return System::getCurrentMilliseconds();
}

inline uint64_t Timer::getMilliseconds(uint64_t startTime)
{
   return System::getCurrentMilliseconds() - startTime;
}

inline double Timer::getSeconds(uint64_t startTime)
{
   return (System::getCurrentMilliseconds() - startTime) / 1000.;
}
