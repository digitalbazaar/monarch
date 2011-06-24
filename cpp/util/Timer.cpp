/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/Timer.h"

#include "monarch/rt/System.h"

using namespace monarch::rt;
using namespace monarch::util;

Timer::Timer() :
   mStartTime(0)
{
}

Timer::~Timer()
{
}

uint64_t Timer::start()
{
   mStartTime = System::getCurrentMilliseconds();
   return mStartTime;
}

uint64_t Timer::getElapsedMilliseconds()
{
   return System::getCurrentMilliseconds() - mStartTime;
}

double Timer::getElapsedSeconds()
{
   return getElapsedMilliseconds() / 1000.;
}

uint64_t Timer::getRemainingMilliseconds(uint64_t until)
{
   uint64_t rval = 0;

   uint64_t now = System::getCurrentMilliseconds();
   uint64_t then = mStartTime + until;
   if(now > then)
   {
      rval = now - then;
   }

   return rval;
}

double Timer::getRemainingSeconds(uint64_t until)
{
   return getRemainingMilliseconds(until) / 1000.;
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
