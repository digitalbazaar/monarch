/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_IOException_H
#define db_io_IOException_H

#include "Exception.h"

namespace db
{
namespace io
{

/**
 * An IOException is raised when some kind of IO error occurs.
 *
 * @author Dave Longley
 */
class IOException : public db::rt::Exception
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
   IOException(const char* message = NULL, const char* code = NULL);
   
   /**
    * Destructs this IOException.
    */
   virtual ~IOException();
};

} // end namespace io
} // end namespace db
#endif
