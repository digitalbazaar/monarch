/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/Date.h"

#include "monarch/rt/System.h"
#include "monarch/util/StringTools.h"

#include <cstring>

using namespace std;
using namespace db::util;

Date::Date()
{
   // get the current time
   setSeconds(time(NULL));
}

Date::Date(time_t seconds)
{
   setSeconds(seconds);
}

Date::~Date()
{
}

inline int Date::second()
{
   return mBrokenDownTime.tm_sec;
}

inline int Date::minute()
{
   return mBrokenDownTime.tm_min;
}

inline int Date::hour()
{
   return mBrokenDownTime.tm_hour;
}

inline int Date::day()
{
   return mBrokenDownTime.tm_mday;
}

inline int Date::month()
{
   return mBrokenDownTime.tm_mon;
}

inline int Date::year()
{
   return mBrokenDownTime.tm_year;
}

void Date::setDosTime(unsigned int dosTime)
{
   // MS-DOS date & time bit-breakdown (4-bytes total):
   // [0-4][5-10][11-15][16-20][21-24][25-31]
   //  sec  min   hour   day    month  years
   //
   // sec is 0-59 / 2 (yes, divide by 2)
   // min is 0-59
   // hour is 0-23
   // day is 1-31
   // month is 1-12
   // years are from 1980
   mBrokenDownTime.tm_sec = (dosTime & 0x1f) * 2;
   mBrokenDownTime.tm_min = (dosTime >> 5) & 0x3f;
   mBrokenDownTime.tm_hour = (dosTime >> 11) & 0x1f;
   mBrokenDownTime.tm_mday = (dosTime >> 16) & 0x1f;
   mBrokenDownTime.tm_mon = ((dosTime >> 21) & 0x0f) - 1;
   mBrokenDownTime.tm_year = ((dosTime >> 25) & 0x7f) + 80;
}

unsigned int Date::dosTime(bool local)
{
   struct tm time;

   if(local)
   {
      // get local time
      localtime_r(&mSecondsSinceEpoch, &time);
   }
   else
   {
      // use broken down time
      time = mBrokenDownTime;
   }

   // MS-DOS date & time bit-breakdown (4-bytes total):
   // [0-4][5-10][11-15][16-20][21-24][25-31]
   //  sec  min   hour   day    month  years
   //
   // sec is 0-59 / 2 (yes, divide by 2)
   // min is 0-59
   // hour is 0-23
   // day is 1-31
   // month is 1-12
   // years are from 1980
   return
      (time.tm_sec / 2) |
      time.tm_min << 5 |
      time.tm_hour << 11 |
      time.tm_mday << 16 |
      (time.tm_mon + 1) << 21 |
      (time.tm_year - 80) << 25;
}

void Date::addSeconds(time_t seconds)
{
   setSeconds(mSecondsSinceEpoch + seconds);
}

void Date::setSeconds(time_t seconds)
{
   mSecondsSinceEpoch = seconds;
   gmtime_r(&mSecondsSinceEpoch, &mBrokenDownTime);
}

time_t Date::getSeconds()
{
   return mSecondsSinceEpoch;
}

string Date::getDateTime(TimeZone* tz)
{
   // %F = YYYY-MM-DD
   // %T = HH:MM:SS
   string rval;
   // Note: windows fails on %F and %T specifiers, so
   // we use the more compatible ones instead
   return format(rval, "%Y-%m-%d %H:%M:%S", tz);
}

string Date::getUtcDateTime()
{
   TimeZone tz = TimeZone::getTimeZone("UTC");
   return getDateTime(&tz);
}

string& Date::format(string& str, const char* format, TimeZone* tz)
{
   struct tm time;

   // apply time zone
   if(tz == NULL)
   {
      // get local time
      localtime_r(&mSecondsSinceEpoch, &time);
   }
   else if(tz->getMinutesWest() != 0)
   {
      // remove minutes west and get time
      time_t seconds = mSecondsSinceEpoch - tz->getMinutesWest() * 60UL;
      gmtime_r(&seconds, &time);
   }
   else
   {
      // use stored time
      time = mBrokenDownTime;
   }

   // print the time to a string
   unsigned int size = strlen(format) + 100;
   char out[size];
   strftime(out, size, format, &time);
   str.assign(out);

   return str;
}

bool Date::parse(const char* str, const char* format, TimeZone* tz)
{
   bool rval = false;

   if(strptime(str, format, &mBrokenDownTime) != NULL)
   {
      rval = true;

      if(tz == NULL)
      {
         // get local time
         mSecondsSinceEpoch = mktime(&mBrokenDownTime);
      }
      else
      {
         // get gmt time (applies no timezone)
         mSecondsSinceEpoch = timegm(&mBrokenDownTime);
      }

      // ensure broken down time is in GMT and totally filled out
      // (strptime may not populate all fields)
      gmtime_r(&mSecondsSinceEpoch, &mBrokenDownTime);
   }

   return rval;
}

string Date::toString(const char* format, TimeZone* tz)
{
   string rval;
   return this->format(rval, format, tz);
}
