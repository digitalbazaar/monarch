/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/TimeWindow.h"
#include "db/rt/System.h"

#include <math.h>

using namespace db::rt;
using namespace db::util;

TimeWindow::TimeWindow(unsigned long long length)
{
   // set the window length and reset
   setLength(length);
   reset();
}

TimeWindow::~TimeWindow()
{
}

void TimeWindow::adjustItemCount(unsigned long long timeChange)
{
   lock();
   {
      // multiply the current rate by the time change and
      // increase the item count accordingly
      increaseItemCount((unsigned long long)
         roundl(getIncreaseRate() * timeChange));
   }
   unlock();
}
   
void TimeWindow::reset()
{
   lock();
   {
      mStartTime = 0;
      mItemCount = 0;
      mTimePassed = 0;
      
      // reset the last time that time was added
      mLastAddTime = System::getCurrentMilliseconds();
   }
   unlock();
}

void TimeWindow::setEqualTo(TimeWindow& window)
{
   lock();
   {
      mLength = window.getLength();
      mStartTime = window.getStartTime();
      mItemCount = window.getItemCount();
      mTimePassed = window.getTimePassed();
      mLastAddTime = window.mLastAddTime;
   }
   unlock();
}

double TimeWindow::getIncreaseRateInItemsPerMillisecond()
{
   double rval = 0.0;
   
   lock();
   {
      // get the rate in items per millisecond
      rval = getItemsPerMillisecond(getItemCount(), getTimePassed());
   }
   unlock();
   
   return rval;
}

double TimeWindow::getIncreaseRate()
{
   double rval = 0.0;
   
   lock();
   {
      // get the rate in items per second
      rval = getItemsPerSecond(getItemCount(), getTimePassed());
   }
   unlock();
   
   return rval;
}

void TimeWindow::setLength(unsigned long long length)
{
   setLength(length, false);
}

void TimeWindow::setLength(unsigned long long length, bool adjust)
{
   lock();
   {
      // if adjusting the item count
      if(adjust)
      {
         // get the time change
         long timeChange = length - mLength;
         
         // adjust the item count
         adjustItemCount(timeChange);
      }
      
      // set new length
      mLength = length;
   }
   unlock();
}

unsigned long long TimeWindow::getLength()
{
   return mLength;
}

void TimeWindow::setStartTime(unsigned long long time)
{
   setStartTime(time, false);
}

void TimeWindow::setStartTime(unsigned long long time, bool adjust)
{
   lock();
   {
      // if adjusting the item count
      if(adjust)
      {
         // get the time change
         long timeChange = time - mStartTime;
         
         // adjust the item count
         adjustItemCount(timeChange);
      }
      
      // set new start time
      mStartTime = time;
   }
   unlock();
}

unsigned long long TimeWindow::getStartTime()
{
   return mStartTime;
}

unsigned long long TimeWindow::getEndTime()
{
   unsigned long long rval = 0;
   
   if(getLength() > 0)
   {
      rval = getStartTime() + getLength() - 1;
   }
   
   return rval;
}

unsigned long long TimeWindow::getCurrentTime()
{
   unsigned long long rval = 0;
   
   lock();
   {
      rval = getStartTime() + getTimePassed();
   }
   unlock();
   
   return rval;
}

unsigned long long TimeWindow::getRemainingTime()
{
   unsigned long long rval = 0;
   
   lock();
   {
      rval = getEndTime() - getCurrentTime();
   }
   unlock();
   
   return rval;
}

bool TimeWindow::isTimeInWindow(unsigned long long time)
{
   bool rval = false;
   
   lock();
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
   unlock();
   
   return rval;
}

void TimeWindow::increaseItemCount(unsigned long long increase)
{
   lock();
   {
      mItemCount += increase;
   }
   unlock();
}

void TimeWindow::increaseItemCount(
   unsigned long long increase, unsigned long long interval)
{
   lock();
   {
      increaseItemCount(increase);
      increaseTimePassed(interval);
   }
   unlock();
}

void TimeWindow::increaseTimePassed(unsigned long long time)
{
   lock();
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
   unlock();
}

void TimeWindow::increaseTimePassedWithCurrentTime()
{
   lock();
   {
      // add the time since the last add time
      increaseTimePassed(System::getCurrentMilliseconds() - mLastAddTime);
   }
   unlock();
}

unsigned long long TimeWindow::getItemCount()
{
   return mItemCount;
}

unsigned long long TimeWindow::getTimePassed()
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
