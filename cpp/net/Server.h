/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_net_Server_H
#define monarch_net_Server_H

#include "monarch/modest/OperationRunner.h"
#include "monarch/modest/OperationList.h"
#include "monarch/net/InternetAddress.h"
#include "monarch/net/PortService.h"
#include "monarch/net/ConnectionService.h"
#include "monarch/net/ConnectionServicer.h"
#include "monarch/net/SocketDataPresenter.h"
#include "monarch/net/DatagramService.h"
#include "monarch/net/DatagramServicer.h"

#include <list>
#include <map>

namespace monarch
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
class Server
{
public:
   /**
    * A unique ID for a service that resides on this Server.
    */
   typedef uint32_t ServiceId;

   /**
    * An invalid ServiceId.
    */
   static ServiceId sInvalidServiceId;

protected:
   /**
    * The modest OperationRunner used to run this server.
    */
   monarch::modest::OperationRunner* mOperationRunner;

   /**
    * A list of available ServiceIds from removed services.
    */
   typedef std::list<ServiceId> ServiceIdFreeList;
   ServiceIdFreeList mServiceIdFreeList;

   /**
    * A map of ServiceId to PortService.
    */
   typedef std::map<ServiceId, PortService*> PortServiceMap;
   PortServiceMap mPortServices;

   /**
    * True if this server is running, false if not.
    */
   bool mRunning;

   /**
    * The maximum number of connections for this server.
    */
   int32_t mMaxConnections;

   /**
    * The current number of connections for this server.
    */
   int32_t mCurrentConnections;

   /**
    * A lock for synchronizing the use of this server.
    */
   monarch::rt::ExclusiveLock mLock;

   /**
    * Connection service is a friend so it can access the connection count.
    */
   friend class ConnectionService;

public:
   /**
    * Creates a new Server.
    */
   Server();

   /**
    * Destructs this Server.
    */
   virtual ~Server();

   /**
    * Adds a ConnectionService to this server or replaces an existing one. The
    * ConnectionService will be automatically deleted once it is removed from
    * the server. If the server is running the service will be started. The
    * added service will be assigned an ID which can be used to remove the
    * service if desired.
    *
    * @param a the address to listen on.
    * @param s the ConnectionServicer to service Connections with.
    * @param p the SocketDataPresenter to use to present data to the servicer.
    * @param name a name for the connection service.
    * @param maxConnections the maximum number of current connections.
    * @param backlog the number of connections to backlog.
    *
    * @return the ServiceId for the new service if the service was added, 0
    *         if the service could not be added -- if the server is running
    *         and the service could not be started, it will not be added, if
    *         the server is not running, the service will be added.
    */
   virtual ServiceId addConnectionService(
      InternetAddress* a, ConnectionServicer* s, SocketDataPresenter* p = NULL,
      const char* name = "unnamed",
      int maxConnections = 100, int backlog = 100);

   /**
    * Adds a DatagramService to this server or replaces an existing one. The
    * DatagramService will be automatically deleted once it is removed from
    * the server. If the server is running the service will be started. The
    * added service will be assigned an ID which can be used to remove the
    * service if desired.
    *
    * @param a the address to bind to.
    * @param s the DatagramServicer to service Datagrams with.
    * @param name a name for the datagram service.
    *
    * @return the ServiceId for the new service if the service was added, 0
    *         if the service could not be added -- if the server is running
    *         and the service could not be started, it will not be added, if
    *         the server is not running, the service will be added.
    */
   virtual ServiceId addDatagramService(
      InternetAddress* a, DatagramServicer* s,
      const char* name = "unnamed");

   /**
    * Adds a new PortService. If the server is running, the new service is
    * started. The PortService's memory must be managed externally.
    *
    * @param ps the new PortService to add.
    *
    * @return the ServiceId for the PortService if it was added/started, 0
    *         if it was not added/failed to start and should be deleted.
    */
   virtual ServiceId addPortService(PortService* ps);

   /**
    * Removes a ConnectionService or DatagramService by its assigned ID. If
    * the service is running, it will be stopped before it is removed.
    *
    * @param id the ServiceId of the service to remove.
    *
    * @return true if a service was actually removed, false if not.
    */
   virtual bool removePortService(ServiceId id);

   /**
    * Gets the PortService associated with the given ServiceId or NULL if none
    * exists.
    *
    * @param id the ServiceId associated with the PortService.
    *
    * @return the PortService associated with the given ServiceId or NULL if
    *         none exists.
    */
   virtual PortService* getPortService(ServiceId id);

   /**
    * Gets the PortService associated with the given ServiceId or NULL if none
    * exists.
    *
    * @param id the ServiceId associated with the PortService.
    *
    * @return the PortService associated with the given ServiceId or NULL if
    *         none exists.
    */
   virtual ConnectionService* getConnectionService(ServiceId id);

   /**
    * Gets the PortService associated with the given ServiceId or NULL if none
    * exists.
    *
    * @param id the ServiceId associated with the PortService.
    *
    * @return the PortService associated with the given ServiceId or NULL if
    *         none exists.
    */
   virtual DatagramService* getDatagramService(ServiceId id);

   /**
    * Starts this server if it isn't already running. If any service fails to
    * start, an exception will be set and start will fail.
    *
    * @param opRunner the OperationRunner used to run this Server.
    *
    * @return true if all of the services for this server started, false if
    *         at least one failed.
    */
   virtual bool start(monarch::modest::OperationRunner* opRunner);

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
   virtual monarch::modest::OperationRunner* getOperationRunner();

   /**
    * Sets the maximum number of concurrent connections this server should
    * allow.
    *
    * @param count the maximum number of concurrent connections this server
    *        should allow.
    */
   virtual void setMaxConnectionCount(int32_t count);

   /**
    * Gets the maximum number of concurrent connections this server allows.
    *
    * @return the maximum number of concurrent connections this server allows.
    */
   virtual int32_t getMaxConnectionCount();

   /**
    * Gets the current number of connections to this server.
    *
    * @return the current number of connections to this server.
    */
   virtual int32_t getConnectionCount();
};

} // end namespace net
} // end namespace monarch
#endif
