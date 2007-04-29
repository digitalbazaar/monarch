/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Exception_H
#define Exception_H

#include <string>

#include "Object.h"

namespace db
{
namespace rt
{

/**
 * An Exception is thrown when some kind of unhandled behavior occurs.
 *
 * @author Dave Longley
 */
class Exception : public virtual Object
{
protected:
   /**
    * A message for this Exception.
    */
   std::string mMessage;

   /**
    * A code associated with this Exception.
    */
   std::string mCode;

public:
   /**
    * Creates a new Exception. A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   Exception(std::string message = "", std::string code = "");

   /**
    * Gets the message for this Exception.
    *
    * @return the message for this Exception.
    */
   const std::string& getMessage();

   /**
    * Gets the code for this Exception.
    *
    * @return the code for this Exception.
    */
   const std::string& getCode();
};

} // end namespace rt
} // end namespace db
#endif
