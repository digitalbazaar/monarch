/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef IOException_H
#define IOException_H

#include "Exception.h"

namespace db
{
namespace io
{

/**
 * An IOException is thrown when some kind of IO error occurs.
 *
 * @author Dave Longley
 */
class IOException : public virtual db::rt::Exception
{
public:
   /**
    * Creates a new IOException.
    *
    * A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   IOException(std::string message = "", std::string code = "");
};

} // end namespace io
} // end namespace db
#endif
