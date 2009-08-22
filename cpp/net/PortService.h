/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_net_PortService_H
#define db_net_PortService_H

#include "db/net/InternetAddress.h"
#include "db/modest/Operation.h"

namespace db
{
namespace net
{

// forward declare Server
class Server;

/**
 * A PortService is a Runnable service that handles network traffic over
 * a port.
 *
 * @author Dave Longley
 */
class PortService : public db::rt::Runnable
{
protected:
   /**
    * A name for this port service.
    */
   char* mName;

   /**
    * The Server associated with this PortService.
    */
   Server* mServer;

   /**
    * The address for this service.
    */
   InternetAddress* mAddress;

   /**
    * The Operation used to run this service.
    */
   db::modest::Operation mOperation;

   /**
    * Initializes this service and creates the Operation for running it,
    * typically through the Server's OperationRunner. If the service could
    * not be initialized, an exception should be set on the current thread
    * indicating the reason why the service could not be initialized.
    *
    * @return the Operation for running this service, or NULL if the
    *         service could not be initialized.
    */
   virtual db::modest::Operation initialize() = 0;

   /**
    * Called to clean up resources for this service that were created or
    * obtained via a call to initialize(). If there are no resources to
    * clean up, then this method should have no effect.
    */
   virtual void cleanup() = 0;

public:
   /**
    * Creates a new PortService for a Server.
    *
    * @param server the Server this service is for.
    * @param address the address for this service.
    * @param name a name for this port service.
    */
   PortService(
      Server* server, InternetAddress* address, const char* name = "unnamed");

   /**
    * Destructs this PortService.
    */
   virtual ~PortService();

   /**
    * Starts this PortService or restarts it if it is already running. If
    * this service fails to start, an exception will be set on the current
    * thread.
    *
    * @return true if this PortService started, false if not.
    */
   virtual bool start();

   /**
    * Runs this PortService.
    */
   virtual void run() = 0;

   /**
    * Interrupts this PortService.
    */
   virtual void interrupt();

   /**
    * Stops this PortService.
    */
   virtual void stop();

   /**
    * Gets the address for this PortService.
    *
    * @return the address for this PortService.
    */
   virtual InternetAddress* getAddress();
};

} // end namespace net
} // end namespace db
#endif
