/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_TimeFunctions_H
#define monarch_rt_TimeFunctions_H

/**
 * This header provides some cross-platform definitions for time functions.
 *
 * Primarily, this header defines functionality for missing POSIX time
 * functions in windows.
 *
 * FIXME: This header's code should be relocated to .c/.cpp.
 *
 * @author Dave Longley
 */

// include types, time, and errno
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/timeb.h>
#include <inttypes.h>

//// prevent C++ name mangling
//#ifdef __cplusplus
//extern "C" {
//#endif

/**
 * Gets the number of minutes west of GMT the current time zone is.
 *
 * @return the number of minutes west of GMT the current time zone is.
 */
inline static long long gGetTimeZoneMinutesWest()
{
   struct timeb time;
   ftime(&time);

   // apply daylight savings time
   if(time.dstflag != 0)
   {
      time.timezone -= 60;
   }

   return time.timezone;
}

#ifdef WIN32

// define time between the epoch (01/01/1970) and UTC (01/01/1601)
// measured in 10ths of a microsecond (which is equal to 100 nanoseconds
// as is the increment for UTC)
#define EPOCH_UTC_TENTHMICROSECS_DELTA INT64_C(116444736000000000)

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
//      // get the system time as a file time (a UTC time object)
//      FILETIME ft;
//      GetSystemTimeAsFileTime(&ft);
//
//      // use a large integer to combine the time into a 64-bit long
//      LARGE_INTEGER li;
//      li.LowPart = ft.dwLowDateTime;
//      li.HighPart = ft.dwHighDateTime;
//
//      // get the 64-bit long in
//      // the time is in 100-nanosecond intervals (1000 nanoseconds in
//      // a microsecond, so this is really just microseconds / 10)
//      __int64 time = li.QuadPart;

      // get the system time as a file time (a UTC time object),
      // interpretable as a 64-bit integer
      union now
      {
         FILETIME ft;
         __int64 time;
      };

      GetSystemTimeAsFileTime(&now.ft);

      // eliminate time between 01/01/1601 (UTC) and 01/01/1970 (epoch)
      now.time -= EPOCH_UTC_TENTHMICROSECS_DELTA;

      // get time in microseconds
      now.time /= UINT64_C(10);

      // store time in seconds and microseconds (1 microsecond = 1000000 sec)
      tv->tv_sec  = (unsigned long long)(now.time / UINT64_C(1000000));
      tv->tv_usec = (unsigned long long)(now.time % UINT64_C(1000000));
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

      // get the number of minutes west (tz_dsttime is supposedly obsolete)
      tz->tz_minuteswest = gGetTimeZoneMinutesWest();
      tz->tz_dsttime = _daylight;
   }

   // success
   return 0;
}

#endif // end of not defined __MINGW32__

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

// define months
static const char* gMonths[12] =
{
   "January",
   "February",
   "March",
   "April",
   "May",
   "June",
   "July",
   "August",
   "September",
   "October",
   "November",
   "December"
};

