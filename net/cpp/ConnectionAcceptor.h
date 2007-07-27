/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_ConnectionAccepter_H
#define db_net_ConnectionAcceptor_H

#include "Runnable.h"
#include "Socket.h"
#include "ConnectionService.h"

namespace db
{
namespace net
{

/**
 * A ConnectionAcceptor accepts a Socket connection and passes the connected
 * Socket to the associated ConnectionService.
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
    * The ConnectionService.
    */
   ConnectionService* mService;
   
public:
   /**
    * Creates a new ConnectionAcceptor that uses the passed server Socket to
    * accept connections.
    * 
    * @param socket the server Socket to use to accept connections.
    * @param service the ConnectionService to pass the connected Socket to.
    */
   ConnectionAcceptor(Socket* socket, ConnectionService* service);
   
   /**
    * Destructs this ConnectionAcceptor.
    */
   virtual ~ConnectionAcceptor();
   
   /**
    * Accepts a socket connection and passes it to a ConnectionService.
    */
   virtual void run();
};

} // end namespace net
} // end namespace db
#endif
