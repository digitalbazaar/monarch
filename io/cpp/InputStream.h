/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_io_InputStream_H
#define db_io_InputStream_H

#include "Thread.h"
#include "IOException.h"

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
class InputStream : public virtual db::rt::Object
{
public:
   /**
    * Creates a new InputStream.
    */
   InputStream() {};
   
   /**
    * Destructs this InputStream.
    */
   virtual ~InputStream() {};
   
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
   virtual int read(char* b, unsigned int length) = 0;
   
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
    *         stream has been reached or an IO exception occurred.
    */
   virtual int peek(char* b, unsigned int length);
   
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
   virtual void close() {};
};

inline int InputStream::peek(char* b, unsigned int length)
{
   // extending classes must implement this method if they want support
   db::rt::Thread::setException(new IOException(
      "InputStream::peek() is not implemented!"));
   return -1;
}

inline long InputStream::skip(long count)
{
   long skipped = 0;
   
   if(count > 0)
   {
      // read and dump bytes
      char b[2048];
      int numBytes = 0;
      unsigned int length = (count < 2048) ? count : 2048;
      while((numBytes = read(b, length)) != -1 && count > 0)
      {
         skipped = numBytes;
         count -= numBytes;
         length = (count < 2048) ? count : 2048;
      }
      
      if(skipped == 0 && numBytes == -1)
      {
         skipped = -1;
      }
   }
   
   return skipped;
}

} // end namespace io
} // end namespace db
#endif
