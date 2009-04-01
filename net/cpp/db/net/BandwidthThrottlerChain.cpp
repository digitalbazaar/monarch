/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/BandwidthThrottlerChain.h"

using namespace db::net;
using namespace db::rt;

BandwidthThrottlerChain::BandwidthThrottlerChain()
{
}

BandwidthThrottlerChain::~BandwidthThrottlerChain()
{
}

void BandwidthThrottlerChain::add(BandwidthThrottler* bt)
{
   mChain.push_back(bt);
}

void BandwidthThrottlerChain::add(BandwidthThrottlerRef& bt)
{
   // save reference
   mThrottlerRefList.push_back(bt);
   add(&(*bt));
}

bool BandwidthThrottlerChain::requestBytes(int count, int& permitted)
{
   bool rval = true;
   
   if(!mChain.empty())
   {
      // request bytes from each throttler in the chain
      for(ThrottlerChain::iterator i = mChain.begin();
          rval && i != mChain.end(); i++)
      {
         rval = (*i)->requestBytes(count, permitted);
      }
   }
   
   return rval;
}

void BandwidthThrottlerChain::setRateLimit(int rateLimit)
{
   if(!mChain.empty())
   {
      mChain.front()->setRateLimit(rateLimit);
   }
}

int BandwidthThrottlerChain::getRateLimit()
{
   return (mChain.empty() ? 0 : mChain.front()->getRateLimit());
}
