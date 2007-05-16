/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef MalformedUrlException_H
#define MalformedUrlException_H

#include "Exception.h"

namespace db
{
namespace net
{

/**
 * A MalformedUrlException is thrown when a URL is malformed.
 *
 * @author Dave Longley
 */
class MalformedUrlException : public db::rt::Exception
{
public:
   /**
    * Creates a new MalformedUrlException.
    *
    * A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   MalformedUrlException(
      const std::string& message = "", const std::string& code = "");
   
   /**
    * Destructs this MalformedUrlException.
    */
   virtual ~MalformedUrlException();
};

} // end namespace net
} // end namespace db
#endif
