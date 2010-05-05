/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/TimeZone.h"

#include "monarch/rt/System.h"

#include <cstring>

using namespace std;
using namespace monarch::rt;
using namespace monarch::util;

TimeZone::TimeZone(time_t minutesWest) :
   mMinutesWest(minutesWest)
{
}

TimeZone::~TimeZone()
{
}

inline time_t TimeZone::getMinutesWest()
{
   return mMinutesWest;
}

TimeZone TimeZone::getTimeZone(const char* tz)
{
   TimeZone rval;

   if(strcasecmp(tz, "GMT") == 0 || strcasecmp(tz, "UTC") == 0)
   {
      rval.mMinutesWest = 0;
   }
   else if(strcasecmp(tz, "EDT") == 0)
   {
      // 4 hours west
      rval.mMinutesWest = 240;
   }
   else if(strcasecmp(tz, "EST") == 0)
   {
      // 5 hours west
      rval.mMinutesWest = 300;
   }
   else if(strcasecmp(tz, "PDT") == 0)
   {
      // 7 hours west
      rval.mMinutesWest = 420;
   }
   else if(strcasecmp(tz, "PST") == 0)
   {
      // 8 hours west
      rval.mMinutesWest = 480;
   }
   else
   {
      rval.mMinutesWest = gGetTimeZoneMinutesWest();
   }

   return rval;
}
