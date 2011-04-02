/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/Date.h"

#include "monarch/rt/Exception.h"
#include "monarch/rt/System.h"
#include "monarch/util/StringTools.h"

#include <cstring>

using namespace std;
using namespace monarch::rt;
using namespace monarch::util;

// %F = YYYY-MM-DD
// %T = HH:MM:SS
// Note: windows fails on %F and %T specifiers, so
// we use the more compatible ones instead
#define FORMAT_UTC_DATETIME      "%Y-%m-%d %H:%M:%S"
#define FORMAT_UTC_DATETIME_TZ   "%Y-%m-%dT%H:%M:%SZ"

Date::Date()
{
   // get the current time in seconds since the Epoch
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

inline int Date::yearSince1900()
{
   return mBrokenDownTime.tm_year;
}

inline int Date::year()
{
   return 1900 + mBrokenDownTime.tm_year;
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
      // use broken down time
      time = mBrokenDownTime;
   }
   else
   {
      // get UTC time
      gmtime_r(&mSecondsSinceEpoch, &time);
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
   localtime_r(&mSecondsSinceEpoch, &mBrokenDownTime);
}

time_t Date::getSeconds()
{
   return mSecondsSinceEpoch;
}

/**
 * Converts a time from one timezone to another. This is accomplished by
 * adding the minutes west from the first timezone to get to UTC, and then
 * subtracting the minutes west from the second timezone.
 *
 * @param in the input time in seconds.
 * @param inTz the input timezone.
 * @param outTz the output timezone.
 *
 * @return the output time in seconds.
 */
inline static time_t changeTimeZone(time_t in, TimeZone* inTz, TimeZone* outTz)
{
   // add in timezone to get to UTC, then subtract out timezone
   return in + (inTz->getMinutesWest() - outTz->getMinutesWest()) * 60;
}

string Date::getDateTime(TimeZone* tz)
{
   string rval;
   return format(rval, FORMAT_UTC_DATETIME, tz);
}

string Date::getUtcDateTime(bool includeTandZ)
{
   TimeZone tz = TimeZone::getTimeZone("UTC");
   string rval;
   return format(
      rval, includeTandZ ? FORMAT_UTC_DATETIME_TZ : FORMAT_UTC_DATETIME, &tz);
}

string& Date::format(string& str, const char* format, TimeZone* tz)
{
   struct tm time;

   // no timezone provided
   if(tz == NULL)
   {
      // use stored local time
      time = mBrokenDownTime;
   }
   // apply timezone
   else
   {
      /* Note: In this case the output timezone might be different from the
       * local one. The stored broken down time represents the local timezone.
       * This means we must generate a new broken down time to use for our
       * formatted output.
       *
       * The API calls available to generate a broken down time are gmtime_r()
       * or localtime_r(). So in order to produce a correct broken down
       * time for a timezone that is different from the local one, we must
       * change the stored time by the difference between the two timezones and
       * then call the appropriate function. This means we can use either:
       *
       * 1. UTC and the target timezone and then call gmtime_r(), or
       * 2. the local timezone and the target timezone and then call
       *    localtime_r().
       *
       * We chose gmtime_r() because getting the UTC timezone is faster than
       * getting the local one.
       *
       * To get the appropriate broken down time, first changeTimeZone() is
       * called with UTC as the input timezone and the target timezone as the
       * output timezone. We then reproduce the broken down time by calling
       * gmtime_r(). The end result is what the broken down time would be
       * for our original stored time if it were in the target timezone.
       */
      TimeZone utc = TimeZone::getTimeZone("UTC");
      time_t seconds = changeTimeZone(mSecondsSinceEpoch, &utc, tz);
      gmtime_r(&seconds, &time);
   }

   // print the time to a string
   size_t size = strlen(format) + 100;
   char out[size];
   strftime(out, size, format, &time);
   str.assign(out);

   return str;
}

bool Date::parse(const char* str, const char* format, TimeZone* tz)
{
   bool rval = true;

   if(strptime(str, format, &mBrokenDownTime) == NULL)
   {
      ExceptionRef e = new Exception(
         "Could not parse date.",
         "monarch.util.Date.ParseError");
      e->getDetails()["date"] = str;
      e->getDetails()["format"] = format;
      Exception::set(e);
      rval = false;
   }
   else
   {
      // make time (-1 = use C lib to determine daylight savings time)
      mBrokenDownTime.tm_isdst = -1;
      mSecondsSinceEpoch = mktime(&mBrokenDownTime);

      /* Note: mktime() returns the number of seconds since the Epoch for the
       * given broken down time assuming that it is in the local timezone. The
       * input's timezone may not be in the local timezone, however. In order
       * to generate the correct broken down time for this machine's local
       * time zone, we must convert from the given timezone to the local one.
       *
       * To do this, we simply call changeTimeZone() with the input timezone
       * as the given one and the output timezone as this machine's local
       * timezone. The result is stored in mSecondsSinceEpoch.
       *
       * For example, if the input was 5:00pm PST, and this machine's local
       * timezone is EST, then mktime() will return the number of seconds since
       * the Epoch for 5:00pm EST. But the input, if it were in EST, would
       * actually 8:00pm because of the three hour difference between PST and
       * EST. To correct this discrepancy, we simply interpret the result from
       * mktime() as if it were 5:00pm PST and convert it to EST. The end
       * result is the number of seconds since the Epoch for 8:00pm EST, which
       * is the correct value for the given input.
       */
      if(tz != NULL)
      {
         TimeZone local = TimeZone::getTimeZone();
         mSecondsSinceEpoch = changeTimeZone(mSecondsSinceEpoch, tz, &local);
      }

      // ensure broken down time is totally filled out and reflects local time
      // (strptime may not populate all fields)
      localtime_r(&mSecondsSinceEpoch, &mBrokenDownTime);
   }

   return rval;
}

string Date::toString(const char* format, TimeZone* tz)
{
   string rval;
   return this->format(rval, format, tz);
}

bool Date::parseUtcDateTime(const char* str, bool includeTandZ)
{
   TimeZone tz = TimeZone::getTimeZone("UTC");
   return parse(
      str, includeTandZ ? FORMAT_UTC_DATETIME_TZ : FORMAT_UTC_DATETIME, &tz);
}

std::string Date::utcDateTime(bool includeTandZ)
{
   Date d;
   return d.getUtcDateTime(includeTandZ);
}

time_t Date::utcSeconds(const char* str, bool includeTandZ)
{
   Date d;
   if(str != NULL)
   {
      d.parseUtcDateTime(str, includeTandZ);
   }
   return d.getSeconds();
}
