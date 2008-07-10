/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
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
   // reset start and stop times
   setStartTime(0);
   setStopTime(0);
   
   // reset time passed
   mTimePassed = 0;
   
   // reset last time a rate was added
   mLastAddTime = System::getCurrentMilliseconds();
   
   // reset the windows
   mCurrentWindow.reset();
   mNextWindow.reset();
   
   // reset total item count
   mTotalItemCount = 0;
}

inline void RateAverager::setStartTime(uint64_t time)
{
   mStartTime = time;
}

inline void RateAverager::setStopTime(uint64_t time)
{
   mStopTime = time;
}

void RateAverager::setWindowStartTimes(uint64_t time)
{
   // set current window start time
   mCurrentWindow.setStartTime(time);
   
   // set next window start time
   mNextWindow.setStartTime(time + getHalfWindowLength());
}

uint64_t RateAverager::getHalfWindowLength()
{
   return (uint64_t)roundl(getWindowLength() / 2.0);
}

void RateAverager::updateWindowLengths(uint64_t length)
{
   // set the current window length, adjusting items
   mCurrentWindow.setLength(length, true);
   
   // set the next window length, adjusting items
   mNextWindow.setLength(length, true);
   
   // update the next window start time
   mNextWindow.setStartTime(
      mCurrentWindow.getStartTime() + getHalfWindowLength());
}

void RateAverager::moveCurrentWindow()
{
   // set the current window to the next window
   mCurrentWindow.setEqualTo(mNextWindow);
   
   // reset the next window
   mNextWindow.reset();
   
   // set the next window start time
   mNextWindow.setStartTime(
      mCurrentWindow.getStartTime() + getHalfWindowLength());
}

void RateAverager::start(uint64_t time)
{
   mLock.lock();
   {
      if(!isRunning())
      {
         // reset
         reset();
   
         // now running
         mRunning = true;
         
         // set start time
         setStartTime(time);
         
         // set window start times
         setWindowStartTimes(time);
      }
   }
   mLock.unlock();
}

void RateAverager::stop()
{
   mLock.lock();
   {
      stop(getCurrentTime());
   }
   mLock.unlock();
}

void RateAverager::stop(uint64_t time)
{
   mLock.lock();
   {
      if(isRunning())
      {
         // no longer running
         mRunning = false;
         
         // set stop time
         setStopTime(time);
      }
   }
   mLock.unlock();
}

void RateAverager::restart()
{
   mLock.lock();
   {
      restart(getCurrentTime(), 0);
   }
   mLock.unlock();
}

void RateAverager::restart(uint64_t stopTime, uint64_t startTime)
{
   mLock.lock();
   {
      stop(stopTime);
      start(startTime);
   }
   mLock.unlock();
}

bool RateAverager::isRunning()
{
   bool rval = false;
   
   mLock.lock();
   {
      rval = mRunning;
   }
   mLock.unlock();
   
   return rval;
}

void RateAverager::addRate(uint64_t count)
{
   mLock.lock();
   {
      addRate(count, System::getCurrentMilliseconds() - mLastAddTime);      
   }
   mLock.unlock();
}

void RateAverager::addRate(uint64_t count, uint64_t interval)
{
   mLock.lock();
   {
      // increase the time passed
      mTimePassed += interval;
      
      // get the remaining time in the current window
      uint64_t remaining = mCurrentWindow.getRemainingTime();
      
      // see if the interval can be added to the current window without
      // overflowing
      if(interval < remaining)
      {
         // get the overlap time between the current window and the next window
         uint64_t overlap =
            mCurrentWindow.getCurrentTime() + interval -
            mNextWindow.getCurrentTime();
         
         if(overlap > 0)
         {
            // get the portion of the item count in the overlap
            double rate = TimeWindow::getItemsPerMillisecond(count, interval);
            uint64_t portion = (uint64_t)roundl(rate * overlap);
            
            // increase the next window count and time
            mNextWindow.increaseItemCount(portion, overlap);
         }
         
         // add the count and interval to the current window
         mCurrentWindow.increaseItemCount(count, interval);
      }
      else
      {
         // there is overflow, so we'll be moving windows
         
         // get the overflow of the interval that cannot be added to
         // the current window
         uint64_t overflow = interval - remaining;
         
         // add the amount of time it takes to get the next window up to
         // the end of the current window
         overflow += mCurrentWindow.getEndTime() - mNextWindow.getCurrentTime();
         
         // get the remaining time in the next window
         remaining = mNextWindow.getRemainingTime();
         
         // see if the overflow can be added to the next window
         if(overflow < remaining)
         {
            // get the portion of the item count in the overflow
            double rate = TimeWindow::getItemsPerMillisecond(count, interval);
            uint64_t portion = (uint64_t)roundl(rate * overflow);
            
            // increase the next window count and time
            mNextWindow.increaseItemCount(portion, overflow);
            
            // move the current window
            moveCurrentWindow();
         }
         else
         {
            // reset the windows
            mCurrentWindow.reset();
            mNextWindow.reset();
            
            // set the current window start time to half of a window - 1
            // before the the current time
            uint64_t startTime = getCurrentTime() - getHalfWindowLength() - 1;
            setWindowStartTimes(startTime);
            
            // get the remainder of the interval that will be used
            uint64_t remainder =
               getCurrentTime() - mCurrentWindow.getStartTime();
            
            // add the portion of the item count to the current window
            double rate = TimeWindow::getItemsPerMillisecond(count, interval);
            uint64_t portion = (uint64_t)roundl(rate * remainder);
            mCurrentWindow.increaseItemCount(portion, remainder);
         }
      }
      
      // add items to the total item count
      mTotalItemCount += count;
      
      // update the last rate add time
      mLastAddTime = System::getCurrentMilliseconds();
   }
   mLock.unlock();
}

inline uint64_t RateAverager::getStartTime()
{
   return mStartTime;
}

inline uint64_t RateAverager::getStopTime()
{
   return mStopTime;
}

inline uint64_t RateAverager::getCurrentTime()
{
   uint64_t rval = 0;
   
   mLock.lock();
   {
      rval = getStartTime() + getTimePassed();
   }
   mLock.unlock();
   
   return rval;
}

inline uint64_t RateAverager::getTimePassed()
{
   uint64_t rval = 0;
   
   mLock.lock();
   {
      if(isRunning())
      {
         rval = mTimePassed;
      }
      else
      {
         rval = getStopTime() - getStartTime();
      }
   }
   mLock.unlock();
   
   return rval;
}

double RateAverager::getCurrentRate()
{
   double rval = 0.0;
   
   mLock.lock();
   {
      // make sure that the current time is greater than 0
      if(getCurrentTime() > 0)
      {
         // get the current window rate
         rval = mCurrentWindow.getIncreaseRate();
      }
   }
   mLock.unlock();
   
   return rval;      
}

double RateAverager::getTotalRate()
{
   double rval = 0.0;
   
   mLock.lock();
   {
      // make sure that the current time is greater than 0
      if(getCurrentTime() > 0)
      {
         // get the rate in items per second
         rval = TimeWindow::getItemsPerSecond(mTotalItemCount, getTimePassed());
      }
   }
   mLock.unlock();
   
   return rval;
}

inline uint64_t RateAverager::getTotalItemCount()
{
   return mTotalItemCount;
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

uint64_t RateAverager::getETA(uint64_t count)
{
   uint64_t rval = 0;
   
   mLock.lock();
   {
      if(count > 0)
      {
         // multiply the current rate by the count
         rval = (uint64_t)roundl(count / getCurrentRate());
      }
   }
   mLock.unlock();
   
   return rval;
}
