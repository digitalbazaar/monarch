/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef TimeZone_H
#define TimeZone_H

#include "Object.h"
#include "IllegalArgumentException.h"

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
class TimeZone : public virtual db::rt::Object
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
    * 
    * @exception IllegalArgumentException thrown if the passed time zone
    *            abbreviation is invalid.
    */
   static TimeZone getTimeZone(const std::string& tz = "")
   throw(db::rt::IllegalArgumentException);
};

} // end namespace util
} // end namespace db
#endif
