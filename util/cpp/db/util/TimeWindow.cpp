/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
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
   mLock.lock();
   {
      // multiply the current rate by the time change and
      // increase the item count accordingly
      increaseItemCount((uint64_t)roundl(getIncreaseRate() * timeChange));
   }
   mLock.unlock();
}
   
void TimeWindow::reset()
{
   mLock.lock();
   {
      mStartTime = 0;
      mItemCount = 0;
      mTimePassed = 0;
      
      // reset the last time that time was added
      mLastAddTime = System::getCurrentMilliseconds();
   }
   mLock.unlock();
}

void TimeWindow::setEqualTo(TimeWindow& window)
{
   mLock.lock();
   {
      mLength = window.getLength();
      mStartTime = window.getStartTime();
      mItemCount = window.getItemCount();
      mTimePassed = window.getTimePassed();
      mLastAddTime = window.mLastAddTime;
   }
   mLock.unlock();
}

double TimeWindow::getIncreaseRateInItemsPerMillisecond()
{
   double rval = 0.0;
   
   mLock.lock();
   {
      // get the rate in items per millisecond
      rval = getItemsPerMillisecond(getItemCount(), getTimePassed());
   }
   mLock.unlock();
   
   return rval;
}

double TimeWindow::getIncreaseRate()
{
   double rval = 0.0;
   
   mLock.lock();
   {
      // get the rate in items per second
      rval = getItemsPerSecond(getItemCount(), getTimePassed());
   }
   mLock.unlock();
   
   return rval;
}

inline void TimeWindow::setLength(uint64_t length)
{
   setLength(length, false);
}

void TimeWindow::setLength(uint64_t length, bool adjust)
{
   mLock.lock();
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
   mLock.unlock();
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
   mLock.lock();
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
   mLock.unlock();
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
   uint64_t rval = 0;
   
   mLock.lock();
   {
      rval = getStartTime() + getTimePassed();
   }
   mLock.unlock();
   
   return rval;
}

uint64_t TimeWindow::getRemainingTime()
{
   uint64_t rval = 0;
   
   mLock.lock();
   {
      rval = getEndTime() - getCurrentTime();
   }
   mLock.unlock();
   
   return rval;
}

bool TimeWindow::isTimeInWindow(uint64_t time)
{
   bool rval = false;
   
   mLock.lock();
   {
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
   }
   mLock.unlock();
   
   return rval;
}

void TimeWindow::increaseItemCount(uint64_t increase)
{
   mLock.lock();
   {
      mItemCount += increase;
   }
   mLock.unlock();
}

void TimeWindow::increaseItemCount(uint64_t increase, uint64_t interval)
{
   mLock.lock();
   {
      increaseItemCount(increase);
      increaseTimePassed(interval);
   }
   mLock.unlock();
}

void TimeWindow::increaseTimePassed(uint64_t time)
{
   mLock.lock();
   {
      mTimePassed += time;
      
      // see if this window has a maximum length
      if(getLength() != 0)
      {
         // cap time passed at the length of the window
         mTimePassed = (mTimePassed < getLength()) ? mTimePassed : getLength();
      }
      
      // update last time that time was added
      mLastAddTime = System::getCurrentMilliseconds();
   }
   mLock.unlock();
}

void TimeWindow::increaseTimePassedWithCurrentTime()
{
   mLock.lock();
   {
      // add the time since the last add time
      increaseTimePassed(System::getCurrentMilliseconds() - mLastAddTime);
   }
   mLock.unlock();
}

inline uint64_t TimeWindow::getItemCount()
{
   return mItemCount;
}

inline uint64_t TimeWindow::getTimePassed()
{
   return mTimePassed;
}

double TimeWindow::getItemsPerMillisecond(double items, double interval)
{
   // items / millisecond -- force interval to 1
   return items / (1.0 > interval ? 1.0 : interval);
}

double TimeWindow::getItemsPerSecond(double items, double interval)
{
   // items / millisecond * 1000 = items / second
   return getItemsPerMillisecond(items, interval) * 1000.0;
}
