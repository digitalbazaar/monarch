/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/Date.h"

#include "db/rt/System.h"
#include "db/util/StringTools.h"

using namespace std;
using namespace db::util;

Date::Date()
{
   // get the current time
   mSecondsSinceEpoch = time(NULL);
   gmtime_r(&mSecondsSinceEpoch, &mBrokenDownTime);
}

Date::~Date()
{
}

int Date::second()
{
   return mBrokenDownTime.tm_sec;
}

int Date::minute()
{
   return mBrokenDownTime.tm_min;
}

int Date::hour()
{
   return mBrokenDownTime.tm_hour;
}

int Date::day()
{
   return mBrokenDownTime.tm_mday;
}

int Date::month()
{
   return mBrokenDownTime.tm_mon;
}

int Date::year()
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

string& Date::format(
   string& str, const string& format, const string& formatType, TimeZone* tz)
{
   string f = format;
   
   if(strcmp(formatType.c_str(), "java") == 0)
   {
      // FIXME: need to get negative lookbehind assertions working for regex
      
      // AM/PM
      StringTools::regexReplaceAll(f, "\\ba\\b", "%p");
      
      // year (4 digit)
      StringTools::regexReplaceAll(f, "\\by{4}\\b", "%Y");
      
      // year (2 digit)
      StringTools::regexReplaceAll(f, "\\by{1,2}\\b", "%y");
      
      // month in year (full)
      StringTools::regexReplaceAll(f, "\\bM{4,}\\b", "%B");
      
      // month in year (abbreviated)
      StringTools::regexReplaceAll(f, "\\bM{1,3}\\b", "%b");
      
      // week in year
      StringTools::regexReplaceAll(f, "\\bw{1,2}\\b", "%U");
      
      // day in year
      StringTools::regexReplaceAll(f, "\\bD{1,3}\\b", "%j");
      
      // day in month
      StringTools::regexReplaceAll(f, "\\bd{1,2}\\b", "%d");
      
      // day in week (full)
      StringTools::regexReplaceAll(f, "\\bE{4,}\\b", "%A");
      
      // day in week (abbreviated)
      StringTools::regexReplaceAll(f, "\\bE{1,3}\\b", "%a");
      
      // hour in day (0-24)
      StringTools::regexReplaceAll(f, "\\bH{1,2}\\b", "%H");
      
      // hour in day (1-12)
      StringTools::regexReplaceAll(f, "\\bh{1,2}\\b", "%I");
      
      // minute in hour
      StringTools::regexReplaceAll(f, "\\bm{1,2}\\b", "%M");
      
      // second in minute
      StringTools::regexReplaceAll(f, "\\bs{1,2}\\b", "%S");
   }
   
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
   unsigned int size = format.length() + 100;
   char out[size];
   memset(out, '\0', size);
   strftime(out, size, f.c_str(), &time);
   str.assign(out);
   
   return str;
}

bool Date::parse(
   const string& str, const string& format, const string& formatType,
   TimeZone* tz)
{
   bool rval = false;
   
   if(strcmp(formatType.c_str(), "c") == 0)
   {
      if(strptime(str.c_str(), format.c_str(), &mBrokenDownTime) != NULL)
      {
         rval = true;
      }
   }
   else
   {
      // update to current time
      mSecondsSinceEpoch = time(NULL);
      gmtime_r(&mSecondsSinceEpoch, &mBrokenDownTime);
   }
   
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
   
   // ensure broken down time is accurate
   gmtime_r(&mSecondsSinceEpoch, &mBrokenDownTime);
   
   return rval;
}
