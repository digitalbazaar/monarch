/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.datagram;

import java.net.InetAddress;
import java.util.Collection;
import java.util.HashMap; 
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A Datagram Server is a server that listens for datagrams and responds
 * with them. 
 * 
 * @author Dave Longley
 */
public class DatagramServer
{
   /**
    * The ports for this server in the order that they were added.
    */
   protected List<Integer> mPorts;
   
   /**
    * A table mapping ports to datagram handlers.
    */
   protected HashMap<Integer, DatagramHandler> mPortToDatagramHandler;
   
   /**
    * A table mapping datagram handlers to bind addresses.
    */
   protected HashMap<DatagramHandler, InetAddress>
      mDatagramHandlerToBindAddress;
   
   /**
    * A list of datagramm handlers that are assigned to use any free ephemeral
    * port -- so that they don't have actual ports set yet until they start
    * accepting datagrams. 
    */
   protected List<DatagramHandler> mUnassignedDatagramHandlers;

   /**
    * Whether or not this server is running.
    */
   protected boolean mRunning;
   
   /**
    * Creates a new DatagramServer.
    */
   public DatagramServer()
   {
      // create list for ports
      mPorts = new LinkedList<Integer>();
      
      // create the port to handler map
      mPortToDatagramHandler = new HashMap<Integer, DatagramHandler>();
      
      // create handler to bind address map
      mDatagramHandlerToBindAddress =
         new HashMap<DatagramHandler, InetAddress>();
      
      // create vector for unassigned datagram handlers
      mUnassignedDatagramHandlers = new LinkedList<DatagramHandler>();
      
      // server not running by default
      mRunning = false;
   }
   
   /**
    * Disposes this DatagramServer. Stops this DatagramServer.
    */
   @Override
   protected void finalize()
   {
      stop();
   }
   
   /**
    * Adds a datagram handler to this server. The handler will bind to
    * 0.0.0.0. Only one datagram handler is permitted per port.
    * 
    * A datagram handler will fail to be added to listen on the given port
    * if there already exists another datagram handler listening on that port.

    * If the server is already running, then the datagram handler will begin
    * accepting datagrams on the specified port.
    * 
    * @param dh the datagram handler to add to this server.
    * @param port the port the datagram handler will listen on.
    * 
    * @return true if the datagram handler was successfully added, false if
    *         it was not.
    */
   public synchronized boolean addDatagramHandler(DatagramHandler dh, int port)   
   {
      return addDatagramHandler(dh, null, port);
   }
   
   /**
    * Adds a datagram handler to this server that will bind to a specific
    * local address (null indicates 0.0.0.0). Only one datagram handler is
    * permitted per port.
    * 
    * A datagram handler will fail to be added to listen on the given port
    * if there already exists another datagram handler listening on that port.
    * 
    * If the server is already running, then the datagram handler will begin
    * accepting datagrams on the specified port.
    * 
    * @param dh the datagram handler to add to this server.
    * @param bindAddress the local address to bind to (null indicates 0.0.0.0).
    * @param port the port the datagram handler will listen on.
    * 
    * @return true if the datagram handler was successfully added, false if
    *         it was not.
    */
   public synchronized boolean addDatagramHandler(
      DatagramHandler dh, InetAddress bindAddress, int port)
   {
      boolean rval = false;
      
      // see if there is already a datagram handler on the provided port
      DatagramHandler existingHandler = getDatagramHandler(port);
      if(existingHandler == null)
      {
         // if the server has already started, then start accepting datagrams
         // on the port
         if(isRunning())
         {
            // start accepting datagrams on the port
            dh.startAcceptingDatagrams(bindAddress, port);
         }
         
         if(dh.getPort() > 0 || port > 0)
         {
            // use the larger port number
            port = Math.max(dh.getPort(), port);
            
            // add port to the port list
            mPorts.add(port);
            
            // so assign port to handler and handler to bind address
            mPortToDatagramHandler.put(port, dh);
            mDatagramHandlerToBindAddress.put(dh, bindAddress);
            
            // the handler was added
            rval = true;
         }
         else if(!isRunning())
         {
            // add an unassigned datagram handler
            mUnassignedDatagramHandlers.add(dh);
            
            // assign handler to bind address
            mDatagramHandlerToBindAddress.put(dh, bindAddress);
            
            // the handler was added
            rval = true;
         }         
      }
      
      return rval;
   }
   
   /**
    * Removes a datagram handler from this server. If the server is already
    * running, then it will stop accepting datagrams.
    * 
    * @param dh the datagram handler to remove from this server.
    */
   public synchronized void removeDatagramHandler(DatagramHandler dh)
   {
      // see if the handler exists
      dh = getDatagramHandler(dh.getPort());
      if(dh != null)
      {
         // save port
         int port = dh.getPort();
         
         if(isRunning())
         {
            // stop accepting datagrams
            dh.stopAcceptingDatagrams();
            
            // stop datagram servicer
            dh.stopDatagramServicer();
         }
         
         // remove the port from the port list
         mPorts.remove(port);
         
         // unmap the datagram handler
         mPortToDatagramHandler.remove(port);
      }

      // remove the datagram handler from the unassigned list
      mUnassignedDatagramHandlers.remove(dh);
      
      // unmap the bind address
      mDatagramHandlerToBindAddress.remove(dh);
   }
   
