/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_Server_H
#define db_net_Server_H

#include "ConnectionHandler.h"
#include "DatagramHandler.h"
#include "Kernel.h"

#include <map>

namespace db
{
namespace net
{

/**
 * A PortHandler is either a ConnectionHandler or a DatagramHandler.
 */
typedef struct PortHandler
{
   /**
    * The type of port handler is 0 for a ConnectionHandler and 1
    * for a DatagramHandler.
    */
   int type;
   
   /**
    * A pointer to the appropriate handler.
    */
   union
   {
      ConnectionHandler* connectionHandler;
      DatagramHandler* datagramHandler;
   };
};

/**
 * A Server communicates by using Connections that are established on
 * ports that this Server listens to or by using Datagrams that are
 * sent/received on ports that this Server binds to.
 * 
 * This Server maintains a list of PortHandlers that either use
 * Connections or Datagrams to handle traffic over a port.
 * 
 * @author Dave Longley
 */
class Server : public virtual db::rt::Object
{
protected:
   /**
    * The modest Kernel used to run this server.
    */
   db::modest::Kernel* mKernel;
   
   /**
    * A map of ports to PortHandlers.
    */
   std::map<unsigned short, PortHandler*> mPortHandlers;
   
   /**
    * True if this server is running, false if not.
    */
   bool mRunning;
   
   /**
    * The current number of connections to this Server.
    */
   unsigned long mConnectionCount;
   
   /**
    * Gets the PortHandler associated with the given port or NULL if none
    * exists.
    * 
    * @return the PortHandler associated with the given port or NULL if
    *         none exists.
    */
   virtual PortHandler* getPortHandler(unsigned short port);
   
   /**
    * Creates a PortHandler for the given port if one does not already exist.
    * 
    * @return the PortHandler associated with the given port.
    */
   virtual PortHandler* createPortHandler(unsigned short port);
   
public:
   /**
    * Creates a new Server that runs using the passed modest Kernel.
    * 
    * @param k the modest Kernel used to run this Server.
    */
   Server(db::modest::Kernel* k);
   
   /**
    * Destructs this Server.
    */
   virtual ~Server();
   
   /**
    * Adds a ConnectionHandler to this Server or replaces an existing one.
    * 
    * @param h the ConnectionHandler to add.
    */
   virtual void addConnectionHandler(ConnectionHandler* h);
   
   /**
    * Adds a DatagramHandler to this Server or replaces an existing one.
    * 
    * @param h the DatagramHandler to add.
    */
   virtual void addDatagramHandler(DatagramHandler* h);
   
   /**
    * Starts this Server if it isn't already running.
    */
   virtual void start();
   
   /**
    * Stops this Server if it is running.
    */
   virtual void stop();
   
   /**
    * Returns true if this Server is running.
    * 
    * @return true if this Server is running, false if not.
    */
   virtual bool isRunning();
   
   /**
    * Gets the current number of connections to this Server.
    * 
    * @return the current number of connections to this Server.
    */
   virtual unsigned long getConnectionCount();
};

} // end namespace net
} // end namespace db
#endif
