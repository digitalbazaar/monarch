/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_DefaultBandwidthThrottler_H
#define db_net_DefaultBandwidthThrottler_H

#include "monarch/net/BandwidthThrottler.h"
#include "monarch/rt/ExclusiveLock.h"

namespace db
{
namespace net
{

/**
 * A DefaultBandwidthThrottler is a class that is used to throttle bandwidth.
 *
 * @author Dave Longley
 */
class DefaultBandwidthThrottler : public BandwidthThrottler
{
protected:
   /**
    * The rate limit for this BandwidthThrottler.
    */
   volatile int mRateLimit;

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
   volatile uint64_t mAvailableBytes;

   /**
    * A lock for synchronizing the use of this throttler.
    */
   monarch::rt::ExclusiveLock mLock;

public:
   /**
    * Creates a new DefaultBandwidthThrottler.
    *
    * @param rateLimit the bytes/second rate limit to use. A value of 0
    *                  indicates no rate limit.
    */
   DefaultBandwidthThrottler(int rateLimit);

   /**
    * Destructs this DefaultBandwidthThrottler.
    */
   virtual ~DefaultBandwidthThrottler();

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
    * Adds available bytes. This method should be called when not all of the
    * permitted bytes could be obtained and they should be made available
    * again.
    *
    * @param bytes the number of bytes that should be made available.
    */
   virtual void addAvailableBytes(int bytes);

   /**
    * Gets the number of bytes that are currently available.
    *
    * @return the number of bytes that are currently available.
    */
   virtual int getAvailableBytes();

   /**
    * Sets the rate limit in bytes/second. A value of 0 indicates no rate limit.
    *
    * @param rateLimit the bytes/second rate limit to use.
    */
   virtual void setRateLimit(int rateLimit);

   /**
    * Gets the rate limit in bytes/second. A value of 0 indicates no rate limit.
    *
    * @return the rate limit in bytes/second.
    */
   virtual int getRateLimit();

protected:
   /**
    * Resets the window time.
    */
   virtual void resetWindowTime();

   /**
    * Updates the time at which a window for requesting data began --
    * if the number of granted bytes in the current window is high enough.
    */
   virtual void updateWindowTime();

   /**
    * Gets the time at which the current window for requesting
    * data began.
    *
    * @return the time at which the current window for requesting data began.
    */
   virtual uint64_t getWindowTime();

   /**
    * Updates the amount of time (in milliseconds) that must pass before
    * a byte is available.
    */
   virtual void updateAvailableByteTime();

   /**
    * Gets the amount of time (in milliseconds) that must pass before
    * a byte is available. This number is never more than 1000 and never
    * less than 1.
    *
    * @return the amount of time (in milliseconds) that must pass before
    *         a byte is available.
    */
   virtual uint64_t getAvailableByteTime();

   /**
    * Updates the number of bytes that are currently available.
    */
   virtual void updateAvailableBytes();

   /**
    * This method blocks until at least one byte is available without
    * violating the rate limit or until the current thread has been
    * interrupted.
    *
    * @return false if the thread this throttler is waiting on gets
    *         interrupted (with an InterruptedException set), true otherwise.
    */
   virtual bool limitBandwidth();
};

} // end namespace net
} // end namespace db
#endif
