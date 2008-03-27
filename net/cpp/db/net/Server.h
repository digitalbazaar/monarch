/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_Server_H
#define db_net_Server_H

#include "db/rt/Semaphore.h"
#include "db/modest/OperationRunner.h"
#include "db/modest/OperationList.h"
#include "db/net/InternetAddress.h"
#include "db/net/PortService.h"
#include "db/net/ConnectionServicer.h"
#include "db/net/SocketDataPresenter.h"
#include "db/net/DatagramServicer.h"

#include <map>

namespace db
{
namespace net
{

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
    * The modest OperationRunner used to run this server.
    */
   db::modest::OperationRunner* mOperationRunner;
   
   /**
    * A map of ports to PortServices.
    */
   std::map<unsigned short, PortService*> mPortServices;
   
   /**
    * True if this server is running, false if not.
    */
   bool mRunning;
   
   /**
    * The connection semaphore for this server.
    */
   db::rt::Semaphore mConnectionSemaphore;
   
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
    * Adds a new PortService for the given port. If an old service exists
    * on the given port, it is stopped and removed. If the server is running,
    * the new service is started.
    * 
    * @param ps the new PortService to add.
    * 
    * @return true if the service was added/started properly.
    */
   virtual bool addPortService(PortService* ps);
   
public:
   /**
    * Creates a new Server that runs using the passed modest OperationRunner.
    * 
    * @param opRunner the OperationRunner used to run this Server.
    */
   Server(db::modest::OperationRunner* opRunner);
   
   /**
    * Destructs this Server.
    */
   virtual ~Server();
   
   /**
    * Adds a ConnectionServicer to this server or replaces an existing one. If
    * the server is running the service will be started.
    * 
    * @param a the address to listen on.
    * @param s the ConnectionServicer to service Connections with.
    * @param p the SocketDataPresenter to use to present data to the servicer.
    * 
    * @return if the server is running: true if the service started, false
    *         if not; if the server is not running: true.
    */
   virtual bool addConnectionService(
      InternetAddress* a, ConnectionServicer* s, SocketDataPresenter* p = NULL);
   
   /**
    * Adds a DatagramService to this server or replaces an existing one. If
    * the server is running the service will be started.
    * 
    * @param a the address to bind to.
    * @param s the DatagramServicer to service Datagrams with.
    * 
    * @return if the server is running: true if the service started, false
    *         if not; if the server is not running: true.
    */
   virtual bool addDatagramService(InternetAddress* a, DatagramServicer* s);
   
   /**
    * Starts this server if it isn't already running. If a service fails to
    * start, an exception should be set on the current thread indicating why.
    * 
    * @return true if all of the services for this server started, false if
    *         at least one failed.
    */
   virtual bool start();
   
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
    * Gets the OperationRunner used to run Operations for this server.
    * 
    * @return the OperationRunner used to run Operations for this server.
    */
   virtual db::modest::OperationRunner* getOperationRunner();
   
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
