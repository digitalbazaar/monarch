/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SocketTimeoutException_H
#define SocketTimeoutException_H

#include "SocketException.h"

namespace db
{
namespace net
{

/**
 * A SocketTimeoutException is thrown when a socket timeout occurs.
 *
 * @author Dave Longley
 */
class SocketTimeoutException : public virtual SocketException
{
public:
   /**
    * Creates a new SocketTimeoutException.
    *
    * A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   SocketTimeoutException(std::string message = "", std::string code = "");
};

} // end namespace net
} // end namespace db
#endif
