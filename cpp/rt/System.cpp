/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include "monarch/rt/System.h"

// include NULL
#include <stddef.h>

#ifdef WIN32
#include <windows.h>
#elif MACOS
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif

using namespace monarch::rt;

uint64_t System::getCurrentMilliseconds()
{
   uint64_t rval = 0;

   // FIXME: consider using clock_gettime(), at least on *nix
   // get the current time of day
   struct timeval now;
   gettimeofday(&now, NULL);

   // get total number of milliseconds
   // 1 millisecond is 1000 microseconds
   rval = now.tv_sec * UINT64_C(1000) + (uint64_t)(now.tv_usec / 1000.0);

   return rval;
}

uint32_t System::getCpuCoreCount()
{
#ifdef WIN32
   SYSTEM_INFO sysinfo;
   GetSystemInfo(&sysinfo);
   return sysinfo.dwNumberOfProcessors;
#elif MACOS
   int nm[2];
   size_t len = 4;
   uint32_t count;

   // names to look up
   nm[0] = CTL_HW;
   nm[1] = HW_AVAILCPU;
   sysctl(nm, 2, &count, &len, NULL, 0);

   // handle error case
   if(count < 1)
   {
      // look up HW_NCPU instead
      nm[1] = HW_NCPU;
      sysctl(nm, 2, &count, &len, NULL, 0);
      if(count < 1)
      {
         // default to 1 cpu/core
         count = 1;
      }
   }

   return count;
#else
   return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}
