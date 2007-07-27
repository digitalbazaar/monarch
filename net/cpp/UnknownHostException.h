/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_UnknownHostException_H
#define db_net_UnknownHostException_H

#include "Exception.h"

namespace db
{
namespace net
{

/**
 * An UnknownHostException is raised when host's internet address cannot
 * be resolved.
 *
 * @author Dave Longley
 */
class UnknownHostException : public db::rt::Exception
{
public:
   /**
    * Creates a new UnknownHostException.
    *
    * A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   UnknownHostException(const char* message = NULL, const char* code = NULL);
   
   /**
    * Destructs this UnknownHostException.
    */
   virtual ~UnknownHostException();
};

} // end namespace net
} // end namespace db
#endif
