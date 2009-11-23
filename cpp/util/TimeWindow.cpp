/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/util/TimeWindow.h"

#include "db/rt/System.h"

#include <math.h>

using namespace db::rt;
using namespace db::util;

TimeWindow::TimeWindow(uint64_t length)
{
   // set the window length and reset
   setLength(length);
   reset();
}

TimeWindow::~TimeWindow()
{
}

void TimeWindow::adjustItemCount(uint64_t timeChange)
{
   // multiply the current rate by the time change and
   // increase the item count accordingly
   increaseItemCount((uint64_t)roundl(getItemsPerMillisecond() * timeChange));
}

void TimeWindow::reset()
{
   mStartTime = 0;
   mItemCount = 0;
   mTimePassed = 0;
}

void TimeWindow::setEqualTo(TimeWindow& window)
{
   mLength = window.getLength();
   mStartTime = window.getStartTime();
   mItemCount = window.getItemCount();
   mTimePassed = window.getTimePassed();
}

double TimeWindow::getItemsPerMillisecond()
{
   // get the rate in items per millisecond
   return calcItemsPerMillisecond(getItemCount(), getTimePassed());
}

double TimeWindow::getItemsPerSecond()
{
   // get the rate in items per second
   return calcItemsPerSecond(getItemCount(), getTimePassed());
}

inline void TimeWindow::setLength(uint64_t length)
{
   setLength(length, false);
}

void TimeWindow::setLength(uint64_t length, bool adjust)
{
   // if adjusting the item count
   if(adjust)
   {
      // get the time change
      uint64_t timeChange = length - mLength;

      // adjust the item count
      adjustItemCount(timeChange);
   }

   // set new length
   mLength = length;
}

inline uint64_t TimeWindow::getLength()
{
   return mLength;
}

inline void TimeWindow::setStartTime(uint64_t time)
{
   setStartTime(time, false);
}

void TimeWindow::setStartTime(uint64_t time, bool adjust)
{
   // if adjusting the item count
   if(adjust)
   {
      // get the time change
      uint64_t timeChange = time - mStartTime;

      // adjust the item count
      adjustItemCount(timeChange);
   }

   // set new start time
   mStartTime = time;
}

inline uint64_t TimeWindow::getStartTime()
{
   return mStartTime;
}

uint64_t TimeWindow::getEndTime()
{
   uint64_t rval = 0;

   if(getLength() > 0)
   {
      rval = getStartTime() + getLength() - 1;
   }

   return rval;
}

uint64_t TimeWindow::getCurrentTime()
{
   return getStartTime() + getTimePassed();
}

uint64_t TimeWindow::getRemainingTime()
{
   return getEndTime() - getCurrentTime();
}

bool TimeWindow::isTimeInWindow(uint64_t time)
{
   bool rval = false;

   // make sure time is passed start time
   if(time >= getStartTime())
   {
      // make that there is no maximum length or the time falls within
      // the maximum length
      if(getLength() == 0 || time < getEndTime())
      {
         rval = true;
      }
   }

   return rval;
}

void TimeWindow::increaseItemCount(uint64_t increase)
{
   mItemCount += increase;
}

void TimeWindow::increaseItemCount(uint64_t increase, uint64_t interval)
{
   increaseItemCount(increase);
   increaseTimePassed(interval);
}

void TimeWindow::increaseTimePassed(uint64_t time)
{
   setTimePassed(mTimePassed + time);
}

void TimeWindow::setTimePassed(uint64_t time)
{
   mTimePassed = time;

   // see if this window has a maximum length
   if(getLength() != 0)
   {
      // cap time passed at the length of the window
      mTimePassed = (mTimePassed < getLength()) ? mTimePassed : getLength();
   }
}

inline uint64_t TimeWindow::getItemCount()
{
   return mItemCount;
}

inline uint64_t TimeWindow::getTimePassed()
{
   return mTimePassed;
}

double TimeWindow::calcItemsPerMillisecond(double items, double interval)
{
   // items / millisecond -- force interval to 1
   return items / (1.0 > interval ? 1.0 : interval);
}

double TimeWindow::calcItemsPerSecond(double items, double interval)
{
   // items / millisecond * 1000 = items / second
   return calcItemsPerMillisecond(items, interval) * 1000.0;
}
