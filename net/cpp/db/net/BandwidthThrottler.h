/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_BandwidthThrottler_H
#define db_net_BandwidthThrottler_H

#include "db/rt/ExclusiveLock.h"

namespace db
{
namespace net
{

/**
 * A BandwidthThrottler is a class that is used to throttle bandwidth.
 * 
 * @author Dave Longley
 */
class BandwidthThrottler
{
protected:
   /**
    * The rate limit for this BandwidthThrottler.
    */
   uint64_t mRateLimit;
   
   /**
    * The time (in milliseconds) at which a window began for requesting data.
    */
   uint64_t mWindowTime;
   
   /**
    * The number of bytes that have been granted in the current window.
    */
   uint64_t mBytesGranted;
   
   /**
    * The last time (in milliseconds) a request was made for bytes.
    */
   uint64_t mLastRequestTime;
   
   /**
    * The amount of time (in milliseconds) that must pass before a byte
    * is available. This number is never more than 1000 and never less
    * than 1.
    */
   uint64_t mAvailableByteTime;
   
   /**
    * The number of available bytes.
    */
   uint64_t mAvailableBytes;
   
   /**
    * A lock for synchronizing the use of this throttler.
    */
   db::rt::ExclusiveLock mLock;
   
   /**
    * Resets the window time.
    */
   void resetWindowTime();
   
   /**
    * Updates the time at which a window for requesting data began --
    * if the number of granted bytes in the current window is high enough.
    */
   void updateWindowTime();
   
   /**
    * Gets the time at which the current window for requesting
    * data began. 
    * 
    * @return the time at which the current window for requesting data began.
    */
   uint64_t getWindowTime();
   
   /**
    * Updates the amount of time (in milliseconds) that must pass before
    * a byte is available.
    */
   void updateAvailableByteTime();
   
   /**
    * Gets the amount of time (in milliseconds) that must pass before
    * a byte is available. This number is never more than 1000 and never
    * less than 1.
    * 
    * @return the amount of time (in milliseconds) that must pass before
    *         a byte is available.
    */
   uint64_t getAvailableByteTime();
   
   /**
    * Updates the number of bytes that are currently available.
    */
   void updateAvailableBytes();
   
   /**
    * Gets the number of bytes that are currently available.
    * 
    * @return the number of bytes that are currently available.
    */
   uint64_t getAvailableBytes();
   
   /**
    * This method blocks until at least one byte is available without
    * violating the rate limit or until the current thread has been
    * interrupted.
    * 
    * @return false if the thread this throttler is waiting on gets
    *         interrupted (with an InterruptedException set), true otherwise.
    */
   bool limitBandwidth();
   
public:
   /**
    * Creates a new BandwidthThrottler.
    * 
    * @param rateLimit the bytes/second rate limit to use. A value of 0
    *                  indicates no rate limit.
    */
   BandwidthThrottler(uint64_t rateLimit);
   
   /**
    * Destructs this BandwidthThrottler.
    */
   virtual ~BandwidthThrottler();
   
   /**
    * Requests the passed number of bytes from this throttler. This method
    * will block until at least one byte can be sent without violating
    * the rate limit or if the current thread has been interrupted.
    * 
    * @param count the number of bytes requested.
    * @param permitted set to the number of bytes permitted to send.
    * 
    * @return false if the thread this throttler is waiting on gets
    *         interrupted (with an Exception set), true otherwise.
    */
   virtual bool requestBytes(int count, int& permitted);
   
   /**
    * Sets the rate limit in bytes/second. A value of 0 indicates no rate limit.
    * 
    * @param rateLimit the bytes/second rate limit to use.
    */
   virtual void setRateLimit(uint64_t rateLimit);
   
   /**
    * Gets the rate limit in bytes/second. A value of 0 indicates no rate limit.
    * 
    * @return the rate limit in bytes/second.
    */
   virtual uint64_t getRateLimit();
};

} // end namespace net
} // end namespace db
#endif
