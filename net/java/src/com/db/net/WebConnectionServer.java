/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.net.InetAddress;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A generic WebConnectionServer. This server communicates using WebConnections
 * that are established on ports that this server listens to.
 * 
 * @author Dave Longley
 */
public class WebConnectionServer
{
   /**
    * The ports for this web server in the order that they were added.
    */
   protected Vector<Integer> mPorts;
   
   /**
    * A table mapping ports to web connection handlers.
    */
   protected HashMap<Integer, WebConnectionHandler> mPortToWebConnectionHandler;
   
   /**
    * A table mapping web connection handlers to bind addresses.
    */
   protected HashMap<WebConnectionHandler, InetAddress>
      mWebConnectionHandlerToBindAddress;
   
   /**
    * A list of web connection handlers that are assigned to use any free
    * ephemeral port -- so that they don't have actual ports set yet until
    * they start accepting web connections. 
    */
   protected Vector<WebConnectionHandler> mUnassignedWebConnectionHandlers;
   
   /**
    * Whether or not this web server is running.
    */
   protected boolean mRunning;
   
   /**
    * Creates a new WebConnectionServer.
    */
   public WebConnectionServer()
   {
      // create list for ports
      mPorts = new Vector<Integer>();
      
      // create the port to handler map
      mPortToWebConnectionHandler =
         new HashMap<Integer, WebConnectionHandler>();
      
      // create the handler to bind address map
      mWebConnectionHandlerToBindAddress =
         new HashMap<WebConnectionHandler, InetAddress>();
      
      // create vector for unassigned web connection handlers
      mUnassignedWebConnectionHandlers = new Vector<WebConnectionHandler>();
      
      // not running by default
      mRunning = false;
   }
   
   /**
    * Disposes this WebConnectionServer. Stops this WebConnectionServer.
    */
   @Override
   protected void finalize()
   {
      stop();
   }
   
   /**
    * Adds a web connection handler to this web server. The handler will
    * bind to 0.0.0.0. Only one web connection handler is permitted per port.
    * 
    * A web connection handler will fail to be added to listen on the
    * given port if there already exists another web connection handler
    * listening on that port.

    * If the web server is already running, then the web connection handler
    * will begin accepting connections on the specified port.
    * 
    * @param wch the web connection handler to add to this server.
    * @param port the port the web connection handler will listen on.
    * 
    * @return true if the web connection handler was successfully added,
    *         false if it was not.
    */
   public synchronized boolean addWebConnectionHandler(
      WebConnectionHandler wch, int port)
   {
      return addWebConnectionHandler(wch, null, port);
   }
   
