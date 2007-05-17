/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef TcpSocket_H
#define TcpSocket_H

#include "AbstractSocket.h"

namespace db
{
namespace net
{

/**
 * A TcpSocket is a Socket that uses the TCP/IP protocol.
 * 
 * @author Dave Longley
 */
class TcpSocket : public AbstractSocket
{
protected:
   /**
    * Initializes this Socket by acquiring a file descriptor for it. This
    * method must be called before trying to use this TcpSocket.
    * 
    * @param address the SocketAddress for the Socket.
    * 
    * @exception SocketException thrown if the Socket could not be initialized.
    */
   virtual void initialize(SocketAddress* address) throw(SocketException);
   
   /**
    * Creates a new Socket with the given file descriptor that points to
    * the socket for an accepted connection.
    * 
    * @param fd the file descriptor for the socket.
    * 
    * @return the allocated Socket.
    * 
    * @exception SocketException thrown if a socket error occurs.
    */
   virtual Socket* createConnectedSocket(unsigned int fd)
   throw(SocketException);
   
public:
   /**
    * Creates a new TcpSocket.
    */
   TcpSocket();
   
   /**
    * Destructs this TcpSocket.
    */
   virtual ~TcpSocket();
};

} // end namespace net
} // end namespace db
#endif
