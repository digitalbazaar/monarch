/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_io_InputStream_H
#define db_io_InputStream_H

#include "monarch/rt/Collectable.h"

namespace db
{
namespace io
{

/**
 * An InputStream is the abstract base class for all classes that represent an
 * input stream of bytes.
 *
 * If an exception occurs during an operation it can be retrieved via
 * getException().
 *
 * @author Dave Longley
 */
class InputStream
{
public:
   /**
    * Creates a new InputStream.
    */
   InputStream();

   /**
    * Destructs this InputStream.
    */
   virtual ~InputStream();

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
   virtual int read(char* b, int length) = 0;

   /**
    * Peeks ahead and looks at some bytes in the stream. If specified, this
    * method will block until at least one byte can be read or until the end of
    * the stream is reached. A value of 0 will be returned if the end of the
    * stream has been reached and block is true, otherwise the number of bytes
    * read in the peek will be returned. If block is false, 0 may be returned
    * before the end of the stream is reached.
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
    * @return the number of bytes peeked from the stream or 0 if the end of the
    *         stream has been reached or -1 if an IO exception occurred.
    */
   virtual int peek(char* b, int length, bool block = true);

   /**
    * Skips some bytes in the stream. This method will block until the
    * some number of bytes up to specified number of bytes have been skipped
    * or the end of the stream is reached. This method will return the
    * number of bytes skipped or 0 if the end of the stream was reached or
    * -1 if an IO exception occurred.
    *
    * @param count the number of bytes to skip.
    *
    * @return the actual number of bytes skipped, or -1 if the end of the
    *         stream is reached or -1 if an IO exception occurred.
    */
   virtual long long skip(long long count);

   /**
    * Closes the stream.
    */
   virtual void close() {};
};

// typedef for a counted reference to an InputStream
typedef db::rt::Collectable<InputStream> InputStreamRef;

} // end namespace io
} // end namespace db
#endif
