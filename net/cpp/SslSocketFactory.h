/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SslSocketFactory_H
#define SslSocketFactory_H

#include "Object.h"
#include "SslSocket.h"

namespace db
{
namespace net
{

/**
 * An SslSocketFactory uses on SSL context to produce SslSockets.
 * 
 * @author Dave Longley
 */
class SslSocketFactory : public virtual db::rt::Object
{
public:
   /**
    * Creates a new SslSocketFactory.
    */
   SslSocketFactory();
   
   /**
    * Destructs this SslSocketFactory.
    */
   virtual ~SslSocketFactory();
   
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
