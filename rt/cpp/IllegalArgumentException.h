/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_IllegalArgumentException_H
#define db_rt_IllegalArgumentException_H

#include "Exception.h"

namespace db
{
namespace rt
{

/**
 * An IllegalArgumentException is raised when an invalid argument is passed
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
      const char* message = NULL, const char* code = NULL);
   
   /**
    * Destructs this IllegalArgumentException.
    */
   virtual ~IllegalArgumentException();
};

} // end namespace rt
} // end namespace db
#endif
