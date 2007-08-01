/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_SslContext_H
#define db_net_SslContext_H

#include <openssl/ssl.h>
#include <string>

#include "TcpSocket.h"

namespace db
{
namespace net
{

/**
 * An SslContext uses on SSL context to produce SslSockets.
 * 
 * @author Dave Longley
 */
class SslContext
{
protected:
   /**
    * The SSL context object.
    */
   SSL_CTX* mContext;
   
public:
   /**
    * Creates a new SslContext.
    * 
    * @param protocol the protocol to be used for this context.
    */
   SslContext(const char* protocol = "SSLv2/SSLv3/TLS");
   
   /**
    * Destructs this SslContext.
    */
   virtual ~SslContext();
   
   /**
    * Creates a new openssl "SSL" object for a TcpSocket.
    * 
    * @param socket the TcpSocket to create the SSL object for.
    * @param true if the socket is a client socket, false if it is a server
    *        socket.
    * 
    * @return the created SSL object.
    */
   virtual SSL* createSSL(TcpSocket* socket, bool client);
};

} // end namespace net
} // end namespace db
#endif
