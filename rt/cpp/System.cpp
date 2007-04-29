/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "System.h"

using namespace db::rt;

unsigned long long System::getCurrentMilliseconds()
{
   unsigned long long rval = 0;
   
   // get the current time of day
   struct timeval now;
   gettimeofday(&now, NULL);
   
   // get total number of milliseconds
   // 1 millisecond is 1000 microseconds
   rval = now.tv_sec * 1000LL + (unsigned long)(now.tv_usec / 1000.0);
   
   return rval;
}
