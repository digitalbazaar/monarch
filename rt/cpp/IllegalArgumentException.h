/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef IllegalArgumentException_H
#define IllegalArgumentException_H

#include "Exception.h"

namespace db
{
namespace rt
{

/**
 * An IllegalArgumentException is thrown when an invalid argument is passed
 * to a method.
 *
 * @author Dave Longley
 */
class IllegalArgumentException : public Exception
{
public:
   /**
    * Creates a new IllegalArgumentException.
    *
    * A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   IllegalArgumentException(
      const std::string& message = "", const std::string& code = "");
   
   /**
    * Destructs this IllegalArgumentException.
    */
   virtual ~IllegalArgumentException();
};

} // end namespace rt
} // end namespace db
#endif
