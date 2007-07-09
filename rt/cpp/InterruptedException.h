/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_rt_InterruptedException_H
#define db_rt_InterruptedException_H

#include "Exception.h"

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
    * A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   InterruptedException(
      const std::string& message = "", const std::string& code = "");
   
   /**
    * Destructs this InterruptedException.
    */
   virtual ~InterruptedException();
};

} // end namespace rt
} // end namespace db
#endif
