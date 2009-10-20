/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/DefaultBandwidthThrottler.h"

#include "db/rt/System.h"
#include "db/rt/Thread.h"
#include "db/util/Math.h"

#include <math.h>

using namespace db::net;
using namespace db::rt;
using namespace db::util;

DefaultBandwidthThrottler::DefaultBandwidthThrottler(int rateLimit)
{
   // initialize the last request time
   mLastRequestTime = System::getCurrentMilliseconds();

   // initialize the available number of bytes
   mAvailableBytes = 0;

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
         int available = (uint64_t)Math::MAX_INT_VALUE ?
            Math::MAX_INT_VALUE : (int)mAvailableBytes;
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

int DefaultBandwidthThrottler::getAvailableBytes()
{
   int rval = 0;

   mLock.lock();
   {
      if(mRateLimit > 0)
      {
         rval = mAvailableBytes;
      }
      else
      {
         rval = Math::MAX_INT_VALUE;
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

   // reset the bytes already granted in this window
   mBytesGranted = 0;
}

void DefaultBandwidthThrottler::updateWindowTime()
{
   // get the current time
   uint64_t now = System::getCurrentMilliseconds();

   // Cap the number of bytes granted per window at maximum uint value
   // so that there isn't any overflow. This should also be a sufficiently
   // large enough number such that rate calculations aren't affected
   // very often at all.
   if(mBytesGranted > 0xffffffff)
   {
      resetWindowTime();
   }
   else if(now - mLastRequestTime > 3000)
   {
      // If it has been more than 3 seconds since the last request
      // for bytes, then reset the time window.
      //
      // 3 seconds was chosen because the minimum rate limit is
      // 1 byte per second. So if a request is made at that rate limit
      // then at least one byte would be available after one second.
      //
      // If it subsequently actually takes one second to transfer the
      // byte, then the next request would be somewhere shortly after
      // 2 seconds.
      //
      // It is assumed that any request more than a second later
      // involves a different transfer so we shouldn't store up a
      // lot of available bytes (by failing to reset the window) for
      // that transfer artificially bloating its rate.
      //
      // If the assumption fails, and, for instance, it takes more
      // than one second for a single byte to be transferred or bytes
      // are only requested ever so often, then the requester will
      // have to wait a maximum of one second to acquire another byte.
      resetWindowTime();
   }
}

inline uint64_t DefaultBandwidthThrottler::getWindowTime()
{
   return mWindowTime;
}

void DefaultBandwidthThrottler::updateAvailableByteTime()
{
   // the amount of time until a byte is available is 1000 milliseconds
   // divided by the rate in bytes/second, with a minimum of 1 millisecond
   mAvailableByteTime = (uint64_t)roundl(1000. / getRateLimit());
   mAvailableByteTime = (1 > mAvailableByteTime) ? 1 : mAvailableByteTime;
}

inline uint64_t DefaultBandwidthThrottler::getAvailableByteTime()
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

   // while there aren't any available bytes, sleep for the available byte time
   while(rval && mAvailableBytes == 0)
   {
      // unlock to sleep
      mLock.unlock();
      rval = Thread::sleep(getAvailableByteTime());
      mLock.lock();

      // update the number of available bytes
      updateAvailableBytes();
   }

   return rval;
}
