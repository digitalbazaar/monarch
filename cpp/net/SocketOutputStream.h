/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_net_SocketOutputStream_H
#define monarch_net_SocketOutputStream_H

#include "monarch/io/OutputStream.h"
#include "monarch/net/Socket.h"

namespace monarch
{
namespace net
{

/**
 * A SocketOutputStream is used to write bytes to a Socket.
 *
 * @author Dave Longley
 */
class SocketOutputStream : public monarch::io::OutputStream
{
protected:
   /**
    * The Socket to write to.
    */
   Socket* mSocket;

public:
   /**
    * Creates a new SocketOutputStream.
    *
    * @param s the Socket to write to.
    */
   SocketOutputStream(Socket* s);

   /**
    * Destructs this SocketOutputStream.
    */
   virtual ~SocketOutputStream();

   /**
    * Writes some bytes to the stream.
    *
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    *
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool write(const char* b, int length);
};

} // end namespace net
} // end namespace monarch
#endif
