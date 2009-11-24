/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_BandwidthThrottler_H
#define db_net_BandwidthThrottler_H

#include "db/rt/Collectable.h"

namespace db
{
namespace net
{

/**
 * A BandwidthThrottler is an interface that is used to throttle bandwidth.
 *
 * @author Dave Longley
 */
class BandwidthThrottler
{
public:
   /**
    * Creates a new BandwidthThrottler.
    */
   BandwidthThrottler() {};

   /**
    * Destructs this BandwidthThrottler.
    */
   virtual ~BandwidthThrottler() {};

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
   virtual bool requestBytes(int count, int& permitted) = 0;

   /**
    * Adds available bytes. This method should be called when not all of the
    * permitted bytes could be obtained and they should be made available
    * again.
    *
    * @param bytes the number of bytes that should be made available.
    */
   virtual void addAvailableBytes(int bytes) = 0;

   /**
    * Gets the number of bytes that are currently available.
    *
    * @return the number of bytes that are currently available.
    */
   virtual int getAvailableBytes() = 0;

   /**
    * Sets the rate limit in bytes/second. A value of 0 indicates no rate limit.
    *
    * @param rateLimit the bytes/second rate limit to use.
    */
   virtual void setRateLimit(int rateLimit) = 0;

   /**
    * Gets the rate limit in bytes/second. A value of 0 indicates no rate limit.
    *
    * @return the rate limit in bytes/second.
    */
   virtual int getRateLimit() = 0;
};

// typedef for a reference counted BandwidthThrottler
typedef db::rt::Collectable<BandwidthThrottler> BandwidthThrottlerRef;

} // end namespace net
} // end namespace db
#endif
