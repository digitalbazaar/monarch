/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
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
   
public:
   /**
    * Creates a new ByteArrayOutputStream that writes to the passed ByteBuffer.
    * 
    * @param b the ByteBuffer to write to.
    */
   ByteArrayOutputStream(ByteBuffer* b);
   
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
   virtual bool write(const char* b, unsigned int length);
   
   /**
    * Gets the ByteBuffer that this stream writes to.
    * 
    * @return the ByteBuffer that this stream writes to.
    */
   virtual ByteBuffer* getBytes();
};

} // end namespace io
} // end namespace db
#endif
