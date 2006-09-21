/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.util.Iterator;

/**
 * A ProxyPortWebConnectionServer is a WebConnectionServer with one port used
 * for all incoming web connections. Once a connection is established, is it
 * proxied to an internal WebConnectionServer that will service it.
 * 
 * A ProxyPortWebConnectionServer allows a WebConnectionServer to be
 * constructed such that only one port must be opened for external access,
 * yet multiple protocols can be supported. For instance, an SSL-client may
 * connect to a ProxyPortWebConnectionServer's proxy port -- which accepts any
 * connection protocol and it will be forwarded to the internal web server for
 * servicing as an SSL connection.
 * 
 * Any WebConnectionHandler added to this proxy port web server will be added
 * to its internal web server. This ProxyPortWebServer uses only one port for
 * incoming external traffic and passes all connections to the internal
 * web server. 
 * 
 * @author Dave Longley
 */
public class ProxyPortWebConnectionServer extends WebConnectionServer
{
   /**
    * The internal web connection server.
    */
   protected WebConnectionServer mInternalServer;
   
   /**
    * The proxy port for this proxy port web server.
    */
   protected int mProxyPort;
   
   /**
    * The default proxy port.
    */
   protected static final int DEFAULT_PROXY_PORT = 8080; 
   
   /**
    * The web connection handler for connections to this proxy port web
    * server.
    */
   protected WebConnectionHandler mWebConnectionHandler;
   
   /**
    * The proxy web connection servicers for this proxy port web server.
    */
   protected ProxyPortWebConnectionServicer mWebConnectionServicer;
   
   /**
    * Creates a new ProxyPortWebConnectionServer.
    */
   public ProxyPortWebConnectionServer()
   {
      this(new WebConnectionServer());
   }
   
   /**
    * Creates a new ProxyPortWebConnectionServer using the given web server as
    * the internal web server.
    * 
    * @param internalServer the web connection server to use internally, behind
    *                       the proxy port.
    */
   public ProxyPortWebConnectionServer(WebConnectionServer internalServer)
   {
      // set the internal web connection server
      mInternalServer = internalServer;

      // create the web connection handler
      mWebConnectionHandler = createWebConnectionHandler();
      
      // set default proxy port
      setProxyPort(DEFAULT_PROXY_PORT);
   }
   
   /**
    * Creates the web connection handler for this proxy port web server.
    *
    * @return the web connection handler for this proxy port web server.
    */
   protected WebConnectionHandler createWebConnectionHandler()
   {
      // create the proxy port web connection servicer
      mWebConnectionServicer = new ProxyPortWebConnectionServicer();
      
      // create a generic web connection handler
      // the web connection handler is generic so that it will accept
      // all web connection protocols 
      WebConnectionHandler handler =
         new GenericWebConnectionHandler(mWebConnectionServicer);
      
      return handler;
   }
   
   /**
    * Gets the internal web connection server.
    * 
    * @return the internal web connection server.
    */
   protected WebConnectionServer getInternalServer()
   {
      return mInternalServer;
   }
   
   /**
    * Adds a web connection handler to the internal web server.
    * 
    * Only one web connection handler is permitted per port.
    * 
    * A web connection handler will fail to be added to listen on the
    * given port if there already exists another web connection handler
    * listening on that port.

    * If the web server is already running, then the web connection handler
    * will begin accepting connections on the specified port.
    * 
    * @param wch the web connection handler to add to this server.
    * @param port the port the web connection handler will listen on.
    * @return true if the web connection handler was successfully added,
    *         false if it was not.
    */
   public synchronized boolean addWebConnectionHandler(
      WebConnectionHandler wch, int port)
   {
      // add the web connection handler to the priority list for
      // determining supported protocols
      mWebConnectionServicer.addPrioritizedWebConnectionHandler(wch);
      
      // add the web connection handler to the internal web server
      return getInternalServer().addWebConnectionHandler(wch, port);
   }
   
   /**
    * Removes a web connection handler from the internal web server.
    * 
    * If the web server is already running, then it will stop accepting
    * connections on all ports and its connections be terminated.
    * 
    * @param wch the web connection handler to remove from this server.
    */
   public synchronized void removeWebConnectionHandler(
      WebConnectionHandler wch)
   {
      // remove the web connection handler from the internal web server
      getInternalServer().removeWebConnectionHandler(wch);
      
      // remove the web connection handler from the priority list
      mWebConnectionServicer.removePrioritizedWebConnectionHandler(wch);
   }
   
   /**
    * Removes all web connection handlers from the internal web server.
    */
   public synchronized void removeAllWebConnectionHandlers()
   {
      // remove all web connection handlers from internal web server
      getInternalServer().removeAllWebConnectionHandlers();
      
      // remove all web connection handlers from the priority list
      mWebConnectionServicer.removeAllPrioritizedWebConnectionHandlers();
   }

