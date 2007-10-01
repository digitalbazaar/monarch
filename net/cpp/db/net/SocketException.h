/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_SocketException_H
#define db_net_SocketException_H

#include "db/io/IOException.h"

namespace db
{
namespace net
{

/**
 * A SocketException is raised when some kind of Socket error occurs.
 *
 * @author Dave Longley
 */
class SocketException : public db::io::IOException
{
public:
   /**
    * Creates a new SocketException.
    *
    * A message, type, and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param type the type for this Exception.
    * @param code the code for this Exception.
    */
   SocketException(
      const char* message = "",
      const char* type = "db.net.Socket", int code = 0);
   
   /**
    * Destructs this SocketException.
    */
   virtual ~SocketException();
};

} // end namespace net
} // end namespace db
#endif
