/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/rt/ExclusiveLock.h"

#include "db/rt/System.h"
#include "db/rt/Thread.h"

using namespace db::rt;

ExclusiveLock::ExclusiveLock()
{
}

ExclusiveLock::~ExclusiveLock()
{
}

inline void ExclusiveLock::lock()
{
   mMonitor.enter();
}

inline void ExclusiveLock::unlock()
{
   mMonitor.exit();
}

inline void ExclusiveLock::notify()
{
   mMonitor.notify();
}

inline void ExclusiveLock::notifyAll()
{
   mMonitor.notifyAll();
}

inline bool ExclusiveLock::wait(uint32_t timeout)
{
   // instruct the current thread to wait to enter this Object's monitor
   return Thread::waitToEnter(&mMonitor, timeout);
}

bool ExclusiveLock::wait(uint32_t& timeout, bool* condition, bool stop)
{
   bool rval = true;
   
   uint32_t remaining = timeout;
   uint64_t st = System::getCurrentMilliseconds();
   uint64_t et;
   uint64_t dt;
   while(rval && (*condition != stop) && (timeout == 0 || remaining > 0))
   {
      rval = wait(remaining);
      if(rval && timeout > 0)
      {
         // decrement remaining time
         et = System::getCurrentMilliseconds();
         dt = et - st;
         remaining = (dt > remaining ? 0 : remaining - dt);
         st = et;
      }
   }
   timeout = remaining;
   
   return rval;
}
