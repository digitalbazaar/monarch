/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_io_ByteArrayInputStream_H
#define monarch_io_ByteArrayInputStream_H

#include "monarch/io/InputStream.h"
#include "monarch/io/ByteBuffer.h"

namespace monarch
{
namespace io
{

/**
 * A ByteArrayInputStream is used to read bytes from a byte array or,
 * alternatively, from a ByteBuffer.
 *
 * @author Dave Longley
 */
class ByteArrayInputStream : public monarch::io::InputStream
{
protected:
   /**
    * The byte array to read from.
    */
   const char* mBytes;

   /**
    * The number of bytes remaining in the byte array.
    */
   int mLength;

   /**
    * The ByteBuffer to read from.
    */
   ByteBuffer* mBuffer;

   /**
    * True to clean up the ByteBuffer when destructing, false not to.
    */
   bool mCleanupBuffer;

public:
   /**
    * Creates a new ByteArrayInputStream.
    *
    * @param b the byte array to read from.
    * @param length the length of the byte array.
    */
   ByteArrayInputStream(const char* b, int length);

   /**
    * Creates a new ByteArrayInputStream that reads from a ByteBuffer.
    *
    * @param b the ByteBuffer to read from.
    * @param cleanup true to clean up ByteBuffer when destructing, false not to.
    */
   ByteArrayInputStream(ByteBuffer* b, bool cleanup = false);

   /**
    * Destructs this ByteArrayInputStream.
    */
   virtual ~ByteArrayInputStream();

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

   /**
    * Sets the byte array to read from.
    *
    * @param b the byte array to read from.
    * @param length the length of the byte array.
    */
   virtual void setByteArray(const char* b, int length);

   /**
    * Sets the ByteBuffer to read from.
    *
    * @param b the ByteBuffer to read from.
    * @param cleanup true to clean up ByteBuffer when destructing, false not to.
    */
   virtual void setByteBuffer(ByteBuffer* b, bool cleanup = false);
};

} // end namespace io
} // end namespace monarch
#endif
