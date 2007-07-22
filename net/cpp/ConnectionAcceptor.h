/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_ConnectionAccepter_H
#define db_net_ConnectionAcceptor_H

#include "Runnable.h"
#include "Socket.h"
#include "ConnectionHandler.h"

namespace db
{
namespace net
{

/**
 * A ConnectionAcceptor accepts a Socket connection and passes the connected
 * Socket to the associated ConnectionHandler.
 * 
 * @author Dave Longley
 */
class ConnectionAcceptor : public db::rt::Runnable
{
protected:
   /**
    * The server Socket to use to accept Connections.
    */
   Socket* mSocket;
   
   /**
    * The ConnectionHandler.
    */
   ConnectionHandler* mHandler;
   
public:
   /**
    * Creates a new ConnectionAcceptor that uses the passed server Socket to
    * accept connections.
    * 
    * @param s the server Socket to use to accept connections.
    * @param h the ConnectionHandler to pass the connected Socket to.
    */
   ConnectionAcceptor(Socket* s, ConnectionHandler* h);
   
   /**
    * Destructs this ConnectionAcceptor.
    */
   virtual ~ConnectionAcceptor();
   
   /**
    * Accepts a connection and passes it to a ConnectionHandler.
    */
   virtual void run();
};

} // end namespace net
} // end namespace db
#endif
