/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/fiber/FiberMessageCenter.h"

#include <algorithm>

using namespace std;
using namespace monarch::fiber;
using namespace monarch::rt;

FiberMessageCenter::FiberMessageCenter()
{
}

FiberMessageCenter::~FiberMessageCenter()
{
}

void FiberMessageCenter::registerFiber(MessagableFiber* fiber)
{
   // lock to add fiber to map
   mMessageLock.lockExclusive();
   {
      mFibers.insert(make_pair(fiber->getId(), fiber));
   }
   mMessageLock.unlockExclusive();
}

void FiberMessageCenter::unregisterFiber(MessagableFiber* fiber)
{
   // lock to remove fiber from map
   mMessageLock.lockExclusive();
   {
      mFibers.erase(fiber->getId());
   }
   mMessageLock.unlockExclusive();
}

bool FiberMessageCenter::sendMessage(FiberId id, DynamicObject& msg)
{
   bool rval = false;

   // get shared lock to deliver message
   mMessageLock.lockShared();
   {
      FiberMap::iterator i = mFibers.find(id);
      if(i != mFibers.end())
      {
         i->second->addMessage(msg);
         rval = true;
      }
   }
   mMessageLock.unlockShared();

   return rval;
}
