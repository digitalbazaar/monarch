/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_Date_H
#define monarch_util_Date_H

#include "monarch/rt/System.h"
#include "monarch/util/TimeZone.h"

#include <string>

namespace monarch
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
    * The number of seconds past the Epoch for this Date.
    */
   time_t mSecondsSinceEpoch;

   /**
    * The broken down time data structure. Based on the local time zone.
    */
   struct tm mBrokenDownTime;

public:
   /**
    * Creates a new Date. The current day and time is used.
    */
   Date();

   /**
    * Creates a new Date.
    *
    * @param seconds the number of seconds since the Epoch.
    */
   Date(time_t seconds);

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
   virtual int yearSince1900();

   /**
    * Gets the full year for this date.
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
    * Sets this date according to the passed number of seconds since the Epoch.
    *
    * @param seconds the number of seconds since the Epoch.
    */
   virtual void setSeconds(time_t seconds);

   /**
    * Gets the number of seconds since the Epoch.
    *
    * @return the number of seconds since the Epoch.
    */
   virtual time_t getSeconds();

   /**
    * Gets this date as a standard datetime, namely, as a string in the format:
    *
    * YYYY-MM-DD HH:MM:SS
    *
    * @param tz the TimeZone to get the Date in (NULL for local time).
    *
    * @return the datetime string.
    */
   virtual std::string getDateTime(TimeZone* tz = NULL);

   /**
    * Gets this date as a standard datetime, namely, as a string in the format:
    *
    * YYYY-MM-DD HH:MM:SS
    *
    * @param includeTandZ true to include T and Z specifications:
    *           YYYY-MM-DDTHH:MM:SSZ, false not to.
    *
    * @return the datetime string using the UTC time zone.
    */
   virtual std::string getUtcDateTime(bool includeTandZ = false);

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

   /**
    * Parses this Date from a UTC date.
    *
    * @param str the string to convert the date from.
    * @param includeTandZ true to include T and Z specifications:
    *           YYYY-MM-DDTHH:MM:SSZ, false not to.
    *
    * @return true if the Date was successfully parsed, false if not.
    */
   virtual bool parseUtcDateTime(const char* str, bool includeTandZ = false);

   /**
    * Gets the current date as a standard datetime, namely, as a string in the
    * format:
    *
    * YYYY-MM-DD HH:MM:SS
    *
    * @param includeTandZ true to include T and Z specifications:
    *           YYYY-MM-DDTHH:MM:SSZ, false not to.
    *
    * @return the datetime string using the UTC time zone.
    */
   static std::string utcDateTime(bool includeTandZ = false);

   /**
    * Gets the number of seconds since the Epoch given a UTC datetime. This
    * is the equivalent of creating a Date object, parsing a UTC datetime,
    * and calling getSeconds().
    *
    * @param str the UTC datetime, NULL for now.
    * @param includeTandZ true to include T and Z specifications:
    *           YYYY-MM-DDTHH:MM:SSZ, false not to.
    *
    * @return the number of seconds since the Epoch.
    */
   static time_t utcSeconds(const char* str = NULL, bool includeTandZ = false);
};

} // end namespace util
} // end namespace monarch
#endif
