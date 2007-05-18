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
    * Acquiring a file descriptor for this Socket. This method must be called
    * before trying to use this Socket.
    * 
    * This method is called automatically by the default implementation.
    * 
    * @param domain the communication domain for this Socket (i.e. IPv4, IPv6).
    * 
    * @exception SocketException thrown if a file descriptor could not be
    *            acquired.
    */
   virtual void acquireFileDescriptor(const std::string& domain)
   throw(SocketException);
   
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
