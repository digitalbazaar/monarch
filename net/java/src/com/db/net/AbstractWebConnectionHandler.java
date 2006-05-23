/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.net.ServerSocket;
import java.util.HashMap;
import java.util.Iterator;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * This class provides the basic functionality for a web connection handler.
 * It provides management for listening and accepting traffic on several ports.
 * 
 * @author Dave Longley
 */
public abstract class AbstractWebConnectionHandler
implements WebConnectionHandler, WebConnectionAcceptedListener,
           WebConnectionServicer
{
   /**
    * The web connection server used by this web connection handler to
    * service web connections.
    */
   protected WebConnectionServicer mWebConnectionServicer;

   /**
    * A table mapping ports to server sockets. 
    */
   protected HashMap mPortToServerSocketMap;
   
   /**
    * A table mapping server sockets to web connection acceptors.
    */
   protected HashMap mServerSocketToWebConnectionAcceptorMap;
   
   /**
    * A table mapping web connection service threads to their
    * web connections.
    */
   protected HashMap mWebConnectionServiceThreadToWebConnection;
   
   /**
    * Creates a AbstractWebConnectionHandler.
    * 
    * @param wcs the web connection servicer used by this web connection handler
    *            to service web connections.
    */
   public AbstractWebConnectionHandler(WebConnectionServicer wcs)
   {
      // store web connection server
      mWebConnectionServicer = wcs;
      
      // create the port to server socket map
      mPortToServerSocketMap = new HashMap();
      
      // create the server socket to web connection acceptor map
      mServerSocketToWebConnectionAcceptorMap = new HashMap();
      
      // create web connection service thread to web connection
      mWebConnectionServiceThreadToWebConnection = new HashMap();
   }
   
   /**
    * Creates a new server socket for listening on a port. 
    * 
    * @param port the port the server socket will listen on.
    * @return the new server socket.
    */
   protected abstract ServerSocket createServerSocket(int port);
   
   /**
    * Creates a new web connection acceptor for accepting web connections. 
    * 
    * @return the new web connection acceptor.
    */
   protected WebConnectionAcceptor createWebConnectionAcceptor()
   {
      // create generic web connection acceptor
      WebConnectionAcceptor wca = new WebConnectionAcceptor();
      return wca;
   }
   
   /**
    * Maps a port to a server socket.
    * 
    * @param port the port to assign the server socket to.
    * @param serverSocket the server socket to assign to a port.
    */
   protected synchronized void mapPortToServerSocket(int port,
                                                     ServerSocket serverSocket)
   {
      mPortToServerSocketMap.put("" + port, serverSocket);
   }
   
   /**
    * Unmaps a port from a server socket.
    * 
    * @param port the port to unmap.
    */
   protected synchronized void unmapPortFromServerSocket(int port)
   {
      mPortToServerSocketMap.remove("" + port);
   }   

   /**
    * Gets the server socket that is listening on the specified port.
    * 
    * @param port the port to get the server socket for.
    * @return the server socket that is listening on the specified port,
    *         or null if no socket is listening on the port.
    */
   protected ServerSocket getServerSocket(int port)
   {
      return (ServerSocket)mPortToServerSocketMap.get("" + port);
   }
   
   /**
    * Maps a server socket to a web connection acceptor.
    * 
    * @param serverSocket the server socket to assign the web connection
    *                     acceptor to.
    * @param wca the web connection acceptor to assign to the server socket.
    */
   protected synchronized void mapServerSocketToWebConnectionAcceptor(
      ServerSocket serverSocket, WebConnectionAcceptor wca)
   {
      mServerSocketToWebConnectionAcceptorMap.put(serverSocket, wca);
   }

   /**
    * Unmaps a server socket from a web connection acceptor.
    * 
    * @param serverSocket the server socket to unmap.
    */
   protected synchronized void unmapServerSocketFromWebConnectionAcceptor(
      ServerSocket serverSocket)
   {
      mServerSocketToWebConnectionAcceptorMap.remove(serverSocket);
   }   
   
   /**
    * Gets the web connection acceptor for a particular server socket.
    * 
    * @param serverSocket the server socket to get the web connection acceptor
    *                     for.
    * @return the web connection acceptor for the given server socket or null
    *         if one has not yet been assigned.
    */
   protected WebConnectionAcceptor getWebConnectionAcceptor(
         ServerSocket serverSocket)
   {
      return (WebConnectionAcceptor)mServerSocketToWebConnectionAcceptorMap.
             get(serverSocket);
   }
   
   /**
    * Maps a web connection service thread to a web connection.
    * 
    * @param thread the web connection service thread.
    * @param webConnection the web connection.
    */
   protected synchronized void mapWebConnectionServiceThreadToWebConnection(
      Thread thread, WebConnection webConnection)
   {
      mWebConnectionServiceThreadToWebConnection.put(thread, webConnection);
   }
   
   /**
    * Unmaps a web connection service thread from a web connection.
    * 
    * @param thread the web connection service thread to unmap.
    */
   protected synchronized void unmapWebConnectionServiceThreadFromWebConnection(
      Thread thread)
   {
      mWebConnectionServiceThreadToWebConnection.remove(thread);
   }   
   
   /**
    * Gets a web connection for a particular web connection service thread.
    * 
    * @param thread the web connection service thread.
    * @return the web connection for the web connection service thread.
    */
   protected WebConnection getWebConnection(Thread thread)
   {
      return (WebConnection)mWebConnectionServiceThreadToWebConnection.
             get(thread);
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
      // add service thread to map
      Thread thread = Thread.currentThread();
      mapWebConnectionServiceThreadToWebConnection(thread, webConnection);

      try
      {
         getLogger().debug(getClass().getName() + " launching " +
                           "web connection servicer,ip=" +
                           webConnection.getRemoteIP());
         
         // service web connection
         mWebConnectionServicer.serviceWebConnection(webConnection);
         
         getLogger().debug(getClass().getName() + " finished " +
                           "servicing web connection,ip=" +
                           webConnection.getRemoteIP());
      }
      catch(Throwable t)
      {
         getLogger().debug(Logger.getStackTrace(t));
      }
      
      // disconnect web connection if it isn't already disconnected
      webConnection.disconnect();
      
      // remove service thread from list if thread wasn't interrupted
      unmapWebConnectionServiceThreadFromWebConnection(thread);
   }
   
   /**
    * Called when a web connection is accepted.
    * 
    * @param webConnection the accepted web connection.
    */
   public void webConnectionAccepted(WebConnection webConnection)
   {
      // service the web connection
      serviceWebConnection(webConnection);
   }
   
   /**
    * Begins accepting web connections on the given port.
    * 
    * @param port the port to start accepting web connections on.
    */
   public synchronized void startAcceptingWebConnections(int port)
   {
      // determine if this handler is already accepting connections
      // on the specified port
      ServerSocket serverSocket = getServerSocket(port);
      if(serverSocket == null)
      {
         // there is no server socket accepting connections on the
         // specified port, so create one
         serverSocket = createServerSocket(port);
         
         // map the port to the server socket
         mapPortToServerSocket(port, serverSocket);
      }
      
      if(serverSocket != null)
      {
         // get the web connection acceptor for the server socket
         WebConnectionAcceptor wca = getWebConnectionAcceptor(serverSocket);
         if(wca == null)
         {
            // create a web connection acceptor
            wca = createWebConnectionAcceptor();

            // add web connection accepted listener to delegate
            wca.getWebConnectionAcceptedDelegate().
            addWebConnectionAcceptedListener(this);
            
            // assign the server socket to the web connection acceptor
            mapServerSocketToWebConnectionAcceptor(serverSocket, wca);
         }
         
         // start accepting connections
         wca.startAcceptingWebConnections(serverSocket, webConnectionsSecure());
         
         getLogger().debug(getClass().getName() + " accepting " +
                           "web connections on port " + port + ".");
      }
      else
      {
         getLogger().error("could not create server socket to " +
                           "accept web connections on the specified port," +
                           "port=" + port);
      }
   }
   
   /**
    * Stops accepting all web connections. 
    */
   public synchronized void stopAcceptingWebConnections()
   {
      // get all of the web connection acceptors and stop accepting connections
      Iterator wcai = mServerSocketToWebConnectionAcceptorMap.
                      values().iterator();
      while(wcai.hasNext())
      {
         WebConnectionAcceptor wca = (WebConnectionAcceptor)wcai.next();
         wca.stopAcceptingWebConnections();
      }
      
      // clear the server socket to web connection acceptor map
      mServerSocketToWebConnectionAcceptorMap.clear();
      
      // close all server sockets
      Iterator ssi = mPortToServerSocketMap.values().iterator();
      while(ssi.hasNext())
      {
         ServerSocket serverSocket = (ServerSocket)ssi.next();
         
         try
         {
            serverSocket.close();
         }
         catch(Throwable t)
         {
            getLogger().debug(Logger.getStackTrace(t));
         }
      }
      
      // clear the port to server socket map
      mPortToServerSocketMap.clear();
      
      getLogger().debug(getClass().getName() + " no longer accepting " +
                        "web connections.");
   }
   
   /**
    * Stops accepting all web connections on the specified port. 
    * 
    * @param port the port to stop accepting web connections on.
    */
   public synchronized void stopAcceptingWebConnections(int port)
   {
      // get the server socket for the specified port
      ServerSocket serverSocket = getServerSocket(port);
      if(serverSocket != null)
      {
         // get the web connection acceptor for this server socket
         WebConnectionAcceptor wca = getWebConnectionAcceptor(serverSocket);
         if(wca != null)
         {
            // stop accepting connections
            wca.stopAcceptingWebConnections();
         }
         
         // remove the server socket from the web connection acceptor map
         unmapServerSocketFromWebConnectionAcceptor(serverSocket);
      }
      
      // disconnect the server socket
      try
      {
         serverSocket.close();
      }
      catch(Throwable t)
      {
         getLogger().debug(Logger.getStackTrace(t));
      }

      // remove the port from the server socket map
      unmapPortFromServerSocket(port);
      
      getLogger().debug(getClass().getName() + " no longer accepting " +
                        "web connections on port " + port + ".");
   }
   
   /**
    * Accepts a single web connection on the specified port.
    * 
    * @param port the port to accept the web connection on.
    */
   public void acceptWebConnection(int port)
   {
      // get the server socket for the specified port
      ServerSocket serverSocket = getServerSocket(port);
      if(serverSocket == null)
      {
         // server socket is null, so create one
         serverSocket = createServerSocket(port);
      }
      
      if(serverSocket != null)
      {
         // get the web connection acceptor for this server socket
         WebConnectionAcceptor wca = getWebConnectionAcceptor(serverSocket);
         if(wca == null)
         {
            // create a web connection acceptor
            wca = createWebConnectionAcceptor();
         }
         
         // accept web connection
         wca.acceptWebConnection(serverSocket, webConnectionsSecure());
      }
   }

   /**
    * Accepts a single proxy web connection on the specified port.
    * 
    * @param port the port to accept the web connection on.
    * @param originalWebConnection the original web connection to proxy.
    */
   public void acceptProxyWebConnection(int port,
                                        WebConnection originalWebConnection)
   {
      // get the server socket for the specified port
      ServerSocket serverSocket = getServerSocket(port);
      if(serverSocket == null)
      {
         // server socket is null, so create one
         serverSocket = createServerSocket(port);
      }
      
      if(serverSocket != null)
      {
         // get the web connection acceptor for this server socket
         WebConnectionAcceptor wca = getWebConnectionAcceptor(serverSocket);
         if(wca == null)
         {
            // create a web connection acceptor
            wca = createWebConnectionAcceptor();
         }
         
         // accept web proxy connection
         wca.acceptWebProxyConnection(serverSocket, originalWebConnection,
                                      webConnectionsSecure());
      }
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
      boolean rval = false;
      
      // if there are any web connection acceptors, then this
      // web connection handler is accepting connections
      if(mServerSocketToWebConnectionAcceptorMap.size() > 0)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns true if this web connection handler is accepting web connections
    * on the specified port, false if it is not.
    * 
    * @param port the port to check for web connections being accepted.
    */
   public synchronized boolean isAcceptingWebConnections(int port)
   {
      boolean rval = false;
      
      // get the server socket for the port
      ServerSocket serverSocket = getServerSocket(port);
      if(serverSocket != null)
      {
         // get the web connection acceptor for the server socket
         WebConnectionAcceptor wca = getWebConnectionAcceptor(serverSocket);
         if(wca != null)
         {
            // return whether or not the web connection acceptor is accepting
            // connections
            rval = wca.isAcceptingWebConnections();
         }
      }
      
      return rval;      
   }
   
   /**
    * Terminates all web connections this web connection handler is handling.
    */
   public synchronized void terminateWebConnections()
   {
      getLogger().debug(getClass().getName() + " terminating " +
                        "all web connections...");
      
      // interrupt all web connection service threads
      Iterator i = mWebConnectionServiceThreadToWebConnection.
                   keySet().iterator();
      while(i.hasNext())
      {
         Thread thread = (Thread)i.next();
         
         WebConnection webConnection = getWebConnection(thread);
         getLogger().debug(getClass().getName() + " terminating " +
                           "web connection,ip=" + webConnection.getRemoteIP());

         // interrupt thread
         thread.interrupt();
      }
      
      // join all web connection service threads
      i = mWebConnectionServiceThreadToWebConnection.keySet().iterator();
      while(i.hasNext())
      {
         Thread thread = (Thread)i.next();
         
         try
         {
            // attempt to join thread
            thread.join(100);
         }
         catch(Throwable t)
         {
            getLogger().debug(Logger.getStackTrace(t));
         }
      }
      
      getLogger().debug(getClass().getName() + " all web connections " +
                        "terminated.");
   }
   
   /**
    * Returns the number of web connections this web connection handler is
    * currently servicing.
    * 
    * @return the number of web connections currently being serviced.
    */
   public synchronized int webConnectionsBeingServiced()
   {
      // return the number of web connection service threads
      return mWebConnectionServiceThreadToWebConnection.size();
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbcommon");
   }
}
