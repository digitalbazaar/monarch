/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_TimeZone_H
#define monarch_util_TimeZone_H

#include <string>

namespace monarch
{
namespace util
{

/**
 * A TimeZone represents an offset from GMT time.
 *
 * @author Dave Longley
 */
class TimeZone
{
protected:
   /**
    * The number of minutes west of GMT.
    */
   time_t mMinutesWest;

public:
   /**
    * Creates a new TimeZone with the given number of minutes west of GMT.
    *
    * @param minutesWest the number of minutes west of GMT.
    */
   TimeZone(time_t minutesWest = 0);

   /**
    * Destructs this TimeZone.
    */
   virtual ~TimeZone();

   /**
    * Gets the number of minutes west of GMT this TimeZone is.
    *
    * @return the number of minutes west of GMT.
    */
   virtual time_t getMinutesWest();

   /**
    * Gets a TimeZone from its abbreviation.
    *
    * @param tz the abbreviation for the time zone to retrieve (blank for
    *           the local time zone).
    * @param t the time to use to get the local time zone (NULL to use now).
    *
    * @return the TimeZone for the given abbreviation.
    */
   static TimeZone getTimeZone(const char* tz = NULL, time_t* t = NULL);
};

} // end namespace util
} // end namespace monarch
#endif
