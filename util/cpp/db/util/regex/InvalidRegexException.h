/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_util_regex_InvalidRegexException_H
#define db_util_regex_InvalidRegexException_H

#include "db/rt/Exception.h"

namespace db
{
namespace util
{
namespace regex
{

/**
 * An InvalidRegularExpressionException is raised when an invalid regular
 * expression is detected.
 *
 * @author Dave Longley
 */
class InvalidRegexException : public db::rt::Exception
{
public:
   /**
    * Creates a new InvalidRegexException.
    *
    * A message, type, and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param type the type for this Exception.
    * @param code the code for this Exception.
    */
   InvalidRegexException(
      const char* message = "", const char* type = "", int code = 0);
   
   /**
    * Destructs this InvalidRegexException.
    */
   virtual ~InvalidRegexException();
};

} // end namespace regex
} // end namespace util
} // end namespace db
#endif
