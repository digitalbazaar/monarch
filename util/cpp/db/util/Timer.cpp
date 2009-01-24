/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/Timer.h"

#include "db/rt/System.h"

using namespace db::rt;
using namespace db::util;

Timer::Timer() :
   mStartTime(0)
{
}

Timer::~Timer()
{
}

void Timer::start()
{
   mStartTime = System::getCurrentMilliseconds();
}

uint64_t Timer::getElapsedMilliseconds()
{
   return System::getCurrentMilliseconds() - mStartTime;
}

double Timer::getElapsedSeconds()
{
   return getElapsedMilliseconds() / 1000.;
}

// FIXME: remove the static methods below:

uint64_t Timer::startTiming()
{
   return System::getCurrentMilliseconds();
}

uint64_t Timer::getMilliseconds(uint64_t startTime)
{
   return System::getCurrentMilliseconds() - startTime;
}

double Timer::getSeconds(uint64_t startTime)
{
   return (System::getCurrentMilliseconds() - startTime) / 1000.;
}
