/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef TcpSocket_H
#define TcpSocket_H

#include "Socket.h"

namespace db
{
namespace net
{

/**
 * A TcpSocket is a Socket that uses the TCP/IP protocol.
 * 
 * @author Dave Longley
 */
class TcpSocket : public virtual Socket
{
protected:
   /**
    * Initializes this Socket by acquiring a file descriptor for it. This
    * method must be called before trying to use this TcpSocket.
    * 
    * @exception SocketException thrown if the Socket could not be initialized.
    */
   virtual void initialize() throw(SocketException);
   
public:
   /**
    * Creates a new TcpSocket.
    */
   TcpSocket();
   
   /**
    * Destructs this TcpSocket.
    */
   virtual ~TcpSocket();
   
   /**
    * Binds this Socket to a SocketAddress.
    * 
    * @param address the address to bind to.
    * 
    * @exception SocketException thrown if the address could not be bound.
    */
   virtual void bind(SocketAddress* address) throw(SocketException);
   
   /**
    * Listens for a connection to this Socket and accepts it. This method
    * will block until a connection is made to this Socket.
    * 
    * The passed socket will be initialized to the file descriptor that points
    * to the socket that can be used to communicate with the connected socket.
    * 
    * @param socket the socket to use to communicate with the connected socket.
    * @param timeout the timeout, in seconds, 0 for no timeout.
    * 
    * @exception SocketException thrown if a socket error occurs.
    * @exception SocketTimeoutException thrown if the timeout is reached
    *            before a connection was made.
    */
   virtual void accept(Socket* socket, unsigned int timeout)
   throw(SocketException, SocketTimeoutException);
   
   /**
    * Connects this Socket to the given address.
    * 
    * @param address the address to connect to.
    * @param timeout the timeout, in seconds, 0 for no timeout.
    * 
    * @exception SocketException thrown if a socket error occurs.
    */
   virtual void connect(SocketAddress* address, unsigned int timeout)
   throw(SocketException);
};

} // end namespace net
} // end namespace db
#endif
