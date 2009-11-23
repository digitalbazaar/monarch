/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/util/RateAverager.h"

#include "db/rt/System.h"

#include <math.h>

using namespace db::rt;
using namespace db::util;

RateAverager::RateAverager(uint64_t windowLength)
{
   reset();

   // set window lengths
   mCurrentWindow.setLength(windowLength);
   mNextWindow.setLength(windowLength);
}

RateAverager::~RateAverager()
{
}

void RateAverager::reset()
{
   mLock.lock();
   {
      // reset vars
      mItemCount = 0;
      mTimePassed = 0;
      mEarliestAddTime = 0;
      mLastAddTime = 0;

      // reset the windows
      mCurrentWindow.reset();
      mNextWindow.reset();
   }
   mLock.unlock();
}
#include <cstdio>
void RateAverager::addItems(uint64_t count, uint64_t start)
{
   /* Algorithm:
      1. Update the total item count.
      2. Determine the amount of unique time to add to mTimePassed.
      3. Update the windows based on the current time.
      4. Calculate the item rate.
      5. Determine how many items to add to each window:
         1. Get how much time was spent in the window.
         2. Multiply the time spent in the window by the rate.
         3. Add the resulting items to the window.
   */
   mLock.lock();
   {
      // get the current time
      uint64_t now = System::getCurrentMilliseconds();

      // initialize earliest and last add times
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

      // 1 & 2. update item count, time passed, and set new last add time
      mItemCount += count;
      mTimePassed += interval;
      mLastAddTime = now;

      // 3. update windows
      updateWindows(now);

      // 4. get item rate
      double rate = ((double)count) / (now - start);

      // 5. (Current) add proportional item count to current window
      // (remember: "now" is always in the current window):

      // if start falls before the current window start time, then we must
      // only add the portion of our count to the current window that would
      // occurred within its boundaries
      if(start < mCurrentWindow.getStartTime())
      {
         // add item count for all time passed in the current window
         mCurrentWindow.increaseItemCount(
            (uint64_t)roundl(rate * mCurrentWindow.getTimePassed()));
      }
      else
      {
         // add the entire item count
         mCurrentWindow.increaseItemCount(count);
      }

      // 5. (Next) add proportional item count to next window
      if(now > mNextWindow.getStartTime())
      {
         // add item count for all time passed in the next window
         uint64_t passed = now - mNextWindow.getStartTime();
         mCurrentWindow.increaseItemCount((uint64_t)roundl(rate * passed));
      }
   }
   mLock.unlock();
}

uint64_t RateAverager::getTimePassed()
{
   return mTimePassed;
}

double RateAverager::getItemsPerMillisecond()
{
   double rval = 0.0;

   mLock.lock();
   {
      // make sure that the time passed is greater than 0
      if(getTimePassed() > 0)
      {
         // update windows
         updateWindows(System::getCurrentMilliseconds());

         // get the current window rate
         rval = mCurrentWindow.getItemsPerMillisecond();
      }
   }
   mLock.unlock();

   return rval;
}

double RateAverager::getItemsPerSecond()
{
   double rval = 0.0;

   mLock.lock();
   {
      // make sure that the time passed is greater than 0
      if(getTimePassed() > 0)
      {
         // update windows
         updateWindows(System::getCurrentMilliseconds());

         // get the current window rate
         rval = mCurrentWindow.getItemsPerSecond();
      }
   }
   mLock.unlock();

   return rval;
}

double RateAverager::getTotalItemsPerMillisecond()
{
   double rval = 0.0;

   mLock.lock();
   {
      // make sure that the time passed is greater than 0
      if(getTimePassed() > 0)
      {
         // get the rate in items per millisecond
         rval = TimeWindow::calcItemsPerMillisecond(
            mItemCount, getTimePassed());
      }
   }
   mLock.unlock();

   return rval;
}

double RateAverager::getTotalItemsPerSecond()
{
   double rval = 0.0;

   mLock.lock();
   {
      // make sure that the time passed is greater than 0
      if(getTimePassed() > 0)
      {
         // get the rate in items per second
         rval = TimeWindow::calcItemsPerSecond(mItemCount, getTimePassed());
      }
   }
   mLock.unlock();

   return rval;
}

