/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_io_IgnoreOutputStream_H
#define db_io_IgnoreOutputStream_H

#include "monarch/io/FilterOutputStream.h"

namespace db
{
namespace io
{

/**
 * An IgnoreOutputStream is a special output stream that can ignore a number
 * of bytes before actually writing to the underlying stream.
 *
 * @author Dave Longley
 */
class IgnoreOutputStream : public monarch::io::FilterOutputStream
{
protected:
   /**
    * The number of bytes to ignore.
    */
   int mIgnoreCount;

public:
   /**
    * Creates a new IgnoreOutputStream that writes to the passed OutputStream.
    *
    * @param os the OutputStream to write to.
    * @param cleanup true to clean up the passed OutputStream when destructing,
    *                false not to.
    */
   IgnoreOutputStream(OutputStream* os, bool cleanup = false);

   /**
    * Destructs this IgnoreOutputStream.
    */
   virtual ~IgnoreOutputStream();

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
    * Sets the number of bytes to ignore before writing to the underlying
    * stream.
    *
    * @param count the number of bytes to ignore before writing to the
    *              underlying stream.
    */
   virtual void setIgnoreCount(int count);

   /**
    * Gets the number of bytes to ignore before writing to the underlying
    * stream.
    *
    * @return the number of bytes to ignore before writing to the
    *         underlying stream.
    */
   virtual int getIgnoreCount();
};

} // end namespace io
} // end namespace db
#endif
