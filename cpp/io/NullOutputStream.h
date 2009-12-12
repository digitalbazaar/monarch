/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_io_NullOutputStream_H
#define db_io_NullOutputStream_H

#include "monarch/io/OutputStream.h"

namespace db
{
namespace io
{

/**
 * A NullOutputStream silently consumes any data written to it, such that
 * nothing is written out.
 *
 * @author Dave Longley
 */
class NullOutputStream : public OutputStream
{
public:
   /**
    * Creates a new NullOutputStream.
    */
   NullOutputStream() {};

   /**
    * Destructs this NullOutputStream.
    */
   virtual ~NullOutputStream() {};

   /**
    * Writes some bytes to the stream.
    *
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    *
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool write(const char* b, int length) { return true; };
};

} // end namespace io
} // end namespace db
#endif
