/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/rt/System.h"

// include NULL
#include <stddef.h>

using namespace db::rt;

uint64_t System::getCurrentMilliseconds()
{
   uint64_t rval = 0;

   // get the current time of day
   struct timeval now;
   gettimeofday(&now, NULL);

   // get total number of milliseconds
   // 1 millisecond is 1000 microseconds
   rval = now.tv_sec * 1000ULL + (uint64_t)(now.tv_usec / 1000.0);

   return rval;
}
