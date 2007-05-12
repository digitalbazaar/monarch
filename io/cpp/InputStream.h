/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef InputStream_H
#define InputStream_H

#include "Object.h"
#include "IOException.h"

namespace db
{
namespace io
{

/**
 * An InputStream is the abstract base class for all classes that represent an
 * input stream of bytes.
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
   virtual bool read(char& b) throw(IOException) = 0;
   
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

inline int InputStream::read(char* b, unsigned int length)
throw(IOException)
{
   int rval = -1;
   
   char c;
   for(unsigned int i = 0; i < length && read(c); i++)
   {
      b[i] = c;
      rval = (rval == -1) ? 1 : rval + 1;
   }
   
   return rval;
}

inline int InputStream::peek(char* b, unsigned int length)
throw(IOException)
{
   // extending classes must implement this method if they want support
   throw IOException("InputStream::peek() is not implemented!");
}

inline unsigned long InputStream::skip(unsigned long count) throw(IOException)
{
   // read and dump bytes
   char b[2048];
   unsigned int length = (count < 2048) ? count : 2048; 
   int numBytes = 0;
   unsigned long skipped = 0;
   while((numBytes = read(b, length)) != -1 && count > 0)
   {
      skipped += numBytes;
      count -= numBytes;
      length = (count < 2048) ? count : 2048;
   }
   
   return numBytes;
}

inline void InputStream::close() throw(IOException)
{
   // nothing to do in base class
}

} // end namespace io
} // end namespace db
#endif