   /**
    * Adds a web connection handler to this web server that will bind to
    * a specific local address (null indicates 0.0.0.0). Only one web
    * connection handler is permitted per port.
    * 
    * A web connection handler will fail to be added to listen on the
    * given port if there already exists another web connection handler
    * listening on that port.

    * If the web server is already running, then the web connection handler
    * will begin accepting connections on the specified port.
    * 
    * @param wch the web connection handler to add to this server.
    * @param bindAddress the local address to bind to (null indicates 0.0.0.0).
    * @param port the port the web connection handler will listen on.
    * 
    * @return true if the web connection handler was successfully added,
    *         false if it was not.
    */
   public synchronized boolean addWebConnectionHandler(
      WebConnectionHandler wch, InetAddress bindAddress, int port)
   {
      boolean rval = false;
      
      // see if there is already a web connection handler on the provided port
      WebConnectionHandler existingHandler = getWebConnectionHandler(port);
      if(existingHandler == null)
      {
         // if the web server has already started, then start accepting
         // connections on the port
         if(isRunning())
         {
            // start accepting web connections on the port
            wch.startAcceptingWebConnections(bindAddress, port);
         }
         
         if(wch.getPort() > 0 || port > 0)
         {
            // use the larger port number
            port = Math.max(wch.getPort(), port);
            
            // add port to the port list
            mPorts.add(port);
            
            // so assign port to handler and handler to bind address
            mPortToWebConnectionHandler.put(port, wch);
            mWebConnectionHandlerToBindAddress.put(wch, bindAddress);
            
            // the handler was added
            rval = true;
         }
         else if(!isRunning())
         {
            // add an unassigned web connection handler
            mUnassignedWebConnectionHandlers.add(wch);
            
            // assign handler to bind address
            mWebConnectionHandlerToBindAddress.put(wch, bindAddress);
            
            // the handler was added
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Removes a web connection handler from this web server. If the web
    * server is already running, then it will stop accepting connections
    * and its connections be terminated.
    * 
    * @param wch the web connection handler to remove from this server.
    */
   public synchronized void removeWebConnectionHandler(
      WebConnectionHandler wch)
   {
      // try to get the web connection handler
      wch = getWebConnectionHandler(wch.getPort());
      if(wch != null)
      {
         // save port
         int port = wch.getPort();
         
         if(isRunning())
         {
            // stop accepting web connections
            wch.stopAcceptingWebConnections();
            
            // terminate existing web connections
            wch.terminateWebConnections();
            
            // disconnect web connections
            wch.disconnectWebConnections();
         }
         
         // remove the port from the port list
         mPorts.remove(port);
         
         // unmap the handler
         mPortToWebConnectionHandler.remove(port);
      }
      
      // remove the web connection handler from the unassigned list
      mUnassignedWebConnectionHandlers.remove(wch);
      
      // unmap the bind address
      mWebConnectionHandlerToBindAddress.remove(wch);
   }
   
   /**
    * Removes all web connection handlers.
    */
   public synchronized void removeAllWebConnectionHandlers()
   {
      for(WebConnectionHandler handler: mPortToWebConnectionHandler.values())
      {
         // stop accepting web connections
         handler.stopAcceptingWebConnections();
            
         // terminate existing web connections
         handler.terminateWebConnections();
         
         // disconnect web connections
         handler.disconnectWebConnections();
      }
      
      // clear port list
      mPorts.clear();
      
      // clear maps
      mPortToWebConnectionHandler.clear();
      mWebConnectionHandlerToBindAddress.clear();
      
      // clear unassigned list
      mUnassignedWebConnectionHandlers.clear();
   }
   
   /**
    * Starts this web server. All web connection handlers will begin
    * accepting connections.
    */
   public synchronized void start()
   {
      // only start server if it isn't running
      if(!isRunning())
      {
         getLogger().debug(getClass(), "starting web server...");

         // start accepting connections on all assigned web connection handlers
         for(int port: mPorts)
         {
            // start accepting web connections
            WebConnectionHandler handler = getWebConnectionHandler(port);
            handler.startAcceptingWebConnections(getBindAddress(handler), port);
         }
         
         // start accepting connections on all unassigned web connection
         // handlers
         for(Iterator<WebConnectionHandler> i =
             mUnassignedWebConnectionHandlers.iterator(); i.hasNext();)
         {
            WebConnectionHandler handler = i.next();
            
            // start accepting web connections on an ephemeral port
            handler.startAcceptingWebConnections(getBindAddress(handler), 0);
            
            // add port to the port list
            mPorts.add(handler.getPort());
            
            // so assign port to handler
            mPortToWebConnectionHandler.put(handler.getPort(), handler);
            
            // remove the handler from the unassigned list
            i.remove();
         }
         
         // server is running
         mRunning = true;
      }
         
      getLogger().debug(getClass(), "web server started.");
   }
   
   /**
    * Stops this web server. All web connection handlers will stop accepting
    * connections and terminate any connections they are currently servicing.
    */
   public synchronized void stop()
   {
      // only stop server if it is running
      if(isRunning())
      {
         getLogger().debug(getClass(), "stopping web server...");
         
         // stop and terminate all connections on all web connection handlers
         for(int port: mPorts)
         {
            WebConnectionHandler handler = getWebConnectionHandler(port);
            
            // stop accepting web connections
            handler.stopAcceptingWebConnections();
            
            // terminate web connections
            handler.terminateWebConnections();
            
            // disconnect web connections
            handler.disconnectWebConnections();
         }
         
         // server is not running
         mRunning = false;
      }
      
      getLogger().debug(getClass(), "web server stopped.");
   }
   
   /**
    * Returns true if this web server is running, false if not.
    *
    * @return true if this web server is running, false if not.
    */
   public synchronized boolean isRunning()
   {
      return mRunning;
   }
   
   /**
    * Gets the web connection handler for the specified port.
    * 
    * @param port the port to get the web connection handler for.
    * 
    * @return the web connection handler for the specified port or null
    *         if no web connection handler is assigned to the port.
    */
   public WebConnectionHandler getWebConnectionHandler(int port)
   {
      return mPortToWebConnectionHandler.get(port);
   }
   
   /**
    * Gets the local bind address for the given web connection handler. This
    * method will retrieve the bind address for the handler even if the
    * handler is not accepting web connections yet.
    * 
    * @param wch the web connection handler to get the local bind address for.
    * 
    * @return the local bind address the web connection handler uses/will use.
    */
   public InetAddress getBindAddress(WebConnectionHandler wch)
   {
      return mWebConnectionHandlerToBindAddress.get(wch);
   }
   
   /**
    * Gets a list of all of the web connection handlers for this 
    * web server in the order that they were added to this web server.
    * 
    * @return a list of all of the web connection handlers for this
    *         web server in the order that they were added to this web server.
    */
   public Collection<WebConnectionHandler> getWebConnectionHandlers()
   {
      Vector<WebConnectionHandler> vector = new Vector<WebConnectionHandler>();
      
      // add all assigned web connection handlers according to port order
      for(int port: mPorts)
      {
         // add handler
         WebConnectionHandler handler = getWebConnectionHandler(port);
         vector.add(handler);
      }
      
      // add all unassigned web connection handlers
      vector.addAll(mUnassignedWebConnectionHandlers);
      
      return vector;
   }
   
   /**
    * Gets a list of all of the ports for this web server in the order
    * that they were added to this web server.
    * 
    * @return a list of all of the ports for this web server in the order
    *         that they were added to this web server.
    */
   public int[] getWebConnectionHandlerPorts()
   {
      int[] ports = new int[mPorts.size()];
      
      int n = 0;
      for(int port: mPorts)
      {
         ports[n++] = port;
      }
      
      return ports;
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
