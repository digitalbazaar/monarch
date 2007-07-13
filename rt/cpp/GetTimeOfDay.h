/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_GetTimeOfDay_H
#define db_rt_GetTimeOfDay_H

// include types, time, and errno
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>

// define time between the epoch (01/01/1970) and UTC (01/01/1601)
// measured in 10ths of a microsecond (which is equal to 100 nanoseconds
// as is the increment for UTC)
#ifndef __GNUC__
   #define EPOCH_UTC_TENTHMICROSECS_DELTA 116444736000000000i64
#else
   #define EPOCH_UTC_TENTHMICROSECS_DELTA 116444736000000000LL
#endif

// define gettimeofday() for windows as necessary
#ifdef WIN32

// include windows headers for obtaining time
#include <windows.h>

#ifndef __MINGW32__

// define timezone structure
struct timezone
{
   /**
    * The number of minutes west of Greenwich, England.
    */
   int tz_minuteswest;
   
   /**
    * The type of daylight savings time (DST): 0 for no DST is used in
    * the time zone, 1 if it is.
    */
   int tz_dsttime;
};

/**
 * Gets the current time of day and stores it in the passed timeval
 * structure as the number of seconds and microseconds since the epoch. 
 * 
 * @param tv the timeval structure.
 * @param tz the timezone structure.
 * 
 * @return 0 if the method executed with success, -1 if not.
 */
inline static int gettimeofday(struct timeval* tv, struct timezone* tz)
{
   // define tzinit flag (whether or not the timezone has been initialized)
   static bool tzinit = false;
   
   // NOTE: use FileTimeToSystemTime to convert a FILETIME to a SYSTEMTIME
   // that has years, months, days, hours, seconds, milliseconds, etc
   if(tv != NULL)
   {
      // get the system time as a file time (a UTC time object)
      FILETIME ft;
      GetSystemTimeAsFileTime(&ft);
      
      // use a large integer to combine the time into a 64-bit long
      LARGE_INTEGER li;
      li.LowPart = ft.dwLowDateTime;
      li.HighPart = ft.dwHighDateTime;
      
      // get the 64-bit long in 
      // the time is in 100-nanosecond intervals (1000 nanoseconds in
      // a microsecond, so this is really just microseconds / 10)
      __int64 time = li.QuadPart;
      
      // eliminate time between 01/01/1601 (UTC) and 01/01/1970 (epoch)
      time -= EPOCH_UTC_TENTHMICROSECS_DELTA;
      
      // get time in microseconds
      time /= 10;
      
      // store time in seconds and microseconds (1 microsecond = 1000000 sec)
      tv->tv_sec  = (unsigned long)(time / 1000000UL);
      tv->tv_usec = (unsigned long)(time % 1000000UL);
   }
   
   // populate timezone
   if(tz != NULL) 
   {
      if(!tzinit)
      {
         // initialize timezone
         _tzset();
         tzinit = true;
      }
      
      // set timezone information:
      
      // get the number of minutes (_timezone is in seconds)
      tz->tz_minuteswest = _timezone / 60UL;
      tz->tz_dsttime = _daylight;
   }
   
   // success
   return 0;
}

#endif

// define the number of days per month (in non-leap years)
static const short gDaysInPreviousMonth[12] =
{
   0,   // no days before january
   31,  // days before february
   59,  // 31+28 days before march
   90,  // 31+28+31 days before april
   120, // 31+28+31+30 days before may
   151, // 31+28+31+30+31 days before june
   181, // 31+28+31+30+31+30 days before july
   212, // 31+28+31+30+31+30+31 days before august
   243, // 31+28+31+30+31+30+31+31 days before september
   273, // 31+28+31+30+31+30+31+31+30 days before october
   304, // 31+28+31+30+31+30+31+31+30+31 days before november
   334, // 31+28+31+30+31+30+31+31+30+31+30 days before december
};

/**
 * Determines if the passed year is a leap year.
 * 
 * @param year the year to check.
 * 
 * @return true if the passed year is a leap year, false if not.
 */
inline static bool gIsLeapYear(unsigned int year)
{
   // leap years occur every 4 years except for years that are divisible
   // by 100 AND NOT by 400 -- i.e. 1700 and 1800 are not leap years, but
   // 2000 is
   return (year % 4 == 0) && !((year % 100 == 0) && !(year % 400 == 0));
}

// undefine macros for gmtime_r and localtime_r
#undef gmtime_r
#undef localtime_r

/**
 * Breaks the passed time_t struct (seconds since the epoch) into a broken-down
 * time representation in Coordinated Universal Time (UTC) (GMT time). The
 * passed tm struct will be populated.
 * 
 * This function is re-entrant and therefore thread-safe.
 * 
 * @param timep the time_t (seconds since the epoch) to break-down.
 * @param result the tm to populate with the broken-down date.
 * 
 * @return a pointer to the result.
 */
inline static struct tm* gmtime_r(const time_t* timep, struct tm* result)
{
   // the number of seconds per day
   time_t secsPerDay = 86400;
   
   // the number of seconds today
   time_t secs = *timep % secsPerDay;
   
   // the number of whole minutes today
   time_t mins = secs / 60;
   
   // set the number of seconds after the current minute
   result->tm_sec = secs % 60;
   
   // set the number of minutes after the current hour
   result->tm_min = mins % 60;
   
   // set the number of hours past midnight
   result->tm_hour = mins / 60;
   
   // determine the year and the day in the year:
   
   // start with the number of whole days since the epoch
   time_t day = *timep / secsPerDay;
   time_t daysPerYear = 365;
   
   // determine the week day (Jan 1 1970 was a Thursday, so add 4)
   result->tm_wday = (day + 4) % 7;
   
   int* year = &result->tm_year;
   for(*year = 1970; ; (*year)++)
   {
      // check for leap year
      daysPerYear = gIsLeapYear(*year) ? 366 : 365;
      if(day >= daysPerYear)
      {
         // remove year
         day -= daysPerYear;
      }
      else
      {
         // year should be years since 1900
         *year -= 1900;
         break;
      }
   }
   
   // set the day in the year (days do not begin on 0, so +1)
   day++;
   result->tm_yday = day;
   
   // remove extra day from leap years (31 days in Jan + 28 in February = 59)
   if(gIsLeapYear(1900 + *year) && day > 59)
   {
      day--;
   }
   
   // determine the month
   int* month = &result->tm_mon;
   for(*month = 11; day <= gDaysInPreviousMonth[*month]; (*month)--);
   
   // determine the day of the month
   result->tm_mday = day - gDaysInPreviousMonth[*month];
   
   // daylight savings time information not available
   result->tm_isdst = -1;
   
   return result;
}

/**
 * Breaks the passed time_t struct (seconds since the epoch) into a broken-down
 * time representation in the local time zone. The passed tm struct will be
 * populated.
 * 
 * This function is re-entrant and therefore thread-safe.
 * 
 * @param timep the time_t (seconds since the epoch) to break-down.
 * @param result the tm to populate with the broken-down date.
 * 
 * @return a pointer to the result.
 */
inline static struct tm* localtime_r(const time_t* timep, struct tm* result)
{
   // get the local time zone
   struct timezone tz;
   gettimeofday(NULL, &tz);
   
   // FIXME: this is wrong for EDT (it reports EST)
   // remove the minutes west (as seconds) to the passed time
   time_t local = *timep - tz.tz_minuteswest * 60UL;
   
   // get the UTC time
   return gmtime_r(&local, result);
}

#endif // end of defining gettimeofday()

#endif
