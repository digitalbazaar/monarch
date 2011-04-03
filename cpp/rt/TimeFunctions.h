/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_rt_TimeFunctions_H
#define monarch_rt_TimeFunctions_H

/**
 * This header provides some cross-platform definitions for time functions.
 *
 * Primarily, this header defines functionality for missing POSIX time
 * functions in windows.
 *
 * @author Dave Longley
 */

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <inttypes.h>

// prevent C++ name mangling
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Gets the number of minutes west of UTC the local time zone is.
 *
 * @param t the time to use to get the local time zone (time of the year
 *           changes the time zone because of DST).
 *
 * @return the number of minutes west of UTC the local time zone is.
 */
int64_t gGetTimeZoneMinutesWest(time_t* t);

#ifdef WIN32

#ifndef __MINGW32__

// define timezone structure
struct timezone
{
   /**
    * The number of minutes west of Greenwich, England (UTC).
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
int gettimeofday(struct timeval* tv, struct timezone* tz);

#endif // end of not defined __MINGW32__

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
struct tm* gmtime_r(const time_t* timep, struct tm* result);

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
struct tm* localtime_r(const time_t* timep, struct tm* result);

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
char* strptime(const char* s, const char* format, struct tm* tm);

/**
 * Converts the passed broken-down tm struct into seconds since the epoch in
 * UTC.
 *
 * @param tm the broken-down structure to convert.
 *
 * @return the number of seconds since the epoch in UTC for the given structure.
 */
time_t timegm(struct tm* tm);

#endif // end of defined WIN32

#ifdef __cplusplus
}
#endif

#endif
