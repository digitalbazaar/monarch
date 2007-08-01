/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_TimeZone_H
#define db_util_TimeZone_H

#include <string>

namespace db
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
   unsigned int mMinutesWest;
   
   /**
    * Creates a new TimeZone with the given number of minutes west of GMT.
    * 
    * @param minutesWest the number of minutes west of GMT.
    */
   TimeZone(unsigned int minutesWest = 0);

public:
   /**
    * Destructs this TimeZone.
    */
   virtual ~TimeZone();
   
   /**
    * Gets the number of minutes west of GMT this TimeZone is.
    * 
    * @return the number of minutes west of GMT.
    */
   virtual unsigned int getMinutesWest();
   
   /**
    * Gets a TimeZone from its abbreviation.
    * 
    * @param tz the abbreviation for the time zone to retrieve (blank for
    *           the local time zone).
    * 
    * @return the TimeZone for the given abbreviation.
    */
   static TimeZone getTimeZone(const std::string& tz = "");
};

} // end namespace util
} // end namespace db
#endif
