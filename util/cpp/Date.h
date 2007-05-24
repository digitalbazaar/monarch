/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Date_H
#define Date_H

#include "Object.h"
#include "System.h"

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
    * The time data structure.
    */
   time_t mTimeData;

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
    * 
    * @return a reference to the string that was written to.
    */
   virtual std::string& format(
      std::string& str,
      const std::string& format = "EEE, d MMM yyyy HH:mm:ss");
   
   /**
    * Parses this Date from the passed string according to the passed format.
    * 
    * @param str the string to convert the date from.
    * @param format the format for the date.
    * 
    * @return a reference to this Date.
    */
   virtual Date& parse(const std::string& str, const std::string& format);
};

} // end namespace util
} // end namespace db
#endif
