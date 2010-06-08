/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_LIMIT_MACROS

#include "monarch/net/DefaultBandwidthThrottler.h"

#include "monarch/rt/System.h"
#include "monarch/rt/Thread.h"
#include "monarch/util/Math.h"

#include <cmath>

using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;

DefaultBandwidthThrottler::DefaultBandwidthThrottler(int rateLimit) :
   mLastRequestTime(0),
   mAvailableBytes(0),
   mWaiters(0)
{
   // set the rate limit (will also reset the window time if necessary)
   setRateLimit(rateLimit);
}

DefaultBandwidthThrottler::~DefaultBandwidthThrottler()
{
}

bool DefaultBandwidthThrottler::requestBytes(int count, int& permitted)
{
   bool rval = true;

   if(getRateLimit() > 0)
   {
      mLock.lock();
      {
         // limit the bandwidth
         rval = limitBandwidth();

         // get the available bytes
         int available = (mAvailableBytes >= (uint64_t)INT32_MAX) ?
            INT32_MAX : (int)mAvailableBytes;
         permitted = (available > count ? count : available);

         // increment the bytes granted
         mBytesGranted += permitted;

         // subtract the number of permitted bytes from the available bytes
         mAvailableBytes -= permitted;

         // update last request time
         mLastRequestTime = System::getCurrentMilliseconds();
      }
      mLock.unlock();
   }
   else
   {
      // no rate limit, return the count
      permitted = count;
   }

   return rval;
}

void DefaultBandwidthThrottler::addAvailableBytes(int bytes)
{
   mLock.lock();
   {
      // not all bytes were read/written/used, replace bytes for next call
      mBytesGranted -= bytes;
      mAvailableBytes += bytes;

      // notify any waiters
      mLock.notifyAll();
   }
   mLock.unlock();
}

int DefaultBandwidthThrottler::getAvailableBytes()
{
   int rval = 0;

   mLock.lock();
   {
      if(mRateLimit > 0 && mAvailableBytes < (uint64_t)INT32_MAX)
      {
         rval = mAvailableBytes;
      }
      else
      {
         rval = INT32_MAX;
      }
   }
   mLock.unlock();

   return rval;
}

void DefaultBandwidthThrottler::setRateLimit(int rateLimit)
{
   mLock.lock();
   {
      // set new rate limit
      mRateLimit = rateLimit;
      if(mRateLimit > 0)
      {
         // reset the window time
         resetWindowTime();

         // update the available byte time
         updateAvailableByteTime();
      }

      // notify any waiters
      mLock.notifyAll();
   }
   mLock.unlock();
}

int DefaultBandwidthThrottler::getRateLimit()
{
   return mRateLimit;
}

void DefaultBandwidthThrottler::resetWindowTime()
{
   // set the current window time
   mWindowTime = System::getCurrentMilliseconds();

   // reset bytes already granted, available bytes, and request time
   mBytesGranted = 0;
   mAvailableBytes = 0;
   mLastRequestTime = 0;
}

void DefaultBandwidthThrottler::updateWindowTime()
{
   /* Cap the number of bytes granted per window at maximum uint32 value
      so that there isn't any overflow. This should also be a sufficiently
      large enough number such that rate calculations aren't affected
      very often at all.

      If it has been more than 100 milliseconds since the last request for
      bytes and there are no waiters, then reset the time window.

      The window time is reset in this case because it is assumed that the
      request is for a new transfer and we do not want to accumulate a lot of
      available bytes while no transfers are taking place -- as that would lead
      to granting a burst of too many bytes at once and going over the rate
      limit. This reset will not take place if any thread is already waiting
      for bytes to become available, so there is no risk of an infinite reset
      loop.
   */
   uint64_t now = System::getCurrentMilliseconds();
   if(mBytesGranted > (uint64_t)INT32_MAX ||
      (mWaiters == 0 && (now - mLastRequestTime) > 100))
   {
      resetWindowTime();
   }
}

inline uint64_t DefaultBandwidthThrottler::getWindowTime()
{
   return mWindowTime;
}

inline void DefaultBandwidthThrottler::updateAvailableByteTime()
{
   // the amount of time until a byte is available is 1000 milliseconds
   // divided by the rate in bytes/second, with a minimum of 1 millisecond
   mAvailableByteTime = (uint32_t)round(1000. / getRateLimit());
   mAvailableByteTime = (1 > mAvailableByteTime) ? 1 : mAvailableByteTime;
}

inline uint32_t DefaultBandwidthThrottler::getAvailableByteTime()
{
   return mAvailableByteTime;
}

void DefaultBandwidthThrottler::updateAvailableBytes()
{
   // get the passed time in the current window
   double passedTime = System::getCurrentMilliseconds() - getWindowTime();

   // determine how many bytes are available given the passed time --
   // use the floor so as not to go over the rate limit
   mAvailableBytes = (uint64_t)floorl(passedTime / 1000. * getRateLimit());

   // subtract the number of bytes already granted in this window
   mAvailableBytes = (mBytesGranted > mAvailableBytes) ?
      0 : mAvailableBytes - mBytesGranted;
}

bool DefaultBandwidthThrottler::limitBandwidth()
{
   bool rval = true;

   // update the window time
   updateWindowTime();

   // update the number of available bytes
   updateAvailableBytes();

   // thread will wait for available bytes now
   mWaiters++;

   // while there aren't any available bytes, wait for the available byte time
   uint32_t avt;
   while(rval && mRateLimit > 0 && mAvailableBytes == 0)
   {
      // wait for an available byte
      avt = getAvailableByteTime();
      mLock.wait(avt);

      // update the number of available bytes
      updateAvailableBytes();
   }

   // thread finished waiting
   mWaiters--;

   return rval;
}
