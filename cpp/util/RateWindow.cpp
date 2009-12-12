/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/RateWindow.h"

#include "monarch/rt/System.h"

#include <math.h>

using namespace db::rt;
using namespace db::util;

RateWindow::RateWindow(uint64_t length) :
   mLength(length)
{
   reset();
}

RateWindow::~RateWindow()
{
}

void RateWindow::reset()
{
   mStartTime = 0;
   mTimePassed = 0;
   mItemCount = 0;
   mItemTime = 0;
   mEarliestAddTime = 0;
   mLastAddTime = 0;
}

void RateWindow::setEqualTo(RateWindow& window)
{
   mLength = window.getLength();
   mStartTime = window.getStartTime();
   mTimePassed = window.getTimePassed();
   mItemCount = window.getItemCount();
   mItemTime = window.getItemTime();
}

void RateWindow::setLength(uint64_t length)
{
   // set new length
   mLength = length;
}

uint64_t RateWindow::getLength()
{
   return mLength;
}

void RateWindow::setStartTime(uint64_t time)
{
   // set new start time
   mStartTime = time;
}

uint64_t RateWindow::getStartTime()
{
   return mStartTime;
}

uint64_t RateWindow::getEndTime()
{
   uint64_t rval = 0;

   if(getLength() > 0)
   {
      rval = getStartTime() + getLength();
   }

   return rval;
}

void RateWindow::setCurrentTime(uint64_t time)
{
   mTimePassed = time - getStartTime();
}

uint64_t RateWindow::getCurrentTime()
{
   return getStartTime() + getTimePassed();
}

uint64_t RateWindow::getRemainingTime()
{
   return getEndTime() - getCurrentTime();
}

void RateWindow::setTimePassed(uint64_t time)
{
   // see if this window has a maximum length
   if(getLength() != 0)
   {
      // cap time passed at the length of the window
      mTimePassed = (time < getLength()) ? time : getLength();
   }
   else
   {
      mTimePassed = time;
   }
}

uint64_t RateWindow::getTimePassed()
{
   return mTimePassed;
}

void RateWindow::increaseTimePassed(uint64_t time)
{
   setTimePassed(mTimePassed + time);
}

bool RateWindow::isTimeInWindow(uint64_t time)
{
   // time must be at or after start time and there must be no end time or
   // the time must be before it
   return
      time >= getStartTime() &&
      (getLength() == 0 || time < getEndTime());
}

void RateWindow::setItemCount(uint64_t count)
{
   mItemCount = count;
}

uint64_t RateWindow::getItemCount()
{
   return mItemCount;
}

void RateWindow::setItemTime(uint64_t time)
{
   mItemTime = time;
}

uint64_t RateWindow::getItemTime()
{
   return mItemTime;
}

void RateWindow::addItems(uint64_t count, uint64_t start, uint64_t now)
{
   // Note: start *must* be before now, and now *must* be >= getStartTime()

   // if start is before this window, then adjust count and start
   if(start < getStartTime())
   {
      double rate = ((double)count) / (now - start);
      start = getStartTime();
      count = (uint64_t)roundl(rate * (now - start));
   }

   // start >= getStartTime() is assumed, but check start < getEndTime()
   // and now >= getStartTime()
   if((getEndTime() == 0 || start < getEndTime()) && now >= getStartTime())
   {
      // initialize earliest and last add times as necessary
      if(mEarliestAddTime == 0)
      {
         mEarliestAddTime = mLastAddTime = start;
      }

      /* Note: In order to prevent double-counting overlapping time segments,
         we keep track of the earliest start time passed to addItems() as
         mEarliestAddTime and the last time addItems() was called as
         mLastAddTime.

         The start time passed to addItems() may have occurred before, at,
         or after mEarliestAddTime. It may have also occurred before, at,
         or after mLastAddTime. Here are the possible cases:

         1. start < mEarliestAddTime:
            interval = (mEarliestAddTime - start) + (now - mLastAddTime).
         2. start >= mEarliestAddTime && start <= mLastAddTime:
            interval = (now - mLastAddTime).
         3. start >= mEarliestAddTime && start > mLastAddTime:
            interval = (now - start).
       */
      uint64_t interval;
      if(start < mEarliestAddTime)
      {
         interval = (mEarliestAddTime - start) + (now - mLastAddTime);
         mEarliestAddTime = start;
      }
      else if(start <= mLastAddTime)
      {
         interval = (now - mLastAddTime);
      }
      else
      {
         interval = (now - start);
      }

      // update item count, item time, and set new last add time
      mItemCount += count;
      mItemTime += interval;
      mLastAddTime = now;
   }
}

double RateWindow::getItemsPerMillisecond()
{
   // get the rate in items per millisecond
   return calcItemsPerMillisecond(getItemCount(), getItemTime());
}

double RateWindow::getItemsPerSecond()
{
   // get the rate in items per second
   return calcItemsPerSecond(getItemCount(), getItemTime());
}

double RateWindow::calcItemsPerMillisecond(double items, double interval)
{
   // items / millisecond -- force interval to 1
   return items / (1.0 > interval ? 1.0 : interval);
}

double RateWindow::calcItemsPerSecond(double items, double interval)
{
   // items / millisecond * 1000 = items / second
   return calcItemsPerMillisecond(items, interval) * 1000.0;
}
