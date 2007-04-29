#include "BandwidthThrottler.h"
#include "Math.h"
#include "System.h"

using namespace db::net;
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
   mAvailableByteTime = Math::max(1, mAvailableByteTime);
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
   mAvailableBytes = Math::max(0, mAvailableBytes);
}

unsigned long long BandwidthThrottler::getAvailableBytes()
{
   return mAvailableBytes;
}

void BandwidthThrottler::limitBandwidth()
{
   // update the window time
   updateWindowTime();

   // update the number of available bytes
   updateAvailableBytes();
   
   // while there aren't any available bytes, sleep for the
   // available byte time
   while(getAvailableBytes() == 0)
   {
      // FIXME: perform sleep here
      //Thread.sleep(getAvailableByteTime());
      
      // update the number of available bytes
      updateAvailableBytes();
   }
}

// FIXME: this method was synchronized in java
unsigned int BandwidthThrottler::requestBytes(unsigned int count)
{
   unsigned int rval = 0;
   
   if(getRateLimit() > 0)
   {
      // limit the bandwidth
      limitBandwidth();
      
      // get the available bytes
      rval = Math::min(getAvailableBytes(), count);
      
      // increment the bytes granted
      mBytesGranted += rval;
      
      // update last request time
      mLastRequestTime = System::getCurrentMilliseconds();
   }
   else
   {
      // no rate limit, return the count
      rval = count;
   }
   
   return rval;
}

// FIXME: this method was synchronized in java
void BandwidthThrottler::setRateLimit(unsigned long long rateLimit)
{
   // set new rate limit
   mRateLimit = Math::max(0, rateLimit);
   
   if(mRateLimit > 0)
   {
      // reset the window time
      resetWindowTime();
      
      // update the available byte time
      updateAvailableByteTime();
   }
}

unsigned long long BandwidthThrottler::getRateLimit()
{
   return mRateLimit;
}
