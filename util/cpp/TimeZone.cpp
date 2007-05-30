/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "TimeZone.h"
#include "System.h"

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

unsigned int TimeZone::getMinutesWest()
{
   return mMinutesWest;
}

TimeZone TimeZone::getTimeZone(const string& tz)
throw(IllegalArgumentException)
{
   TimeZone rval;
   
   if(tz == "GMT")
   {
      rval.mMinutesWest = 0;
   }
   else if(tz == "EDT")
   {
      // 4 hours west
      rval.mMinutesWest = 240;
   }
   else if(tz == "EST")
   {
      // 5 hours west
      rval.mMinutesWest = 300;
   }
   else if(tz == "PDT")
   {
      // 7 hours west
      rval.mMinutesWest = 420;
   }
   else if(tz == "PST")
   {
      // 8 hours west
      rval.mMinutesWest = 480;
   }
   else if(tz == "")
   {
      // get local time zone
      struct timezone tz;
      gettimeofday(NULL, &tz);
      
      // FIXME: this is wrong for EDT (it reports EST)
      rval.mMinutesWest = tz.tz_minuteswest;
   }
   else
   {
      throw IllegalArgumentException("Invalid time zone '" + tz + "'!");
   }
   
   return rval;
}
