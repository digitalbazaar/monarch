/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_Date_H
#define db_util_Date_H

#include "db/rt/System.h"
#include "db/util/TimeZone.h"

#include <string>

namespace db
{
namespace util
{

/**
 * A Date represents a calendar date.
 * 
 * @author Dave Longley
 */
class Date
{
protected:
   /**
    * The number of seconds past the epoch.
    */
   time_t mSecondsSinceEpoch;
   
   /**
    * The broken down GMT data structure.
    */
   struct tm mBrokenDownTime;

public:
   /**
    * Creates a new Date. The current day and time is used.
    */
   Date();
   
   /**
    * Destructs this Date.
    */
   virtual ~Date();
   
   /**
    * Gets the second for this date (0-59).
    * 
    * @return the second for this date.
    */
   virtual int second();
   
   /**
    * Gets the minute for this date (0-59).
    * 
    * @return the minute for this date.
    */
   virtual int minute();
   
   /**
    * Gets the hour for this date (0-23).
    * 
    * @return the hour for this date.
    */
   virtual int hour();
   
   /**
    * Gets the day of the month for this date (1-31).
    * 
    * @return the day of the month for this date.
    */
   virtual int day();
   
   /**
    * Gets the month for this date (0-11).
    * 
    * @return the month for this date.
    */
   virtual int month();
   
   /**
    * Gets the year for this date (since 1900).
    * 
    * @return the year for this date.
    */
   virtual int year();
   
   /**
    * Sets this date according to an MS-DOS date & time.
    * 
    * @param dosTime the MS-DOS date & time.
    */
   virtual void setDosTime(unsigned int dosTime);
   
   /**
    * Gets this date in MS-DOS format.
    * 
    * @param local true to use the local time, false not to.
    * 
    * @return this date in MS-DOS format.
    */
   virtual unsigned int dosTime(bool local = true);
   
   /**
    * Adds seconds to this date.
    * 
    * @param seconds the seconds to add to this date (can be negative).
    */
   virtual void addSeconds(time_t seconds);
   
   /**
    * Writes this Date to a string according to the passed format.
    * 
    * Here is an example of a date format (uses C language formatting):
    * 
    * "%a, %d %b %Y %H:%M:%S" = Sat, 21 Jan 2006 19:15:46 GMT
    * 
    * @param str the string to write the date to.
    * @param format the format to get the Date in.
    * @param tz the TimeZone to get the Date in (NULL for local time).
    * 
    * @return a reference to the string that was written to.
    */
   virtual std::string& format(
      std::string& str,
      const char* format = "%a, %d %b %Y %H:%M:%S",
      TimeZone* tz = NULL);
   
   /**
    * Parses this Date from the passed string according to the passed format.
    * 
    * @param str the string to convert the date from.
    * @param format the format for the date.
    * @param tz the TimeZone the Date is in (NULL for local time).
    * 
    * @return true if the Date was successfully parsed, false if not.
    */
   virtual bool parse(
      const char* str, const char* format, TimeZone* tz = NULL);
   
   /**
    * Writes this Date to a string according to the passed format.
    * 
    * Here is an example of a date format (uses C language formatting):
    * 
    * "%a, %d %b %Y %H:%M:%S" = Sat, 21 Jan 2006 19:15:46 GMT
    * 
    * @param format the format to get the Date in.
    * @param tz the TimeZone to get the Date in (NULL for local time).
    * 
    * @return a reference to the string that was written to.
    */
   virtual std::string toString(
      const char* format = "%a, %d %b %Y %H:%M:%S",
      TimeZone* tz = NULL);
};

} // end namespace util
} // end namespace db
#endif
