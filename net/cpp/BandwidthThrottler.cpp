/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "BandwidthThrottler.h"
#include "Math.h"
#include "System.h"
#include "Thread.h"

using namespace db::net;
using namespace db::rt;
using namespace db::util;

BandwidthThrottler::BandwidthThrottler(unsigned long long rateLimit)
{
   // initialize the last request time
   mLastRequestTime = System::getCurrentMilliseconds();
   
   // initialize the available number of bytes
   mAvailableBytes = 0;
   
   // set the rate limit (will also reset the window time if necessary)
   setRateLimit(rateLimit);
}

BandwidthThrottler::~BandwidthThrottler()
{
}

void BandwidthThrottler::resetWindowTime()
{
   // set the current window time
   mWindowTime = System::getCurrentMilliseconds();
      
   // reset the bytes already granted in this window
   mBytesGranted = 0;
}

void BandwidthThrottler::updateWindowTime()
{
   // get the current time
   unsigned long long now = System::getCurrentMilliseconds();
   
   // Cap the number of bytes granted per window at Integer.MAX_VALUE
   // so that there isn't any overflow. This should also be a
   // sufficiently large enough number such that rate calculations
   // aren't affected very often at all.
   if(mBytesGranted > Math::MAX_UINT_VALUE)
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

unsigned long long BandwidthThrottler::getWindowTime()
{
   return mWindowTime;
}

void BandwidthThrottler::updateAvailableByteTime()
{
   // the amount of time until a byte is available is 1000 milliseconds
   // divided by the rate in bytes/second, with a minimum of 1 millisecond
   mAvailableByteTime = (unsigned long long)Math::round(1000. / getRateLimit());
   mAvailableByteTime = Math::maximum(1, mAvailableByteTime);
}

unsigned long long BandwidthThrottler::getAvailableByteTime()
{
   return mAvailableByteTime;
}

void BandwidthThrottler::updateAvailableBytes()
{
   // get the passed time in the current window
   double passedTime = System::getCurrentMilliseconds() - getWindowTime();
   
   // determine how many bytes are available given the passed time --
   // use the floor so as not to go over the rate limit
   mAvailableBytes = (unsigned long long)Math::floor(
      passedTime / 1000. * getRateLimit());
   
   // subtract the number of bytes already granted in this window
   mAvailableBytes -= mBytesGranted;
   mAvailableBytes = Math::maximum(0, mAvailableBytes);
}

unsigned long long BandwidthThrottler::getAvailableBytes()
{
   return mAvailableBytes;
}

InterruptedException* BandwidthThrottler::limitBandwidth()
{
   InterruptedException* rval = NULL;
   
   // update the window time
   updateWindowTime();

   // update the number of available bytes
   updateAvailableBytes();
   
   // while there aren't any available bytes, sleep for the
   // available byte time
   while(rval == NULL && getAvailableBytes() == 0)
   {
      // sleep
      rval = Thread::sleep(getAvailableByteTime());
      
      // update the number of available bytes
      updateAvailableBytes();
   }
   
   return rval;
}

InterruptedException* BandwidthThrottler::requestBytes(
   unsigned int count, unsigned int& permitted)
{
   InterruptedException* rval = NULL;
   
   // no bytes permitted yet
   permitted = 0;
   
   lock();
   {
      if(getRateLimit() > 0)
      {
         // limit the bandwidth
         rval = limitBandwidth();
         
         // get the available bytes
         permitted = Math::minimum(getAvailableBytes(), count);
         
         // increment the bytes granted
         mBytesGranted += permitted;
         
         // update last request time
         mLastRequestTime = System::getCurrentMilliseconds();
      }
      else
      {
         // no rate limit, return the count
         permitted = count;
      }
   }
   unlock();
   
   return rval;
}

void BandwidthThrottler::setRateLimit(unsigned long long rateLimit)
{
   lock();
   {
      // set new rate limit
      mRateLimit = Math::maximum(0, rateLimit);
      
      if(mRateLimit > 0)
      {
         // reset the window time
         resetWindowTime();
         
         // update the available byte time
         updateAvailableByteTime();
      }
   }
   unlock();
}

unsigned long long BandwidthThrottler::getRateLimit()
{
   return mRateLimit;
}
