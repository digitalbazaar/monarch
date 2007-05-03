/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SslInputStream_H
#define SslInputStream_H

#include "FilterInputStream.h"
#include "SslSocket.h"

namespace db
{
namespace net
{

/**
 * An SslInputStream is used to read SSL data from an SslSocket.
 * 
 * @author Dave Longley
 */
class SslInputStream : public db::io::FilterInputStream
{
protected:
   /**
    * The SslSocket to read from.
    */
   SslSocket* mSocket;

public:
   /**
    * Creates a new SslInputStream.
    * 
    * @param is the s the SslSocket to read from.
    */
   SslInputStream(SslSocket* s);
   
   /**
    * Destructs this SslSocketInputStream.
    */
   virtual ~SslInputStream();
   
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
   throw(db::io::IOException);
};

} // end namespace net
} // end namespace db
#endif
