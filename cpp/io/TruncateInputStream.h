/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_io_TruncateInputStream_H
#define monarch_io_TruncateInputStream_H

#include "monarch/io/FilterInputStream.h"

#include <inttypes.h>

namespace monarch
{
namespace io
{

/**
 * A TruncateInputStream is an InputStream that is used to read a limited
 * amount of data from a stream, truncating the rest.
 *
 * @author Dave Longley
 */
class TruncateInputStream : public FilterInputStream
{
protected:
   /**
    * The maximum amount of data to allow to pass before truncating.
    */
   uint64_t mMax;

   /**
    * The total amount of data that has passed before max has been hit.
    */
   uint64_t mTotal;

public:
   /**
    * Creates a new TruncateInputStream.
    *
    * @param max the amount of data to allow to pass before truncating.
    * @param is the InputStream to read from.
    * @param cleanup true to clean up the passed InputStream when destructing,
    *                false not to.
    */
   TruncateInputStream(
      uint64_t max, InputStream* is, bool cleanup = false);

   /**
    * Destructs this TruncateInputStream.
    */
   virtual ~TruncateInputStream();

   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of 0 will be returned if the end of the stream has been reached,
    * a value of -1 will be returned if an IO exception occurred, otherwise
    * the number of bytes read will be returned.
    *
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    *
    * @return the number of bytes read from the stream or 0 if the end of the
    *         stream has been reached or -1 if an IO exception occurred.
    */
   virtual int read(char* b, int length);
};

} // end namespace io
} // end namespace monarch
#endif
