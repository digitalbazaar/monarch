/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SslContext_H
#define SslContext_H

#include <openssl/ssl.h>
#include <string>

#include "Object.h"
#include "SslSocket.h"

namespace db
{
namespace net
{

/**
 * An SslContext uses on SSL context to produce SslSockets.
 * 
 * @author Dave Longley
 */
class SslContext : public virtual db::rt::Object
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
   SslContext(const std::string& protocol = "SSLv2/SSLv3/TLS");
   
   /**
    * Destructs this SslContext.
    */
   virtual ~SslContext();
   
   /**
    * Creates a new SslSocket that wraps the passed TcpSocket.
    * 
    * @param socket the TcpSocket to wrap.
    * @param cleanup true to reclaim the memory used for the wrapped Socket
    *                upon destruction, false to do nothing.
    */
   virtual SslSocket* createSocket(TcpSocket* socket, bool cleanup = false);
};

} // end namespace net
} // end namespace db
#endif