   /**
    * Removes all datagram handlers.
    */
   public synchronized void removeAllDatagramHandlers()
   {
      for(DatagramHandler dh: mPortToDatagramHandler.values())
      {
         // stop accepting datagrams
         dh.stopAcceptingDatagrams();
         
         // stop datagram servicer
         dh.stopDatagramServicer();
      }
      
      // clear port list
      mPorts.clear();
      
      // clear maps
      mPortToDatagramHandler.clear();
      mDatagramHandlerToBindAddress.clear();
      
      // clear unassigned list
      mUnassignedDatagramHandlers.clear();
   }
   
   /**
    * Starts this server. All datagram handlers handlers will begin accepting
    * datagrams.
    */
   public synchronized void start()
   {
      // only start server if it isn't running
      if(!isRunning())
      {
         getLogger().debug(getClass(), "starting datagram server...");
         
         // start accepting datagrams on all assigned datagram handlers
         for(int port: mPorts)
         {
            // start accepting datagrams
            DatagramHandler handler = getDatagramHandler(port);
            handler.startAcceptingDatagrams(getBindAddress(handler), port);
         }
         
         // start accepting datagrams on all unassigned datagram handlers
         for(Iterator<DatagramHandler> i =
             mUnassignedDatagramHandlers.iterator(); i.hasNext();)
         {
            DatagramHandler handler = i.next();
            
            // start accepting datagrams on an ephemeral port
            handler.startAcceptingDatagrams(getBindAddress(handler), 0);
            
            // add port to the port list
            mPorts.add(handler.getPort());
            
            // so assign the port to the handler
            mPortToDatagramHandler.put(handler.getPort(), handler);
            
            // remove the handler from the unassigned list
            i.remove();
         }
         
         // server is running
         mRunning = true;
      }
         
      getLogger().debug(getClass(), "datagram server started.");
   }
   
   /**
    * Stops this datagram server. All datagram handlers will stop accepting
    * datagrams.
    */
   public synchronized void stop()
   {
      // only stop server if it is running
      if(isRunning())
      {
         getLogger().debug(getClass(), "stopping datagram server...");
         
         // stop all datagram handlers
         for(int port: mPorts)
         {
            DatagramHandler dh = getDatagramHandler(port);
            
            // stop accepting datagrams
            dh.stopAcceptingDatagrams();
            
            // stop datagram servicer
            dh.stopDatagramServicer();
         }
         
         // server is not running
         mRunning = false;
      }
      
      getLogger().debug(getClass(), "datagram server stopped.");
   }
   
   /**
    * Returns true if this datagram server is running, false if not.
    *
    * @return true if this datagram server is running, false if not.
    */
   public synchronized boolean isRunning()
   {
      return mRunning;
   }
   
   /**
    * Gets the datagram handler for the specified port.
    * 
    * @param port the port to get the datagram handler for.
    * 
    * @return the datagram handler for the specified port or null
    *         if no datagram handler is assigned to the port.
    */
   public DatagramHandler getDatagramHandler(int port)
   {
      return mPortToDatagramHandler.get(port);
   }

   /**
    * Gets the local bind address for the given datagram handler. This
    * method will retrieve the bind address for the handler even if the
    * handler is not accepting datagrams yet.
    * 
    * @param dh the datagram handler to get the local bind address for.
    * 
    * @return the local bind address the datagram handler uses/will use.
    */
   public InetAddress getBindAddress(DatagramHandler dh)
   {
      return mDatagramHandlerToBindAddress.get(dh);
   }
   
   /**
    * Gets a list of all of the datagram handlers for this datagram server
    * in the order that they were added to this datagram server.
    * 
    * @return a list of all of the datagram handlers for this datagram
    *         server in the order that they were added to this datagram server.
    */
   public Collection<DatagramHandler> getDatagramListeners()
   {
      List<DatagramHandler> list = new LinkedList<DatagramHandler>();
      
      // add all datagram handlers according to port order
      for(int port: mPorts)
      {
         // add datagram handler
         DatagramHandler dh = getDatagramHandler(port);
         list.add(dh);
      }
      
      // add all unassigned datagram handlers
      list.addAll(mUnassignedDatagramHandlers);
      
      return list;
   }
   
   /**
    * Gets a list of all of the ports for this datagram server in the order
    * that they were added to this datagram server.
    * 
    * @return a list of all of the ports for this datagram server in the order
    *         that they were added to this datagram server.
    */
   public int[] getDatagramListenerPorts()
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
    * Gets the logger for this server.
    * 
    * @return the logger for this server.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
