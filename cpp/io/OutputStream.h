/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_io_OutputStream_H
#define monarch_io_OutputStream_H

#include "monarch/rt/Collectable.h"

namespace monarch
{
namespace io
{

/**
 * An OutputStream is the abstract base class for all classes that represent an
 * output stream of bytes.
 *
 * If an exception occurs during an operation it can be retrieved via
 * getException().
 *
 * @author Dave Longley
 */
class OutputStream
{
public:
   /**
    * Creates a new OutputStream.
    */
   OutputStream();

   /**
    * Destructs this OutputStream.
    */
   virtual ~OutputStream();

   /**
    * Writes some bytes to the stream.
    *
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    *
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool write(const char* b, int length) = 0;

   /**
    * Forces this stream to flush its output, if any of it was buffered.
    *
    * Default implementation simply returns true.
    *
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool flush();

   /**
    * Forces this stream to finish its output, if the stream has such a
    * function.
    *
    * Default implementation simply returns true.
    *
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool finish();

   /**
    * Closes the stream.
    *
    * Default implementation is a no-op.
    */
   virtual void close();
};

// typedef for a counted reference to an OutputStream
typedef monarch::rt::Collectable<OutputStream> OutputStreamRef;

} // end namespace io
} // end namespace monarch
#endif