inline uint64_t RateAverager::getTotalItemCount()
{
   return mItemCount;
}

void RateAverager::setWindowLength(uint64_t length)
{
   mLock.lock();
   {
      // the window length must be at least two because two windows that
      // are 1/2 of the window length apart are always stored -- and
      // this RateAverager is only accurate to 1 whole millisecond
      length = (2 > length) ? 2 : length;

      // update window lengths
      updateWindowLengths(length);
   }
   mLock.unlock();
}

inline uint64_t RateAverager::getWindowLength()
{
   return mCurrentWindow.getLength();
}

uint64_t RateAverager::getEta(uint64_t count, bool current)
{
   uint64_t rval = 0;

   mLock.lock();
   {
      if(count > 0)
      {
         // use either current or total rate
         double rate = current ?
            getItemsPerSecond() : getTotalItemsPerSecond();

         // divide the remaining count by the rate
         rval = (uint64_t)roundl(count / rate);
      }
   }
   mLock.unlock();

   return rval;
}

void RateAverager::setWindowStartTimes(uint64_t time)
{
   // set window start times (next overlaps with current by half)
   mCurrentWindow.setStartTime(time);
   mNextWindow.setStartTime(time + getHalfWindowLength());
}

void RateAverager::updateWindows(uint64_t now)
{
   /* Algorithm:
      1. If now > end of next window, we need two new windows.
      2. If now > end of current window, we need to move the windows.
      3. If now < end of current window, we need to add time passed.

      Note: Do not bother setting time passed on next window, only set it
      on the current window.
    */
   if(now > mNextWindow.getEndTime())
   {
      // reset the windows
      mCurrentWindow.reset();
      mNextWindow.reset();

      // always start at least 1 millisecond into the current window
      setWindowStartTimes(now - 1);
      mCurrentWindow.setTimePassed(1);
   }
   else if(now > mCurrentWindow.getEndTime())
   {
      // save how far we've gone passed the end of the current window
      // (remember: next window starts halfway into current window)
      uint64_t hwl = getHalfWindowLength();
      uint64_t passed = now - mCurrentWindow.getEndTime() + hwl;

      // set the current window to the next window and set time passed
      mCurrentWindow.setEqualTo(mNextWindow);
      mCurrentWindow.setTimePassed(passed);

      // reset the next window and set its new start time
      mNextWindow.reset();
      mNextWindow.setStartTime(mCurrentWindow.getStartTime() + hwl);

      // if we're into the next window already, then we must add items
      // proportionally to it
      if(mCurrentWindow.getCurrentTime() > mNextWindow.getStartTime())
      {
         passed = mCurrentWindow.getCurrentTime() - mNextWindow.getStartTime();
         uint64_t count = mCurrentWindow.getItemsPerMillisecond() * passed;
         mNextWindow.increaseItemCount(count);
      }
   }
   // advance the time in the current window
   else if(now > mCurrentWindow.getStartTime())
   {
      mCurrentWindow.setTimePassed(now - mCurrentWindow.getStartTime());
   }
}

uint64_t RateAverager::getHalfWindowLength()
{
   return (uint64_t)roundl(getWindowLength() / 2.0);
}

void RateAverager::updateWindowLengths(uint64_t length)
{
   // set current window length
   mCurrentWindow.setLength(length, true);

   // set new window length and reset it
   mNextWindow.setLength(length, false);
   mNextWindow.reset();
   mNextWindow.setStartTime(
      mCurrentWindow.getStartTime() + getHalfWindowLength());

   // if we're in the next window, add items from the current window
   // proportionally
   if(mCurrentWindow.getCurrentTime() > mNextWindow.getStartTime())
   {
      uint64_t passed =
         mCurrentWindow.getCurrentTime() - mNextWindow.getStartTime();
      uint64_t count = mCurrentWindow.getItemsPerMillisecond() * passed;
      mNextWindow.increaseItemCount(count);
   }
}
