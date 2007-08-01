/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_FilterInputStream_H
#define db_io_FilterInputStream_H

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
   
   /**
    * Skips some bytes in the stream. This method will block until the
    * some number of bytes up to specified number of bytes have been skipped
    * or the end of the stream is reached. This method will return the
    * number of bytes skipped or -1 if the end of the stream was reached.
    * 
    * @param count the number of bytes to skip.
    * 
    * @return the actual number of bytes skipped (which may be zero), or -1 if
    *         the end of the stream is reached or an IO exception occurred.
    */
   virtual long skip(long count);
   
   /**
    * Closes the stream.
    */
   virtual void close();
};

} // end namespace io
} // end namespace db
#endif
