/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_io_FilterOutputStream_H
#define monarch_io_FilterOutputStream_H

#include "monarch/io/OutputStream.h"

namespace monarch
{
namespace io
{

/**
 * A FilterOutputStream is a base class for output streams that filter data
 * in some fashion before it is written out to another output stream.
 *
 * @author Dave Longley
 */
class FilterOutputStream : public OutputStream
{
protected:
   /**
    * The underlying output stream to write to.
    */
   OutputStream* mOutputStream;

   /**
    * True to cleanup the underlying output stream when destructing,
    * false not to.
    */
   bool mCleanupOutputStream;

public:
   /**
    * Creates a new FilterOutputStream that writes to the passed OutputStream.
    *
    * @param os the OutputStream to write to.
    * @param cleanup true to clean up the passed OutputStream when destructing,
    *                false not to.
    */
   FilterOutputStream(OutputStream* os, bool cleanup = false);

   /**
    * Destructs this FilterOutputStream.
    */
   virtual ~FilterOutputStream();

   /**
    * Writes some bytes to the stream.
    *
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    *
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool write(const char* b, int length);

   /**
    * Forces this stream to flush its output, if any of it was buffered.
    *
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool flush();

   /**
    * Forces this stream to finish its output, if the stream has such a
    * function.
    *
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool finish();

   /**
    * Closes the stream.
    */
   virtual void close();
};

} // end namespace io
} // end namespace monarch
#endif
