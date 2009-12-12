/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_SocketInputStream_H
#define db_net_SocketInputStream_H

#include "monarch/io/InputStream.h"
#include "monarch/net/Socket.h"

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
   virtual int read(char* b, int length);
};

} // end namespace net
} // end namespace db
#endif
