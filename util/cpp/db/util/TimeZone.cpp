/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/TimeZone.h"
#include "db/rt/System.h"

#include <cstring>

using namespace std;
using namespace db::rt;
using namespace db::util;

TimeZone::TimeZone(unsigned int minutesWest)
{
   mMinutesWest = minutesWest;
}

TimeZone::~TimeZone()
{
}

inline unsigned int TimeZone::getMinutesWest()
{
   return mMinutesWest;
}

TimeZone TimeZone::getTimeZone(const char* tz)
{
   TimeZone rval;
   
   if(strcmp(tz, "GMT") == 0 || strcmp(tz, "UTC") == 0)
   {
      rval.mMinutesWest = 0;
   }
   else if(strcmp(tz, "EDT") == 0)
   {
      // 4 hours west
      rval.mMinutesWest = 240;
   }
   else if(strcmp(tz, "EST") == 0)
   {
      // 5 hours west
      rval.mMinutesWest = 300;
   }
   else if(strcmp(tz, "PDT") == 0)
   {
      // 7 hours west
      rval.mMinutesWest = 420;
   }
   else if(strcmp(tz, "PST") == 0)
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