   /**
    * Starts the internal web server and this web server. All web connection
    * handlers will begin accepting connections.
    * 
    * @param proxyPort the proxy port to accept connections on.
    */
   public synchronized void start(int proxyPort)
   {
      setProxyPort(proxyPort);
      start();
   }   
   
   /**
    * Starts the internal web server and this web server using the
    * default proxy port. All web connection handlers will begin
    * accepting connections.
    */
   public synchronized void start()
   {
      // only start server if it isn't running
      if(!isRunning())
      {
         getLogger().debug(getClass(), "starting proxy port web server...");
         
         // add the web connection handler
         super.addWebConnectionHandler(mWebConnectionHandler, getProxyPort());
         
         // start the internal web server
         getInternalServer().start();
         
         // add all web connection handlers in the internal server to the
         // prioritized list that aren't already present in the list
         for(Iterator i = getInternalServer().getWebConnectionHandlers().
             iterator(); i.hasNext();)
         {
            // add the prioritized web connection handler
            WebConnectionHandler wch = (WebConnectionHandler)i.next();
            mWebConnectionServicer.addPrioritizedWebConnectionHandler(wch);
         }
         
         // start the proxy port web server
         // start accepting connections on all web connection handlers
         for(Iterator i = mPorts.iterator(); i.hasNext();)
         {
            int port = Integer.parseInt((String)i.next());
            WebConnectionHandler handler = super.getWebConnectionHandler(port);
            
            // start accepting web connections
            handler.startAcceptingWebConnections(port);
         }
         
         // start accepting connections on all unassigned web connection
         // handlers
         for(Iterator i = mUnassignedWebConnectionHandlers.iterator();
             i.hasNext();)
         {
            WebConnectionHandler handler = (WebConnectionHandler)i.next();
            
            // start accepting web connections on an ephemeral port
            handler.startAcceptingWebConnections(0);
            
            // add port to the port list
            mPorts.add("" + handler.getPort());
            
            // so assign the port to the handler
            mPortToWebConnectionHandler.put(handler.getPort(), handler);
            
            // remove the handler from the unassigned list
            i.remove();
         }
         
         // server is running
         mRunning = true;
      }
      
      getLogger().debug(getClass(), "proxy port web server started.");      
   }
   
   /**
    * Stops the internal web server and this web server. All web connection
    * handlers will stop accepting connections and terminate any connections
    * they are currently servicing.
    */
   public synchronized void stop()
   {
      // only stop server if it is running
      if(isRunning())
      {
         // stop the proxy port server
         getLogger().debug(getClass(), "stopping proxy port server...");
         
         // stop and terminate all connections on all web connection handlers
         for(Iterator i = mPorts.iterator(); i.hasNext();)
         {
            int port = Integer.parseInt((String)i.next());
            WebConnectionHandler handler = super.getWebConnectionHandler(port);
            
            // stop accepting web connections
            handler.stopAcceptingWebConnections();
            
            // terminate web connections
            handler.terminateWebConnections();
            
            // disconnect web connections
            handler.disconnectWebConnections();
         }
         
         // stop the internal web server
         getInternalServer().stop();
         
         // remove web connection handler
         super.removeWebConnectionHandler(mWebConnectionHandler);
         
         // server is not running
         mRunning = false;
      }
      
      getLogger().debug(getClass(), "proxy port web server stopped."); 
   }
   
   /**
    * Gets the web connection handler for the specified port for the
    * internal web server.
    * 
    * @param port the port to get the web connection handler for.
    * 
    * @return the web connection handler for the specified port or null
    *         if no web connection handler is assigned to the port.
    */
   public WebConnectionHandler getWebConnectionHandler(int port)
   {
      // get the web connection handler from the internal web server
      return getInternalServer().getWebConnectionHandler(port);
   }
   
   /**
    * Sets the proxy port for this proxy port web server. The port will
    * only be set if the server is not running.
    * 
    * @param proxyPort the proxy port for this proxy port web server.
    */
   public void setProxyPort(int proxyPort)
   {
      if(!isRunning())
      {
         // save maximum connections
         int connections = getMaximumProxyConnections();
         
         // set port
         mProxyPort = proxyPort;

         // update maximum connections
         setMaximumProxyConnections(connections);
      }
   }
   
   /**
    * Gets the proxy port for this proxy port web server.
    * 
    * @return the proxy port for this proxy port web server.
    */
   public int getProxyPort()
   {
      return mProxyPort;
   }
   
   /**
    * Sets the maximum number of connections for the proxy port.
    * 
    * If connections is 0, then there will be no maximum.
    * 
    * @param connections the maximum number of connections for the proxy port.
    */
   public void setMaximumProxyConnections(int connections)
   {
      mWebConnectionHandler.setMaxConcurrentConnections(connections);
   }
   
   /**
    * Gets the maximum number of connections for the proxy port.
    * 
    * If connections is 0, then there is no maximum.
    * 
    * @return the maximum number of connections for the proxy port.
    */
   public int getMaximumProxyConnections()
   {
      return mWebConnectionHandler.getMaxConcurrentConnections();
   }
}
