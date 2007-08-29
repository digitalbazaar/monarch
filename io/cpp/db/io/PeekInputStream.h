/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_PeekInputStream_H
#define db_io_PeekInputStream_H

#include "db/io/FilterInputStream.h"

namespace db
{
namespace io
{

/**
 * A PeekInputStream is a FilterInputStream that provides a buffer for peeking
 * ahead at the data in the underlying InputStream.
 * 
 * @author Dave Longley
 */
class PeekInputStream : public FilterInputStream
{
protected:
   /**
    * A buffer for peeking ahead.
    */
   char* mPeekBuffer;
   
   /**
    * The size of the peek buffer.
    */
   size_t mPeekSize; 
   
   /**
    * The current offset in the peek buffer.
    */
   size_t mPeekOffset;
   
   /**
    * The number of bytes in the peek buffer. 
    */
   size_t mPeekLength;

public:
   /**
    * Creates a new PeekInputStream that reads from the passed InputStream.
    * 
    * @param is the InputStream to read from.
    * @param cleanup true to clean up the passed InputStream when destructing,
    *                false not to.
    */
   PeekInputStream(InputStream* is, bool cleanup = false);
   
   /**
    * Destructs this PeekInputStream.
    */
   virtual ~PeekInputStream();
   
   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of -1 will be returned if the end of the stream has been reached
    * or an IO exception occurred, otherwise the number of bytes read will be
    * returned.
    * 
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or -1 if the end of the
    *         stream has been reached or an IO exception occurred.
    */
   virtual int read(char* b, unsigned int length);
   
   /**
    * Peeks ahead and looks at some bytes in the stream. This method will block
    * until at least one byte can be read or until the end of the stream is
    * reached. A value of -1 will be returned if the end of the stream has been
    * reached, otherwise the number of bytes read in the peek will be returned.
    * 
    * A subsequent call to read() or peek() will first read any previously
    * peeked-at bytes. If desired, peek() can be called without blocking
    * and will return the number of bytes read from the peek buffer, which
    * may be zero.
    * 
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    * @param block true to block, false to return only those bytes in
    *              the peek buffer.
    * 
    * @return the number of bytes read from the stream or -1 if the end of the
    *         stream has been reached or an IO exception occurred.
    */
   virtual int peek(char* b, unsigned int length, bool block = true);
};

} // end namespace io
} // end namespace db
#endif
