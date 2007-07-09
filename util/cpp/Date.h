/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_Date_H
#define db_util_Date_H

#include "Object.h"
#include "System.h"
#include "TimeZone.h"

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
class Date : public virtual db::rt::Object
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
    * Writes this Date to a string according to the passed format.
    * 
    * @param str the string to write the date to.
    * @param format the format to get the Date in.
    * @param tz the TimeZone to get the Date in (NULL for local time).
    * 
    * @return a reference to the string that was written to.
    */
   virtual std::string& format(
      std::string& str,
      const std::string& format = "EEE, d MMM yyyy HH:mm:ss",
      TimeZone* tz = NULL);
   
   /**
    * Parses this Date from the passed string according to the passed format.
    * 
    * @param str the string to convert the date from.
    * @param format the format for the date.
    * 
    * @return true if the Date was successfully parsed, false if not.
    */
   virtual bool parse(const std::string& str, const std::string& format);
};

} // end namespace util
} // end namespace db
#endif
