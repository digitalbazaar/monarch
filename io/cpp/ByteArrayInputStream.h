/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_ByteArrayInputStream_H
#define db_io_ByteArrayInputStream_H

#include "InputStream.h"

namespace db
{
namespace io
{

/**
 * A ByteArrayInputStream is used to read bytes from a byte array.
 * 
 * @author Dave Longley
 */
class ByteArrayInputStream : public db::io::InputStream
{
protected:
   /**
    * The byte array to read from.
    */
   const char* mBytes;
   
   /**
    * The number of bytes remaining in the byte array.
    */
   unsigned int mLength;

public:
   /**
    * Creates a new ByteArrayInputStream.
    * 
    * @param b the byte array to read from.
    * @param length the length of the byte array.
    */
   ByteArrayInputStream(const char* b, unsigned int length);
   
   /**
    * Destructs this ByteArrayInputStream.
    */
   virtual ~ByteArrayInputStream();
   
   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of -1 will be returned if the end of the stream has been reached,
    * otherwise the number of bytes read will be returned.
    * 
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or -1 if the end of the
    *         stream has been reached.
    */
   virtual int read(char* b, unsigned int length);
   
   /**
    * Sets the byte array to read from.
    * 
    * @param b the byte array to read from.
    * @param length the length of the byte array.
    */
   virtual void setByteArray(const char* b, unsigned int length); 
};

} // end namespace net
} // end namespace db
#endif
