/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/RateAverager.h"

#include "monarch/rt/System.h"

#include <cmath>

using namespace monarch::rt;
using namespace monarch::util;

RateAverager::RateAverager(uint64_t windowLength)
{
   reset();

   // set window lengths
   mTotal.setLength(0);
   mCurrent.setLength(windowLength);
   mNext.setLength(windowLength);
}

RateAverager::~RateAverager()
{
}

void RateAverager::reset()
{
   mLock.lock();
   {
      // reset windows
      mTotal.reset();
      mCurrent.reset();
      mNext.reset();
   }
   mLock.unlock();
}

void RateAverager::addItems(uint64_t count, uint64_t start)
{
   mLock.lock();
   {
      // get the current time
      uint64_t now = System::getCurrentMilliseconds();

      // initialize windows
      if(mTotal.getStartTime() == 0)
      {
         mTotal.setStartTime(start);
         setWindowStartTimes(start);
      }

      // update the windows
      updateWindows(now);

      // add items to each window
      mTotal.addItems(count, start, now);
      mCurrent.addItems(count, start, now);
      mNext.addItems(count, start, now);
   }
   mLock.unlock();
}

double RateAverager::getItemsPerMillisecond()
{
   double rval = 0.0;

   mLock.lock();
   {
      // update windows and get the current window rate
      updateWindows(System::getCurrentMilliseconds());
      rval = mCurrent.getItemsPerMillisecond();
   }
   mLock.unlock();

   return rval;
}

double RateAverager::getItemsPerSecond()
{
   double rval = 0.0;

   mLock.lock();
   {
      // update windows and get the current window rate
      updateWindows(System::getCurrentMilliseconds());
      rval = mCurrent.getItemsPerSecond();
   }
   mLock.unlock();

   return rval;
}

double RateAverager::getTotalItemsPerMillisecond()
{
   double rval = 0.0;

   mLock.lock();
   {
      // get the total window rate
      rval = mTotal.getItemsPerMillisecond();
   }
   mLock.unlock();

   return rval;
}

double RateAverager::getTotalItemsPerSecond()
{
   double rval = 0.0;

   mLock.lock();
   {
      // get the total window rate
      rval = mTotal.getItemsPerSecond();
   }
   mLock.unlock();

   return rval;
}

uint64_t RateAverager::getTotalItemCount()
{
   return mTotal.getItemCount();
}

uint64_t RateAverager::getTotalItemTime()
{
   return mTotal.getItemTime();
}

void RateAverager::setWindowLength(uint64_t length)
{
   mLock.lock();
   {
      // the window length must be at least two because two windows that
      // are 1/2 of the window length apart are always stored -- and
      // this RateAverager is only accurate to 1 whole millisecond
      updateWindowLengths((2 > length) ? 2 : length);
   }
   mLock.unlock();
}

uint64_t RateAverager::getWindowLength()
{
   return mCurrent.getLength();
}

uint64_t RateAverager::getEta(uint64_t count, bool current)
{
   uint64_t rval = 0;

   if(count > 0)
   {
      mLock.lock();
      {
         // use either current or total rate
         double rate = current ? getItemsPerSecond() : getTotalItemsPerSecond();

         // divide the remaining count by the rate
         rval = (uint64_t)roundl(count / rate);
      }
      mLock.unlock();
   }

   return rval;
}

void RateAverager::setWindowStartTimes(uint64_t time)
{
   // set window start times (next overlaps with current by half)
   mCurrent.setStartTime(time);
   mNext.setStartTime(time + getHalfWindowLength());
}

void RateAverager::updateWindows(uint64_t now)
{
   // update total current time
   mTotal.setCurrentTime(now);

   /* Algorithm:
      1. If now >= end of next window, we need two new windows.
      2. If now >= end of current window, we need to move the windows.
      3. If now < end of current window, we need to add time passed.
    */
   if(now >= mNext.getEndTime())
   {
      // reset the windows
      mCurrent.reset();
      mNext.reset();

      // set start times 1 millisecond in the past because item start time
      // must be before now (see usage of addItems())
      setWindowStartTimes(now - 1);
   }
   else if(now >= mCurrent.getEndTime())
   {
      // set the current window to the next window and set time passed
      mCurrent.setEqualTo(mNext);

      // reset the next window and set its new start time
      mNext.reset();
      mNext.setStartTime(mCurrent.getStartTime() + getHalfWindowLength());
   }

   // update current window time
   mCurrent.setCurrentTime(now);
}

uint64_t RateAverager::getHalfWindowLength()
{
   return (uint64_t)roundl(getWindowLength() / 2.0);
}

void RateAverager::updateWindowLengths(uint64_t length)
{
   // set current window length
   mCurrent.setLength(length);

   // set new window length and reset it
   mNext.setLength(length);
   mNext.reset();
   mNext.setStartTime(mCurrent.getStartTime() + getHalfWindowLength());
}
