/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef FilterInputStream_H
#define FilterInputStream_H

#include "InputStream.h"

namespace db
{
namespace io
{

/**
 * A FilterInputStream is a base class for input streams that filter the
 * data from another input stream in some fashion.
 * 
 * @author Dave Longley
 */
class FilterInputStream : public InputStream
{
protected:
   /**
    * The underlying input stream to read from.
    */
   InputStream* mInputStream;
   
   /**
    * True to cleanup the underlying input stream when destructing,
    * false not to.
    */
   bool mCleanupInputStream;
   
public:
   /**
    * Creates a new FilterInputStream that reads from the passed InputStream.
    * 
    * @param is the InputStream to read from.
    * @param cleanup true to clean up the passed InputStream when destructing,
    *                false not to.
    */
   FilterInputStream(InputStream* is, bool cleanup = false);
   
   /**
    * Destructs this FilterInputStream.
    */
   virtual ~FilterInputStream();
   
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
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or -1 if the end of the
    *         stream has been reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual int read(char* b, unsigned int length) throw(IOException);
   
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
    * @param length the maximum number of bytes to read into the buffer.
    * 
    * @return the number of bytes read from the stream or -1 if the end of the
    *         stream has been reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual int peek(char* b, unsigned int length) throw(IOException);
   
   /**
    * Skips some bytes in the stream. This method will block until the
    * some number of bytes up to specified number of bytes have been skipped
    * or the end of the stream is reached. This method will return the
    * number of bytes skipped.
    * 
    * @param count the number of bytes to skip.
    * 
    * @return the actual number of bytes skipped (which may be zero), or -1 if
    *         the end of the stream is reached.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual unsigned long skip(unsigned long count) throw(IOException);
   
   /**
    * Closes the stream.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual void close() throw(IOException);
};

} // end namespace io
} // end namespace db
#endif
