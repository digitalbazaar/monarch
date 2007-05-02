/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SslSocket_H
#define SslSocket_H

#include "TcpSocket.h"
#include "SocketWrapper.h"

namespace db
{
namespace net
{

/**
 * An SslSocket is a Socket that uses the TCP/IP protocol and the Secure
 * Sockets Layer (SSL v2/v3) and Transport Layer Security (TLS v1).
 * 
 * @author Dave Longley
 */
class SslSocket : public SocketWrapper
{
public:
   /**
    * Creates a new SslSocket that wraps the passed TcpSocket.
    * 
    * @param socket the TcpSocket to wrap.
    * @param cleanup true to reclaim the memory used for the wrapped Socket
    *                upon destruction, false to do nothing.
    */
   SslSocket(TcpSocket* socket, bool cleanup = false);
   
   /**
    * Destructs this SslSocket.
    */
   virtual ~SslSocket();
};

} // end namespace net
} // end namespace db
#endif
