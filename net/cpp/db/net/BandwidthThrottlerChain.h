/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_BandwidthThrottlerChain_H
#define db_net_BandwidthThrottlerChain_H

#include "db/net/BandwidthThrottler.h"

#include <vector>

namespace db
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
 * of the chain and the order of the chain must not be changed.
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
    * Sets the rate limit in bytes/second of the first throttler in the chain.
    * A value of 0 indicates no rate limit.
    * 
    * @param rateLimit the bytes/second rate limit to use.
    */
   virtual void setRateLimit(int rateLimit);
   
   /**
    * Returns the rate limit of the first throttler in the chain.
    * 
    * @return the rate limit in bytes/second of the first throttler in
    *         the chain.
    */
   virtual int getRateLimit();
};

} // end namespace net
} // end namespace db
#endif
