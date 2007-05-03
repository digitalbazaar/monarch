/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef PeekInputStream_H
#define PeekInputStream_H

#include "FilterInputStream.h"

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
    * Reads a single byte from the stream. This method will block until
    * a byte can be read or until the end of the stream is reached.
    * 
    * @param a single byte to populate from the stream.
    * 
    * @return true if a byte was read, false if the end of the stream was
    *         reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual bool read(char& b) throw(IOException);
   
   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of -1 will be returned if the end of the stream has been reached,
    * otherwise the number of bytes read will be returned.
    * 
    * @param b the array of bytes to fill.
    * @param offset the offset at which to start filling the array.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or -1 if the end of the
    *         stream has been reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual int read(char* b, unsigned int offset, unsigned int length)
   throw(IOException);
   
   /**
    * Peeks ahead and looks at some bytes in the stream. This method will block
    * until at least one byte can be read or until the end of the stream is
    * reached. A value of -1 will be returned if the end of the stream has been
    * reached, otherwise the number of bytes read in the peek will be returned.
    * 
    * A subsequent call to read() or peek() will first read any previously
    * peeked-at bytes.
    * 
    * @param b the array of bytes to fill.
    * @param offset the offset at which to start filling the array.
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or -1 if the end of the
    *         stream has been reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual int peek(char* b, unsigned int offset, unsigned int length)
   throw(IOException);
};

} // end namespace io
} // end namespace db
#endif
