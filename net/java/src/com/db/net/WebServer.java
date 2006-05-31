/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A generic web server. This server communicates over ports.
 * 
 * @author Dave Longley
 */
public class WebServer
{
   /**
    * The ports for this web server in the order that they were added.
    */
   protected Vector mPorts;
   
   /**
    * A table mapping ports to web connection handlers.
    */
   protected HashMap mPortToWebConnectionHandler;
   
   /**
    * Whether or not this web server is running.
    */
   protected boolean mRunning;
   
   /**
    * Creates a new web server.
    */
   public WebServer()
   {
      // create list for ports
      mPorts = new Vector();
      
      // create the port to handler map
      mPortToWebConnectionHandler = new HashMap();
      
      mRunning = false;
   }
   
   /**
    * Disposes the web server. Stops the web server.
    */
   protected void finalize()
   {
      stop();
   }   
   
   /**
    * Maps a port to a web connection handler.
    * 
    * @param port the port to map to a web connection handler.
    * @param wch the web connection handler.
    */
   protected void mapPortToWebConnectionHandler(int port,
                                                WebConnectionHandler wch)
   {
      mPortToWebConnectionHandler.put("" + port, wch);
   }
   
   /**
    * Adds a web connection handler to this web server. Only one web
    * connection handler is permitted per port.
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
      boolean rval = false;
      
      // see if there is already a web connection handler on the provided port
      WebConnectionHandler existingHandler = getWebConnectionHandler(port);
      if(existingHandler == null)
      {
         // add port to the port list
         mPorts.add("" + port);
         
         // so assign the port to the handler
         mapPortToWebConnectionHandler(port, wch);
         
         // if the web server has already started, then start accepting
         // connections on the port
         if(isRunning())
         {
            // start accepting web connections on the port
            wch.startAcceptingWebConnections(port);
         }
         
         // the handler was added
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Removes a web connection handler from this web server. If the web
    * server is already running, then it will stop accepting connections
    * on all ports and its connections be terminated.
    * 
    * @param wch the web connection handler to remove from this server.
    */
   public synchronized void removeWebConnectionHandler(
      WebConnectionHandler wch)
   {
      // remove the web connection handler from all ports
      Iterator i = mPorts.iterator();
      while(i.hasNext())
      {
         int port = Integer.parseInt((String)i.next());
         WebConnectionHandler handler = getWebConnectionHandler(port);
         if(handler == wch)
         {
            // stop accepting web connections
            handler.stopAcceptingWebConnections();
            
            // terminate existing web connections
            handler.terminateWebConnections();
            
            // remove the port from the port list
            mPorts.remove("" + port);

            // remove the handler from the map
            i.remove();
         }
      }
   }
   
   /**
    * Removes all web connection handlers.
    */
   public synchronized void removeAllWebConnectionHandlers()
   {
      Iterator i = mPortToWebConnectionHandler.values().iterator();
      while(i.hasNext())
      {
         WebConnectionHandler handler = (WebConnectionHandler)i.next();

         // stop accepting web connections
         handler.stopAcceptingWebConnections();
            
         // terminate existing web connections
         handler.terminateWebConnections();
      }
      
      // clear port list
      mPorts.clear();
      
      // clear map
      mPortToWebConnectionHandler.clear();
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
         getLogger().debug("starting web server...");

         // start accepting connections on all web connection handlers
         Iterator i = mPorts.iterator();
         while(i.hasNext())
         {
            int port = Integer.parseInt((String)i.next());
            WebConnectionHandler handler = getWebConnectionHandler(port);
            
            // start accepting web connections
            handler.startAcceptingWebConnections(port);
         }
         
         // server is running
         mRunning = true;
      }
         
      getLogger().debug("web server started.");
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
         getLogger().debug("stopping web server...");
         
         // stop and terminate all connections on all web connection handlers
         Iterator i = mPorts.iterator();
         while(i.hasNext())
         {
            int port = Integer.parseInt((String)i.next());
            WebConnectionHandler handler = getWebConnectionHandler(port);
            
            // stop accepting web connections
            handler.stopAcceptingWebConnections();
            
            // terminate web connections
            handler.terminateWebConnections();
         }
         
         // server is not running
         mRunning = false;
      }
      
      getLogger().debug("web server stopped.");
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
    * @return the web connection handler for the specified port or null
    *         if no web connection handler is assigned to the port.
    */
   public WebConnectionHandler getWebConnectionHandler(int port)
   {
      return (WebConnectionHandler)mPortToWebConnectionHandler.get("" + port);
   }
   
   /**
    * Gets a list of all of the web connection handlers for this 
    * web server in the order that they were added to this web server.
    * 
    * @return a list of all of the web connection handlers for this
    *         web server in the order that they were added to this web server.
    */
   public Collection getWebConnectionHandlers()
   {
      Vector vector = new Vector();
      
      // add all web connection handlers according to port order
      Iterator i = mPorts.iterator();
      while(i.hasNext())
      {
         int port = Integer.parseInt((String)i.next());
         WebConnectionHandler handler = getWebConnectionHandler(port);
         
         // add handler if it isn't already in the list
         if(!vector.contains(handler))
         {
            vector.add(handler);
         }
      }
      
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
      Iterator i = mPorts.iterator();
      while(i.hasNext())
      {
         try
         {
            int port = Integer.parseInt((String)i.next());
            ports[n++] = port;
         }
         catch(Throwable ignore)
         {
         }
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
