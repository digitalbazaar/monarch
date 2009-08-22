/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_io_ByteArrayOutputStream_H
#define db_io_ByteArrayOutputStream_H

#include "db/io/ByteBuffer.h"
#include "db/io/OutputStream.h"

namespace db
{
namespace io
{

/**
 * A ByteArrayOutputStream is used to write bytes to a single byte array.
 *
 * @author Dave Longley
 */
class ByteArrayOutputStream : public OutputStream
{
protected:
   /**
    * The ByteBuffer to write to.
    */
   ByteBuffer* mBuffer;

   /**
    * True if this stream should resize the underlying buffer as necessary,
    * false if not.
    */
   bool mResize;

public:
   /**
    * Creates a new ByteArrayOutputStream that writes to the passed ByteBuffer.
    *
    * @param b the ByteBuffer to write to.
    * @param resize true to resize the underlying ByteBuffer as necessary,
    *               false not to.
    */
   ByteArrayOutputStream(ByteBuffer* b, bool resize = true);

   /**
    * Destructs this ByteArrayOutputStream.
    */
   virtual ~ByteArrayOutputStream();

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
    * Gets the ByteBuffer that this stream writes to.
    *
    * @return the ByteBuffer that this stream writes to.
    */
   virtual ByteBuffer* getByteArray();

   /**
    * Sets whether or not this stream should resize the underlying ByteBuffer
    * as necessary.
    *
    * @param resize true to resize the underlying ByteBuffer as necessary,
    *               false to never resize.
    */
   virtual void setResize(bool resize);

   /**
    * Gets whether or not this stream will resize the underlying ByteBuffer
    * as necessary.
    *
    * @return true if this stream will resize the underlying ByteBuffer as
    *         necessary, false if it will not.
    */
   virtual bool getResize();
};

} // end namespace io
} // end namespace db
#endif
