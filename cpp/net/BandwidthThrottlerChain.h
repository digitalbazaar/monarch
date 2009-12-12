/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_net_BandwidthThrottlerChain_H
#define monarch_net_BandwidthThrottlerChain_H

#include "monarch/net/BandwidthThrottler.h"

#include <vector>

namespace monarch
{
namespace net
{

/**
 * A BandwidthThrottlerChain is used to chain together multiple bandwidth
 * throttlers such that a series of rate limits are imposed rather than
 * just a single limit. The first rate limit must pass before the second
 * rate limit is checked, and so forth.
 *
 * This class is useful in a system where various levels of rate limiting
 * granularity are needed. For instance, a system might have a "global" level
 * bandwidth throttler that is shared for all transfers, but the users of that
 * system might each have their own bandwidth throttler that limits them
 * further. The system might go even further to allow each individual user to
 * limit their individual tranfers by introducing a third bandwidth throttler
 * into the chain, etc.
 *
 * Note: In order to improve performance and avoid deadlock, a chain must
 * not be altered while it is in use. The individual rate limits of the
 * throttlers in the chain may be changed at any time, but the members
 * of the chain and the order of the chain must not be changed. It is expected,
 * although not enforced, that any throttler in the chain will have a higher
 * rate limit (allowing more data to pass over a unit of time) than any
 * throttler that was added after it.
 *
 * @author Dave Longley
 */
class BandwidthThrottlerChain : public BandwidthThrottler
{
protected:
   /**
    * The chain of bandwidth throttlers.
    */
   typedef std::vector<BandwidthThrottler*> ThrottlerChain;
   ThrottlerChain mChain;

   /**
    * A list of bandwidth throttler references.
    */
   typedef std::vector<BandwidthThrottlerRef> ThrottlerRefList;
   ThrottlerRefList mThrottlerRefList;

public:
   /**
    * Creates a new BandwidthThrottlerChain.
    */
   BandwidthThrottlerChain();

   /**
    * Destructs this BandwidthThrottlerChain.
    */
   virtual ~BandwidthThrottlerChain();

   /**
    * Adds a bandwidth throttler to the end of the chain.
    *
    * @param bt the bandwidth throttler to add.
    */
   virtual void add(BandwidthThrottler* bt);
   virtual void add(BandwidthThrottlerRef& bt);

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
    * Gets the number of bytes that are currently available from the last
    * throttler in the chain (typically the most limited throttler).
    *
    * @return the number of bytes that are currently available.
    */
   virtual int getAvailableBytes();

   /**
    * Sets the rate limit in bytes/second of the last throttler (typically the
    * most limited throttler) in the chain. A value of 0 indicates no rate
    * limit.
    *
    * @param rateLimit the bytes/second rate limit to use.
    */
   virtual void setRateLimit(int rateLimit);

   /**
    * Returns the rate limit of the last throttler (typically the most limited
    * throttler) in the chain.
    *
    * @return the rate limit in bytes/second of the last throttler in
    *         the chain.
    */
   virtual int getRateLimit();
};

} // end namespace net
} // end namespace monarch
#endif
