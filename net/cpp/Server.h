/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_Server_H
#define db_net_Server_H

#include "Kernel.h"
#include "OperationList.h"
#include "InternetAddress.h"
#include "Runnable.h"
#include "ConnectionService.h"
#include "DatagramService.h"

#include <map>

namespace db
{
namespace net
{

/**
 * A PortService has a Runnable service and an operation for running it.
 */
typedef struct PortService
{
   /**
    * The Runnable service.
    */
   db::rt::Runnable* service;
   
   /**
    * The Operation used to run this service.
    */
   db::modest::Operation* operation;
};

/**
 * A Server communicates by using Connections that are established on
 * ports that this server listens to or by using Datagrams received on
 * ports this server binds to.
 * 
 * This server maintains a list of PortServices that service Connection or
 * Datagram traffic over a specific port.
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
    * A map of ports to PortServices.
    */
   std::map<unsigned short, PortService*> mPortServices;
   
   /**
    * True if this server is running, false if not.
    */
   bool mRunning;
   
   /**
    * All of the running PortServices.
    */
   db::modest::OperationList mRunningServices;
   
   /**
    * The maximum number of concurrent connections to handle.
    */
   unsigned int mMaxConnectionCount;
   
   /**
    * The current number of connections to this server.
    */
   unsigned int mConnectionCount;
   
   /**
    * Connection service is a friend so it can access the connection count.
    */
   friend class ConnectionService;
   
   /**
    * Gets the PortService associated with the given port or NULL if none
    * exists.
    * 
    * @return the PortService associated with the given port or NULL if
    *         none exists.
    */
   virtual PortService* getPortService(unsigned short port);
   
   /**
    * Creates a PortService for the given port if one does not already exist,
    * or erases an existing PortService's Runnable service.
    * 
    * @return the PortService associated with the given port.
    */
   virtual PortService* createPortService(unsigned short port);
   
   /**
    * Starts a PortService.
    * 
    * @param ps the PortService to start.
    */
   virtual void startPortService(PortService* ps);
   
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
    * Adds a ConnectionServicer to this server or replaces an existing one.
    * 
    * @param a the address to listen on.
    * @param s the ConnectionServicer to service Connections with.
    * @param p the SocketDataPresenter to use to present data to the servicer.
    */
   virtual void addConnectionService(
      InternetAddress* a, ConnectionServicer* s, SocketDataPresenter* p = NULL);
   
   /**
    * Adds a DatagramService to this server or replaces an existing one.
    * 
    * @param a the address to bind to.
    * @param s the DatagramServicer to service Datagrams with.
    */
   virtual void addDatagramService(InternetAddress* a, DatagramServicer* s);
   
   /**
    * Starts this server if it isn't already running.
    */
   virtual void start();
   
   /**
    * Stops this server if it is running.
    */
   virtual void stop();
   
   /**
    * Returns true if this server is running.
    * 
    * @return true if this server is running, false if not.
    */
   virtual bool isRunning();
   
   /**
    * Gets the Kernel used to run Operations for this server.
    */
   virtual db::modest::Kernel* getKernel();
   
   /**
    * Sets the maximum number of concurrent connections this server should
    * allow.
    * 
    * @param count the maximum number of concurrent connections this server
    *        should allow.
    */
   virtual void setMaxConnectionCount(unsigned int count);
   
   /**
    * Gets the maximum number of concurrent connections this server allows.
    * 
    * @return the maximum number of concurrent connections this server allows.
    */
   virtual unsigned int getMaxConnectionCount();
   
   /**
    * Gets the current number of connections to this server.
    * 
    * @return the current number of connections to this server.
    */
   virtual unsigned int getConnectionCount();
};

} // end namespace net
} // end namespace db
#endif
