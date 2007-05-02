/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SocketException_H
#define SocketException_H

#include "IOException.h"

namespace db
{
namespace net
{

/**
 * A SocketException is thrown when some kind of Socket error occurs.
 *
 * @author Dave Longley
 */
class SocketException : public db::io::IOException
{
public:
   /**
    * Creates a new SocketException.
    *
    * A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   SocketException(
      const std::string& message = "", const std::string& code = "");
   
   /**
    * Destructs this SocketException.
    */
   virtual ~SocketException();
};

} // end namespace net
} // end namespace db
#endif
