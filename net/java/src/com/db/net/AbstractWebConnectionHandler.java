/*
 * Copyright (c) 2005-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.net.InetAddress;
import java.net.ServerSocket;
import java.util.LinkedList;
import java.util.List;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * This class provides the basic functionality for a web connection handler.
 * It provides management for listening and accepting traffic on several ports.
 * 
 * @author Dave Longley
 */
public abstract class AbstractWebConnectionHandler
implements WebConnectionHandler, WebConnectionServicer
{
   /**
    * The local bind address for this web connection handler.
    */
   protected InetAddress mBindAddress;
   
   /**
    * The port for this web connection handler.
    */
   protected int mPort;
   
   /**
    * The server socket for this web connection handler.
    */
   protected ServerSocket mServerSocket;
   
   /**
    * The web connection acceptor for this connection handler.
    */
   protected WebConnectionAcceptor mWebConnectionAcceptor;
   
   /**
    * The web connection server used by this web connection handler to
    * service web connections.
    */
   protected WebConnectionServicer mWebConnectionServicer;
   
   /**
    * A list of all of the current service threads.
    */
   protected List<WebConnectionServiceThread> mServiceThreads;
   
   /**
    * The web connection security manager for this web connection servicer.
    */
   protected WebConnectionSecurityManager mWebConnectionSecurityManager;
   
   /**
    * The maximum number of concurrent connections for this web connection
    * handler. 
    */
   protected int mMaxConcurrentConnections;
   
   /**
    * Creates a AbstractWebConnectionHandler.
    * 
    * @param servicer the web connection servicer used by this web connection
    *                 handler to service web connections.
    */
   public AbstractWebConnectionHandler(WebConnectionServicer servicer)
   {
      // store web connection servicer
      mWebConnectionServicer = servicer;
      
      // create service thread list
      mServiceThreads = new LinkedList<WebConnectionServiceThread>();

      // set default values
      mBindAddress = null;
      mPort = -1;
      mServerSocket = null;
      mMaxConcurrentConnections = 0;
   }
   
   /**
    * Creates a new server socket for listening on a port. 
    * 
    * @param bindAddress the local address to bind to (null indicates 0.0.0.0).
    * @param port the port the server socket will listen on.
    * 
    * @return the new server socket.
    */
   protected abstract ServerSocket createServerSocket(
      InetAddress bindAddress, int port);
   
   /**
    * Creates a new web connection acceptor for accepting web connections. 
    * 
    * @return the new web connection acceptor.
    */
   protected WebConnectionAcceptor createWebConnectionAcceptor()
   {
      // create generic web connection acceptor
      WebConnectionAcceptor wca = new WebConnectionAcceptor(
         this, getMaxConcurrentConnections());
      
      return wca;
   }
   
   /**
    * Gets the server socket for this web connection handler.
    * 
    * @return the server socket for this web connection handler or null if
    *         no server socket is set yet.
    */
   protected ServerSocket getServerSocket()
   {
      return mServerSocket;
   }
   
   /**
    * Gets the web connection acceptor for this web connection handler. One
    * will be created if one does not exist already.
    * 
    * @return the web connection acceptor for this web connection handler.
    */
   protected WebConnectionAcceptor getWebConnectionAcceptor()
   {
      if(mWebConnectionAcceptor == null)
      {
         mWebConnectionAcceptor = createWebConnectionAcceptor();
      }
      
      return mWebConnectionAcceptor;
   }
   
   /**
    * Adds a service thread to the list of service threads.
    * 
    * @param thread the web connection service thread to add.
    */
   protected void addServiceThread(WebConnectionServiceThread thread)
   {
      mServiceThreads.add(thread);
   }
   
   /**
    * Removes a service thread from the list of service threads.
    * 
    * @param thread the service thread to remove.
    */
   protected void removeServiceThread(WebConnectionServiceThread thread)
   {
      mServiceThreads.remove(thread);
   }   
   
   /**
    * Services a web connection.
    * 
    * The web connection should be serviced and disconnected when the
    * servicing is completed.
    *  
    * @param webConnection the web connection to be serviced.
    */   
   public void serviceWebConnection(WebConnection webConnection)
   {
      // create web connection service thread
      WebConnectionServiceThread thread = new WebConnectionServiceThread(
         Thread.currentThread(), webConnection);

      // add service thread
      addServiceThread(thread);

      try
      {
         getLogger().debug(getClass(),
            "launching web connection servicer,ip=" +
            webConnection.getRemoteIP());
         
         // service web connection
         mWebConnectionServicer.serviceWebConnection(webConnection);
         
         getLogger().debug(getClass(),
            "finished servicing web connection,ip=" +
            webConnection.getRemoteIP());
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      // disconnect web connection if it isn't already disconnected
      webConnection.disconnect();
      
      // remove service thread
      removeServiceThread(thread);
   }
   
   /**
    * Starts accepting web connections on the given local address and port.
    * If this web connection handler is already listening on another port,
    * this method should return false. If it is already listening on the
    * passed port, it should have no effect and return true.
    * 
    * @param bindAddress the local address to bind to (null indicates 0.0.0.0).
    * @param port the port to start accepting web connections on.
    * 
    * @return true if this web connection handler is now listening on the
    *         specified port, false if not.
    */
   public synchronized boolean startAcceptingWebConnections(
      InetAddress bindAddress, int port)
   {
      boolean rval = false;
      
      // determine if this handler is already accepting connections
      if(isAcceptingWebConnections())
      {
         if(getPort() == port)
         {
            // this handler is already accepting connections on the specified
            // port, so return true
            rval = true;
         }
         else
         {
            // this handler is accepting connections on another port, so
            // return false (the default return value)
         }
      }
      else
      {
         // create a new server socket
         mServerSocket = createServerSocket(bindAddress, port);
         
         // ensure the server socket was created successfully
         if(mServerSocket != null)
         {
            // set the local bind address for this handler
            mBindAddress = mServerSocket.getInetAddress();
            
            // set the local port for this handler
            mPort = mServerSocket.getLocalPort();
            
            // start accepting connections
            getWebConnectionAcceptor().startAcceptingWebConnections(
               mServerSocket, webConnectionsSecure());
            
            // now accepting web connections
            rval = true;
            
            getLogger().debug(getClass(),
               "accepting web connections on port " + getPort() + ".");
         }
         else
         {
            getLogger().error(getClass(),
               "could not create server socket to accept web connections " +
               "on the specified port,port=" + port);
         }
      }
      
      return rval;
   }
   
   /**
    * Stops accepting all web connections. 
    */
   public synchronized void stopAcceptingWebConnections()
   {
      // determine if this handler is accepting connections
      if(isAcceptingWebConnections())
      {
         // get the web connection acceptor, and stop accepting web connections
         getWebConnectionAcceptor().stopAcceptingWebConnections();
         
         try
         {
            // close the server socket
            getServerSocket().close();
         }
         catch(Throwable t)
         {
            getLogger().debug(getClass(), Logger.getStackTrace(t));
         }
         
         getLogger().debug(getClass(),
            "no longer accepting web connections on port " + getPort() + ".");
         
         // reset server socket, bind address, port
         mBindAddress = null;
         mPort = -1;
         mServerSocket = null;
      }
   }
   
   /**
    * Accepts a single proxy web connection. If this web connection handler
    * is not accepting connections, then this method should return false.
    * 
    * @param originalWebConnection the original web connection to proxy.
    * 
    * @return true if the proxy web connection was accepted, false if not.
    */
   public boolean acceptProxyWebConnection(WebConnection originalWebConnection)   
   {
      boolean rval = false;
      
      if(isAcceptingWebConnections())
      {
         // accept web proxy connection
         getWebConnectionAcceptor().acceptWebProxyConnection(
            getServerSocket(), originalWebConnection, webConnectionsSecure());
         
         // connection accepted
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Indicates whether or not connections handled by this web
    * connection handler should be considered secure.
    * 
    * @return true if the connections handled by this web connection
    *         handler should be considered secure, false if they should
    *         not be considered secure. 
    */
   public abstract boolean webConnectionsSecure();
   
   /**
    * Returns true if this web connection handler is accepting web connections,
    * false if it is not.
    */
   public synchronized boolean isAcceptingWebConnections()
   {
      // return whether or not the web connection acceptor is accepting
      return getWebConnectionAcceptor().isAcceptingWebConnections();
   }
   
   /**
    * Sets the maximum number of connections to handle concurrently.
    * 
    * If connections is 0, then there will be no maximum.
    *
    * @param connections the maximum number of concurrent connections allowed.
    */
   public synchronized void setMaxConcurrentConnections(int connections)
   {
      mMaxConcurrentConnections = Math.max(0, connections);
      getWebConnectionAcceptor().setMaxConcurrentConnections(
         mMaxConcurrentConnections);
   }
   
   /**
    * Gets the maximum number of connections to handle concurrently.
    * 
    * If connections is 0, then there is no maximum.
    *
    * @return the maximum number of connections to handle concurrently.
    */
   public synchronized int getMaxConcurrentConnections()
   {
      return mMaxConcurrentConnections;
   }
   
   /**
    * Terminates all web connections this web connection handler is handling.
    * Tries to shutdown web connections gracefully.
    */
   public void terminateWebConnections()
   {
      getLogger().debug(getClass(), "terminating all web connections...");
      
      // lock on the service thread list
      synchronized(mServiceThreads)
      {
         // interrupt all web connection service threads
         for(WebConnectionServiceThread thread: mServiceThreads)
         {
            WebConnection webConnection = thread.getWebConnection();
            getLogger().debug(getClass(),
               "terminating web connection,ip=" + webConnection.getRemoteIP());

            // interrupt thread
            thread.interrupt();
         }
      }
      
      // terminate all web connections
      if(getWebConnectionAcceptor() != null)
      {
         getWebConnectionAcceptor().terminateAllWebConnections();
      }
      
      getLogger().debug(getClass(), "all web connections terminated.");
   }
   
   /**
    * Disconnects all web connections this web connection handler is
    * handling immediately.
    */
   public void disconnectWebConnections()
   {
      getLogger().debug(getClass(), "disconnecting all web connections...");
      
      // lock on the service thread list
      synchronized(mServiceThreads)
      {
         // interrupt all web connection service threads
         for(WebConnectionServiceThread thread: mServiceThreads)
         {
            WebConnection webConnection = thread.getWebConnection();
            getLogger().debug(getClass(),
               "forcefully disconnecting web connection,ip=" +
               webConnection.getRemoteIP());
            
            // interrupt thread
            thread.interrupt();

            // disconnect web connection
            webConnection.disconnect();
         }
      }
      
      // terminate all web connections
      if(getWebConnectionAcceptor() != null)
      {
         getWebConnectionAcceptor().terminateAllWebConnections();
      }
      
      getLogger().debug(getClass(), "all web connections disconnected.");  
   }
   
   /**
    * Returns the number of web connections this web connection handler is
    * currently servicing.
    * 
    * @return the number of web connections currently being serviced.
    */
   public int webConnectionsBeingServiced()
   {
      // return the number of web connection service threads
      return mServiceThreads.size();
   }
   
   /**
    * Gets the local bind address that this web connection handler is
    * accepting web connections on.
    * 
    * @return the local bind address that this web connection handler is
    *         accepting web connections on (null indicates the web connection
    *         handler is not yet bound to an address).
    */
   public InetAddress getBindAddress()   
   {
      return mBindAddress;
   }
   
   /**
    * Gets the port that this web connection handler is accepting web
    * connections on.
    * 
    * @return the port that this web connection handler is accepting web
    *         connections on, or -1 if it is not accepting connections.
    */
   public int getPort()
   {
      return mPort;
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
   
   /**
    * A WebConnectionServiceThread is a wrapper for a thread that is servicing
    * a web connection.
    * 
    * @author Dave Longley
    */
   public class WebConnectionServiceThread
   {
      /**
       * The thread servicing the web connection.
       */
      protected Thread mThread;
      
      /**
       * The web connection being serviced.
       */
      protected WebConnection mWebConnection;
      
      /**
       * Creates a new WebConnectionServiceThread that wraps the passed thread
       * that is servicing the passed web connection.
       * 
       * @param thread the thread to wrap.
       * @param webConnection the web connection being serviced. 
       */
      public WebConnectionServiceThread(
         Thread thread, WebConnection webConnection)
      {
         // store thread and web connection
         mThread = thread;
         mWebConnection = webConnection;
      }
      
      /**
       * Interrupts this web connection service thread.
       */
      public void interrupt()
      {
         mThread.interrupt();
      }
      
      /**
       * Gets the web connection being serviced.
       * 
       * @return the web connection being serviced.
       */
      public WebConnection getWebConnection()
      {
         return mWebConnection;
      }
   }
}
