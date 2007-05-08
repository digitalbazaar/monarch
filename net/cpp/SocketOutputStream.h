/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SocketOutputStream_H
#define SocketOutputStream_H

#include "OutputStream.h"
#include "Socket.h"

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
    * Writes a single byte to the stream.
    * 
    * @param b the byte to write.
    * 
    * @exception IOException thrown if an IO error occurs. 
    */
   virtual void write(const char& b) throw(db::io::IOException);
   
   /**
    * Writes some bytes to the stream.
    * 
    * @param b the array of bytes to write.
    * @param length the number of bytes to write to the stream.
    * 
    * @exception IOException thrown if an IO error occurs. 
    */
   virtual void write(const char* b, unsigned int length)
   throw(db::io::IOException);
};

} // end namespace net
} // end namespace db
#endif
