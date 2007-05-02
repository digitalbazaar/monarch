/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef GetTimeOfDay_H
#define GetTimeOfDay_H

// include types
#ifndef WIN32
   #include <sys/types.h>
#endif

// check for the appropriate time header to include:
#ifdef TIME_WITH_SYS_TIME
   // first include sys/time and then time 
   #include <sys/time.h>
   #include <time.h>
#else
   #ifdef HAVE_SYS_TIME_H
      // include just sys/time
      #include <sys/time.h>
   #else
      // include just time
      #include <time.h>
   #endif
// end of checking for time headers
#endif

// define gettimeofday() as necessary
#ifndef HAVE_GETTIMEOFDAY

#ifdef WIN32
   // include windows headers for obtaining time
   #include <windows.h>
#else
   // may be unknown system, cannot implement gettimeofday
   #include <errno.h>
#endif

// define time between the epoch (01/01/1970) and UTC (01/01/1601)
// measured in 10ths of a microsecond (which is equal to 100 nanoseconds
// as is the increment for UTC)
#ifndef __GNUC__
   #define EPOCH_UTC_TENTHMICROSECS_DELTA 116444736000000000i64
#else
   #define EPOCH_UTC_TENTHMICROSECS_DELTA 116444736000000000LL
#endif

// define timeval structure
//struct timeval
//{
//   /**
//    * The number of seconds since the epoch.
//    */
//   long tv_sec;
//   
//   /**
//    * The number of microseconds in edition to the number of seconds
//    * since the epoch.
//    */
//   long tv_usec;
//};

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
// if on a windows system, define the method
#ifdef WIN32
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
      tz->tz_minuteswest = _timezone / 60;
      tz->tz_dsttime = _daylight;
   }
   
   // success
   return 0;
// not on a windows system
#else
   // return an error
   errno = ENOSYS;
   return -1;
#endif
}

#endif // end of defining gettimeofday()

#endif
