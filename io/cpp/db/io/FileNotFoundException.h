/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_FileNotFoundException_H
#define db_io_FileNotFoundException_H

#include "db/io/IOException.h"

namespace db
{
namespace io
{

/**
 * A FileNotFoundException is raised when a file cannot be found.
 *
 * @author Dave Longley
 */
class FileNotFoundException : public db::io::IOException
{
public:
   /**
    * Creates a new FileNotFoundException.
    *
    * A message and code may be optionally specified.
    *
    * @param message the message for this Exception.
    * @param code the code for this Exception.
    */
   FileNotFoundException(const char* message = NULL, const char* code = NULL);
   
   /**
    * Destructs this FileNotFoundException.
    */
   virtual ~FileNotFoundException();
};

} // end namespace io
} // end namespace db
#endif