// define days of the week
static const char* gDaysOfWeek[7] =
{
   "Sunday",
   "Monday",
   "Tuesday",
   "Wednesday",
   "Thursday",
   "Friday",
   "Saturday"
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

/**
 * Gets a number from the passed string and increments the string's position
 * to the first non-digit character or the end of the number with the specified
 * number of digits.
 *
 * @param s the string to get the number from.
 * @param num the number to update.
 * @param digits the number of digits for the number.
 *
 * @return true if there were sufficient digits, false if not.
 */
inline static bool gStringToNumber(const char** s, int& num, int digits)
{
   bool rval = true;

   int pow, digit;
   num = 0;
   digits--;
   for(; rval && digits >= 0; digits--)
   {
      // ASCII 48-57 for 0-9
      if(**s > 47 && **s < 58)
      {
         digit = **s - '0';
         for(pow = 0; pow < digits; pow++)
         {
            digit *= 10;
         }

         num += digit;
         ++*s;
      }
      else
      {
         rval = false;
      }
   }

   return rval;
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
   // remove the minutes west (as seconds) to the passed time
   time_t local = *timep - (gGetTimeZoneMinutesWest() * 60);

   // get the UTC time
   return gmtime_r(&local, result);
}

/**
 * Parses a date string based on the given format and writes its broken-down
 * time to the passed tm struct.
 *
 * @param s the date string to parse.
 * @param format the format to parse according to.
 * @param tm the broken-down time structure to write the result to.
 *
 * @return a pointer to the first character not processed by this call.
 */
inline static char* strptime(const char* s, const char* format, struct tm* tm)
{
   // Note: This algorithm searches for the next non-escaped percent sign in
   // the format string on each iteration. As the format string is searched,
   // the date string pointer is also incremented.
   //
   // From there, the date substring is parsed according to the character that
   // follows the percent sign in the format string.
   //
   // FIXME: this parse code is incomplete, and hasn't been robustly tested
   // Note: week day, week in year, and day in year can be ignored because
   // mktime/timegm doesn't use them
   int century = -1;
   unsigned int length;
   const char* old;
   bool pm = false;
   bool parse = true;
   while(parse && *s != 0 && *format != 0 && *(++format) != 0)
   {
      // check for parenthesis
      switch(*(format - 1))
      {
         case '%':
            // check format character
            switch(*format)
            {
               case '%':
                  // escaped parenthesis, skip it
                  s++;
                  break;
               case 'A':
                  // week day (full), skip
                  parse = false;
                  for(int i = 0; i < 12; i++)
                  {
                     length = strlen(gDaysOfWeek[i]);
                     if(strncasecmp(gDaysOfWeek[i], s, length) == 0)
                     {
                        s += length;
                        parse = true;
                        break;
                     }
                  }
                  break;
               case 'a':
                  // week day (abbreviated), skip
                  parse = false;
                  for(int i = 0; i < 12; i++)
                  {
                     if(strncasecmp(gDaysOfWeek[i], s, 3) == 0)
                     {
                        s += 3;
                        parse = true;
                        break;
                     }
                  }
                  break;
               case 'B':
                  // month in year (full)
                  parse = false;
                  for(int i = 0; i < 12; i++)
                  {
                     length = strlen(gMonths[i]);
                     if(strncasecmp(gMonths[i], s, length) == 0)
                     {
                        s += length;
                        tm->tm_mon = i;
                        parse = true;
                        break;
                     }
                  }
                  break;
               case 'b':
               case 'h':
                  // month in year (abbreviated to 3 letters)
                  for(int i = 0; i < 12; i++)
                  {
                     if(strncasecmp(gMonths[i], s, 3) == 0)
                     {
                        s += 3;
                        tm->tm_mon = i;
                        parse = true;
                        break;
                     }
                  }
                  break;
               case 'c':
                  // preferred date/time representation for locale
                  s = strptime(s, "%a, %d %b %Y %H:%M:%S", tm);
                  break;
               case 'C':
                  // century number (0-99) in 2 digits
                  parse = gStringToNumber(&s, century, 2);
                  break;
               case 'd':
                  // day in month (2 digits)
                  old = s;
                  gStringToNumber(&s, tm->tm_mday, 2);
                  parse = (s != old);
                  break;
               case 'D':
                  s = strptime(s, "%m/%d/%y", tm);
                  break;
               case 'e':
                  // same as %d, but leading zero is a space
                  s += (*s == ' ') ? 1 : 0;
                  s = strptime(s, "%d", tm);
               case 'k':
                  // same as %H, but leading zero is a space
                  s += (*s == ' ') ? 1 : 0;
               case 'H':
                  // hour in day (0-23)
                  old = s;
                  gStringToNumber(&s, tm->tm_hour, 2);
                  parse = (s != old);
                  break;
               case 'l':
                  // same as %I, but leading zero is a space
                  s += (*s == ' ') ? 1 : 0;
               case 'I':
                  // hour in day (1-12)
                  old = s;
                  gStringToNumber(&s, tm->tm_hour, 2);
                  tm->tm_hour = pm ? tm->tm_hour + 12 : tm->tm_hour;
                  parse = (s != old);
               case 'm':
                  // month as a number (01-12)
                  old = s;
                  gStringToNumber(&s, tm->tm_mon, 2);
                  parse = (s != old);
                  tm->tm_mon++;
               case 'M':
                  // minute in hour
                  old = s;
                  gStringToNumber(&s, tm->tm_min, 2);
                  parse = (s != old);
                  break;
               case 'n':
               case 't':
                  // skip new line or tab character
                  s++;
                  break;
               case 'P':
                  // AM/PM
                  pm = (*s == 'P' && *(++s) == 'M');
                  break;
               case 'p':
                  // AM/PM
                  pm = (*s == 'p' && *(++s) == 'm');
                  break;
               case 'r':
                  // POSIX locale in am/pm
                  s = strptime(s, "%I:%M:%S %p", tm);
                  break;
               case 'S':
                  // second in minute
                  old = s;
                  gStringToNumber(&s, tm->tm_sec, 2);
                  parse = (s != old);
                  break;
               case 's':
                  // seconds since the epoch
                  // FIXME: call gmtime here?
                  break;
               case 'T':
                  s = strptime(s, "%H:%M:%S", tm);
                  break;
               case 'U':
                  // 2 digit week of year starting on a Sunday, skip
                  s++;
                  if(*s != 0)
                  {
                     s++;
                  }
                  break;
               case 'u':
                  // day of week 1-7, skip
                  s++;
                  break;
               case 'W':
                  // 2 digit week of year starting on a Monday, skip
                  s++;
                  if(*s != 0)
                  {
                     s++;
                  }
                  break;
               case 'w':
                  // day of week 0-6, skip
                  s++;
                  break;
               case 'X':
                  // preferred date representation for locale without time
                  s = strptime(s, "%a, %d %b %Y", tm);
                  break;
               case 'x':
                  // preferred time representation for locale without date
                  s = strptime(s, "%H:%M:%S", tm);
                  break;
               case 'Y':
                  // year (4 digit)
                  parse = gStringToNumber(&s, tm->tm_year, 4);
                  tm->tm_year -= 1900;
                  break;
               case 'y':
                  // year (2 digit)
                  parse = gStringToNumber(&s, tm->tm_year, 2);

                  if(century == -1)
                  {
                     // spec says values under 69 = 21th century, others 20th
                     tm->tm_year = (tm->tm_year < 69) ?
                        tm->tm_year + 2000 : tm->tm_year + 1900;
                  }
                  else
                  {
                     if(century > 20)
                     {
                        tm->tm_year += (century * 100);
                     }
                     else if(century < 20)
                     {
                        tm->tm_year -= (century * 100);
                     }
                  }
                  break;
               case 'Z':
                  // time zone abbreviation
                  break;
               case 'z':
                  // time zone as an hour offset from GMT
                  break;
               case '+':
                  // date and time in date(1) format
                  break;
               default:
                  // unrecognized format character
                  parse = false;
                  break;
            }
            format++;
            break;
         default:
            // non-format character
            s++;
            break;
      }
   }

   return (char*)s;
}

/**
 * Converts the passed broken-down tm struct into seconds since the epoch in
 * UTC.
 *
 * @param tm the broken-down structure to convert.
 *
 * @return the number of seconds since the epoch in UTC for the given structure.
 */
inline static time_t timegm(struct tm* tm)
{
   // get local time
   time_t rval = mktime(tm);
   if(rval != -1)
   {
      // subtract seconds west to get to GMT time
      rval -= gGetTimeZoneMinutesWest() * 60;
   }

   return rval;
}

#endif // end of defined WIN32

//#ifdef __cplusplus
//}
//#endif

#endif
