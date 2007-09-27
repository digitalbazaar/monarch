/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_InterruptedException_H
#define db_rt_InterruptedException_H

#include "db/rt/Exception.h"

namespace db
{
namespace rt
{

/**
 * An InterruptedException is raised when some kind of operation is interrupted.
 *
 * @author Dave Longley
 */
class InterruptedException : public Exception
{
public:
   /**
    * Creates a new InterruptedException.
    *
    * A message, type, and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param type the type for this Exception.
    * @param code the code for this Exception.
    */
   InterruptedException(
      const char* message = "", const char* code = "", int code = 0);
   
   /**
    * Destructs this InterruptedException.
    */
   virtual ~InterruptedException();
};

} // end namespace rt
} // end namespace db
#endif
