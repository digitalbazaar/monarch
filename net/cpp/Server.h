/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_Server_H
#define db_net_Server_H

#include "ConnectionHandler.h"
#include "DatagramHandler.h"
#include "Kernel.h"
#include "OperationList.h"

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
    * A pointer to the appropriate handler. The handler can also be
    * retrieved as Runnables.
    */
   union
   {
      ConnectionHandler* connectionHandler;
      DatagramHandler* datagramHandler;
      db::rt::Runnable* runnable;
   };
   
   /**
    * The Operation that is running a Connection or Datagram handler.
    */
   db::modest::Operation* mOperation;
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
    * All of the running Connection and Datagram handlers.
    */
   db::modest::OperationList mRunningHandlers;
   
   /**
    * The maximum number of concurrent connections to handle.
    */
   unsigned int mMaxConnectionCount;
   
   /**
    * The current number of connections to this Server.
    */
   unsigned int mConnectionCount;
   
   /**
    * Connection handler is a friend so it can access the connection
    * semaphore.
    */
   friend class ConnectionHandler;
   
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
   
   // FIXME: change this to ConnectionServicer and have ConnectionHandlers
   // be allocated and freed internally
   /**
    * Adds a ConnectionHandler to this Server or replaces an existing one.
    * 
    * @param h the ConnectionHandler to add.
    */
   virtual void addConnectionHandler(ConnectionHandler* h);
   
   // FIXME: change this to DatagramServicer and have DatagramHandler
   // be allocated and freed internally
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
    * Gets the Kernel used to run Operations for this Server.
    */
   virtual db::modest::Kernel* getKernel();
   
   /**
    * Sets the maximum number of concurrent connections this Server should
    * allow.
    * 
    * @param count the maximum number of concurrent connections this Server
    *        should allow.
    */
   virtual void setMaxConnectionCount(unsigned int count);
   
   /**
    * Gets the maximum number of concurrent connections this Server allows.
    * 
    * @return the maximum number of concurrent connections this Server allows.
    */
   virtual unsigned int getMaxConnectionCount();
   
   /**
    * Gets the current number of connections to this Server.
    * 
    * @return the current number of connections to this Server.
    */
   virtual unsigned int getConnectionCount();
};

} // end namespace net
} // end namespace db
#endif
