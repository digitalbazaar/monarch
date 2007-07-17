/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_ConnectionHandler_H
#define db_net_ConnectionHandler_H

#include "Connection.h"
#include "InternetAddress.h"

namespace db
{
namespace net
{

/**
 * A ConnectionHandler listens for incoming connections on a given address,
 * detects the protocol for communication and passes the connection off
 * to a ConnectionServicer.
 * 
 * @author Dave Longley
 */
class ConnectionHandler : public virtual db::rt::Object
{
protected:
   /**
    * The address to listen on.
    */
   InternetAddress* mAddress;
   
   /**
    * True to clean up the internet address when destructing, false not to.
    */
   bool mCleanup;
   
   /**
    * The number of connections being handled.
    */
   unsigned long mConnectionCount;
   
public:
   /**
    * Creates a new ConnectionHandler that uses the passed address for
    * communication.
    * 
    * @param address the address to listen on.
    * @param cleanup true to clean up the internet address when destructing,
    *                false not to.
    */
   ConnectionHandler(InternetAddress* address, bool cleanup);
   
   /**
    * Destructs this ConnectionHandler.
    */
   virtual ~ConnectionHandler();
   
   /**
    * Gets the current number of connections being handled.
    * 
    * @return the current number of connections being handled.
    */
   virtual unsigned long getConnectionCount();
   
   /**
    * Gets the address for this ConnectionHandler.
    * 
    * @return the address for this ConnectionHandler.
    */
   virtual InternetAddress* getAddress();
};

} // end namespace net
} // end namespace db
#endif
