/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef InvalidRegexException_H
#define InvalidRegexException_H

#include "Exception.h"

namespace db
{
namespace util
{
namespace regex
{

/**
 * An InvalidRegularExpressionException is thrown when an invalid regular
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
    * A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   InvalidRegexException(
      const std::string& message = "", const std::string& code = "");
   
   /**
    * Destructs this InvalidRegexException.
    */
   virtual ~InvalidRegexException();
};

} // end namespace regex
} // end namespace util
} // end namespace db
#endif
