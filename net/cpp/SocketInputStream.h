/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SocketInputStream_H
#define SocketInputStream_H

#include "InputStream.h"
#include "Socket.h"

namespace db
{
namespace net
{

/**
 * A SocketInputStream is used to read bytes from a Socket.
 * 
 * @author Dave Longley
 */
class SocketInputStream : public db::io::InputStream
{
protected:
   /**
    * The Socket to read from.
    */
   Socket* mSocket;

public:
   /**
    * Creates a new SocketInputStream.
    * 
    * @param s the Socket to read from.
    */
   SocketInputStream(Socket* s);
   
   /**
    * Destructs this SocketInputStream.
    */
   virtual ~SocketInputStream();
   
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
   virtual bool read(char& b) throw(db::io::IOException);
   
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
