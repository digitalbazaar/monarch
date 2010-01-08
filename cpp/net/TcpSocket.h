/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_net_TcpSocket_H
#define monarch_net_TcpSocket_H

#include "monarch/net/AbstractSocket.h"

namespace monarch
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
    * @return true if the file descriptor could be acquired, false if
    *         an exception occurred.
    */
   virtual bool acquireFileDescriptor(
      SocketAddress::CommunicationDomain domain);

   /**
    * Creates a new Socket with the given file descriptor that points to
    * the socket for an accepted connection.
    *
    * @param fd the file descriptor for the socket.
    *
    * @return the allocated Socket.
    */
   virtual Socket* createConnectedSocket(unsigned int fd);

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
} // end namespace monarch
#endif
