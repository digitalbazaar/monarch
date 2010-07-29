/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_WebServer_H
#define monarch_ws_WebServer_H

#include "monarch/config/ConfigManager.h"
#include "monarch/net/Server.h"
#include "monarch/net/SocketDataPresenterList.h"
#include "monarch/net/SslContext.h"
#include "monarch/ws/WebServiceContainer.h"

namespace monarch
{
namespace ws
{

/**
 * A WebServer is a service that runs on a monarch::net::Server that serves
 * the services in a WebServiceContainer.
 *
 * @author Dave Longley
 */
class WebServer
{
protected:
   /**
    * The server this WebServer is running on.
    */
   monarch::net::Server* mServer;

   /**
    * The WebServiceContainer for this server.
    */
   WebServiceContainerRef mContainer;

   /**
    * The server address for http traffic.
    */
   monarch::net::InternetAddressRef mHostAddress;

   /**
    * The root SSL context for https traffic.
    */
   monarch::net::SslContextRef mSslContext;

   /**
    * The SocketDataPresenterList for handling the socket presentation layer.
    */
   monarch::net::SocketDataPresenterListRef mSocketDataPresenterList;

   /**
    * The service ID for the WebServiceContainer's HttpConnectionServicer.
    */
   monarch::net::Server::ServiceId mServiceId;

public:
   /**
    * Creates a new WebServer.
    */
   WebServer();

   /**
    * Destructs this WebServer.
    */
   virtual ~WebServer();

   /**
    * Initializes this WebServer. Must be called before start() at least once.
    * To reconfigure the WebServer after it has stopped, call cleanup() then
    * initialize() with the new configuration.
    *
    * @param cfg the configuration to use.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool initialize(monarch::config::Config& cfg);

   /**
    * Cleans up this WebServer.
    */
   virtual void cleanup();

   /**
    * Starts this WebServer.
    *
    * @param server the Server to add this WebServer to.
    * @param name a unique identifier for this WebServer on the given Server.
    *
    * @return true if successful, false if not.
    */
   virtual bool start(
      monarch::net::Server* server, const char* name = "WebServer");

   /**
    * Stops this WebServer.
    */
   virtual void stop();

   /**
    * Sets this WebServer's container.
    *
    * @param c the container to use.
    */
   virtual void setContainer(WebServiceContainerRef& c);

   /**
    * Gets this WebServer's container. The container can be used to add or
    * remove WebServices.
    *
    * @return the WebServiceContainer for this WebServer.
    */
   virtual WebServiceContainerRef& getContainer();

   /**
    * Gets the host address for this WebServer.
    *
    * @return the host address for this WebServer.
    */
   virtual monarch::net::InternetAddressRef getHostAddress();

   /**
    * Gets the SslContext for this WebServer. Useful for adding virtual host
    * entries.
    *
    * @return the SslContext for this WebServer.
    */
   virtual monarch::net::SslContextRef getSslContext();
};

} // end namespace ws
} // end namespace monarch
#endif
