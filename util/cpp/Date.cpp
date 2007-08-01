/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Date.h"
#include "System.h"
#include "StringTools.h"
#include "StringTokenizer.h"

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

string& Date::format(
   string& str, const string& format, const string& formatType, TimeZone* tz)
{
   string f = format;
   
   if(formatType == "java")
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
   str.erase();
   str.append(out);
   
   return str;
}

bool Date::parse(
   const string& str, const string& format, const string& formatType,
   TimeZone* tz)     
{
   bool rval = false;
   
   if(formatType == "c")
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
   
//   // tokenize string and format
//   long long integer;
//   StringTokenizer strTokenizer(str, ' ');
//   StringTokenizer formatTokenizer(str, ' ');
//   
//   if(strTokenizer.getTokenCount() == formatTokenizer.getTokenCount())
//   {
//      // Note: week day, week in year, and day in year are ignored by
//      // mktime/timegm so they needn't be parsed
//      
//      // FIXME: could probably do this cleaner in the future with sscanf()
//      if(formatType == "c")
//      {
//         
//         
//         string value;
//         string type;
//         bool pm = false;
//         while(strTokenizer.hasMoreTokens())
//         {
//            value = formatTokenizer.nextToken();
//            type = formatTokenizer.nextToken();
//            if(type == "%p")
//            {
//               // AM/PM
//               pm = (value == "PM");
//            }
//            else if(type == "%Y")
//            {
//               // year (4 digit)
//               Convert::stringToInteger(value, integer);
//               mBrokenDownTime.tm_year = integer;
//            }
//            else if(type == "%y")      
//            {
//               // year (2 digit)
//               Convert::stringToInteger(value, integer);
//               
//               // an arbitrary algorithm for guessing if the 2 year date
//               // should be in 20th or 21st century
//               if(mBrokenDownTime.tm_year + 50 > integer)
//               {
//                  mBrokenDownTime.tm_year = integer + 1900;
//               }
//               else
//               {
//                  mBrokenDownTime.tm_year = integer + 2000;
//               }
//            }
//            else if(type == "%B" || type == "%b")
//            {
//               // month in year (full)
//               // FIXME: figure out month
//               mBrokenDownTime.tm_mon
//            }
//            else if(type == "%b")
//            {
//               // month in year (abbreviated)
//               // FIXME: figure out month
//               mBrokenDownTime.tm_mon
//            }
//            else if(type == "%d")
//            {
//               // day in month
//               mBrokenDownTime.tm_mday =
//            }
//            else if(type == "%H")
//            {
//               // hour in day (0-24)
//               mBrokenDownTime.tm_hour =
//            }
//            else if(type == "%I")
//            {
//               // hour in day (1-12)
//               // FIXME: handle AM/PM
//               mBrokenDownTime.tm_hour = 
//            }
//            else if(type == "%M")
//            {
//               // minute in hour
//               mBrokenDownTime.tm_min = 
//            }
//            else if(type == "%S")
//            {
//               // second in minute
//               mBrokenDownTime.tm_sec = 
//            }
//         }
//         
//         rval = true;
//      }
//   }
   
   if(tz == NULL)
   {
      // get local time
      mSecondsSinceEpoch = mktime(&mBrokenDownTime);
   }
   else
   {
      // get gmt time (applies no timezone)
      mSecondsSinceEpoch = timegm(&mBrokenDownTime);
      
      if(tz->getMinutesWest() != 0)
      {
         // add minutes west to get to gmt time
         mSecondsSinceEpoch += tz->getMinutesWest() * 60UL;
      }
   }
   
   // ensure broken down time is accurate
   gmtime_r(&mSecondsSinceEpoch, &mBrokenDownTime);
   
   return rval;
}
