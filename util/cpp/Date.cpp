/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Date.h"
#include "StringTools.h"

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

string& Date::format(string& str, const string& format, TimeZone* tz)
{
   string f = format;
   
   // year (4 digit)
   StringTools::replaceAll(f, "yyyy", "%Y");
   
   // year (2 digit)
   StringTools::replaceAll(f, "yy", "%0y");
   
   // month in year (full)
   StringTools::replaceAll(f, "MMMM", "%B");
   
   // month in year (abbreviated)
   StringTools::replaceAll(f, "MMM", "%b");
   
   // month in year (number)
   StringTools::replaceAll(f, "MM", "%0b");
   StringTools::replaceAll(f, "M", "%b");
   
   // week in year
   StringTools::replaceAll(f, "ww", "%0U");
   StringTools::replaceAll(f, "w", "%U");
   
   // day in year
   StringTools::replaceAll(f, "DDD", "%0j");
   StringTools::replaceAll(f, "D", "%j");
   
   // day in month
   StringTools::replaceAll(f, "dd", "%0d");
   StringTools::replaceAll(f, " d ", " %d ");
   StringTools::replaceAll(f, " d", " %d");
   
   // day in week (full)
   StringTools::replaceAll(f, "EEEE", "%A");
   
   // day in week (abbreviated)
   StringTools::replaceAll(f, "EEE", "%a");
   StringTools::replaceAll(f, "EE", "%a");
   StringTools::replaceAll(f, "E", "%a");
   
   // AM/PM
   StringTools::replaceAll(f, " a ", " %p ");
   StringTools::replaceAll(f, " a", " %p");
   
   // hour in day (0-24)
   StringTools::replaceAll(f, "HH", "%0H");
   StringTools::replaceAll(f, " H ", " %H ");
   StringTools::replaceAll(f, " H", "%H");
   
   // hour in day (1-12)
   StringTools::replaceAll(f, "hh", "%0I");
   StringTools::replaceAll(f, "h", "%I");
   
   // minute in hour
   StringTools::replaceAll(f, "mm", "%0M");
   StringTools::replaceAll(f, "m", "%M");
   
   // second in minute
   StringTools::replaceAll(f, "ss", "%0S");
   StringTools::replaceAll(f, "s", "%S");
   
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
   str.append(out);
   
   return str;
}

bool Date::parse(const string& str, const string& format)     
{
   // FIXME: implement me
   return false;
}
