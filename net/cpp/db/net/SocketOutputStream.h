/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_SocketOutputStream_H
#define db_net_SocketOutputStream_H

#include "db/io/OutputStream.h"
#include "db/net/Socket.h"

namespace db
{
namespace net
{

/**
 * A SocketOutputStream is used to write bytes to a Socket.
 * 
 * @author Dave Longley
 */
class SocketOutputStream : public db::io::OutputStream
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
   virtual bool write(const char* b, unsigned int length);   
};

} // end namespace net
} // end namespace db
#endif
