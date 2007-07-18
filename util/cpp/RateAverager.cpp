/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "RateAverager.h"
#include "System.h"
#include "Math.h"

using namespace db::rt;
using namespace db::util;

RateAverager::RateAverager(unsigned long long windowLength)
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

void RateAverager::setStartTime(unsigned long long time)
{
   mStartTime = time;
}

void RateAverager::setStopTime(unsigned long long time)
{
   mStopTime = time;
}

void RateAverager::setWindowStartTimes(unsigned long long time)
{
   // set current window start time
   mCurrentWindow.setStartTime(time);
   
   // set next window start time
   mNextWindow.setStartTime(time + getHalfWindowLength());
}

unsigned long long RateAverager::getHalfWindowLength()
{
   return (unsigned long long)Math::round(getWindowLength() / 2.0);
}

void RateAverager::updateWindowLengths(unsigned long long length)
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

void RateAverager::start(unsigned long long time)
{
   lock();
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
   unlock();
}

void RateAverager::stop()
{
   lock();
   {
      stop(getCurrentTime());
   }
   unlock();
}

void RateAverager::stop(unsigned long long time)
{
   lock();
   {
      if(isRunning())
      {
         // no longer running
         mRunning = false;
         
         // set stop time
         setStopTime(time);
      }
   }
   unlock();
}

void RateAverager::restart()
{
   lock();
   {
      restart(getCurrentTime(), 0);
   }
   unlock();
}

void RateAverager::restart(
   unsigned long long stopTime, unsigned long long startTime)
{
   lock();
   {
      stop(stopTime);
      start(startTime);
   }
   unlock();
}

bool RateAverager::isRunning()
{
   bool rval = false;
   
   lock();
   {
      rval = mRunning;
   }
   unlock();
   
   return rval;
}

void RateAverager::addRate(unsigned long long count)
{
   lock();
   {
      addRate(count, System::getCurrentMilliseconds() - mLastAddTime);      
   }
   unlock();
}

void RateAverager::addRate(
   unsigned long long count, unsigned long long interval)
{
   lock();
   {
      // increase the time passed
      mTimePassed = Math::maximum(0, mTimePassed + interval);
      
      // get the remaining time in the current window
      unsigned long long remaining = mCurrentWindow.getRemainingTime();
      
      // see if the interval can be added to the current window without
      // overflowing
      if(interval < remaining)
      {
         // get the overlap time between the current window and the next window
         unsigned long long overlap =
            mCurrentWindow.getCurrentTime() + interval -
            mNextWindow.getCurrentTime();
         
         if(overlap > 0)
         {
            // get the portion of the item count in the overlap
            double rate = TimeWindow::getItemsPerMillisecond(count, interval);
            unsigned long long portion =
               (unsigned long long)Math::round(rate * overlap);
            
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
         unsigned long long overflow = interval - remaining;
         
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
            unsigned long long portion =
               (unsigned long long)Math::round(rate * overflow);
            
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
            unsigned long long startTime =
               getCurrentTime() - getHalfWindowLength() - 1;
            setWindowStartTimes(startTime);
            
            // get the remainder of the interval that will be used
            unsigned long long remainder =
               getCurrentTime() - mCurrentWindow.getStartTime();
            
            // add the portion of the item count to the current window
            double rate = TimeWindow::getItemsPerMillisecond(count, interval);
            unsigned long long portion =
               (unsigned long long)Math::round(rate * remainder);
            mCurrentWindow.increaseItemCount(portion, remainder);
         }
      }
      
      // add items to the total item count
      mTotalItemCount = Math::maximum(0, mTotalItemCount + count);
      
      // update the last rate add time
      mLastAddTime = System::getCurrentMilliseconds();
   }
   unlock();
}

unsigned long long RateAverager::getStartTime()
{
   return mStartTime;
}

unsigned long long RateAverager::getStopTime()
{
   return mStopTime;
}

unsigned long long RateAverager::getCurrentTime()
{
   unsigned long long rval = 0;
   
   lock();
   {
      rval = getStartTime() + getTimePassed();
   }
   unlock();
   
   return rval;
}

unsigned long long RateAverager::getTimePassed()
{
   unsigned long long rval = 0;
   
   lock();
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
   unlock();
   
   return rval;
}

double RateAverager::getCurrentRate()
{
   double rval = 0.0;
   
   lock();
   {
      // make sure that the current time is greater than 0
      if(getCurrentTime() > 0)
      {
         // get the current window rate
         rval = mCurrentWindow.getIncreaseRate();
      }
   }
   unlock();
   
   return rval;      
}

double RateAverager::getTotalRate()
{
   double rval = 0.0;
   
   lock();
   {
      // make sure that the current time is greater than 0
      if(getCurrentTime() > 0)
      {
         // get the rate in items per second
         rval = TimeWindow::getItemsPerSecond(mTotalItemCount, getTimePassed());
      }
   }
   unlock();
   
   return rval;
}

unsigned long long RateAverager::getTotalItemCount()
{
   return mTotalItemCount;
}

void RateAverager::setWindowLength(unsigned long long length)
{
   lock();
   {
      // the window length must be at least two because two windows that
      // are 1/2 of the window length apart are always stored -- and
      // this RateAverager is only accurate to 1 whole millisecond
      length = Math::maximum(2, length);
      
      // update window lengths
      updateWindowLengths(length);
   }
   unlock();
}

unsigned long long RateAverager::getWindowLength()
{
   return mCurrentWindow.getLength();
}

unsigned long long RateAverager::getETA(unsigned long long count)
{
   unsigned long long rval = 0;
   
   lock();
   {
      if(count > 0)
      {
         // multiply the current rate by the count
         rval = (unsigned long long)Math::round(count / getCurrentRate());
      }
   }
   unlock();
   
   return rval;
}
