/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.JobThreadPool;
import com.db.util.MethodInvoker;

/**
 * This class accepts web connections with a particular server socket and
 * notifies listeners when a web connection has been accepted.
 * 
 * @author Dave Longley
 */
public class WebConnectionAcceptor
{
   /**
    * The thread used to accept connections.
    */
   protected Thread mAcceptorThread;
   
   /**
    * Whether or not this web connection acceptor is currently accepting
    * web connections.
    */
   protected boolean mAcceptingWebConnections;
   
   /**
    * A thread pool for handling accepted web connections.
    */
   protected JobThreadPool mAcceptedWebConnectionThreadPool; 
   
   /**
    * A web connection accepted delegate for reporting web connection
    * accepted messages.
    */
   protected WebConnectionAcceptedDelegate mWebConnectionAcceptedDelegate;
   
   /**
    * Creates a new WebConnectionAcceptor with the specified maximum number of
    * connections to accept at once.
    * 
    * @param connections the maximum number of connections to accept at once.
    */
   public WebConnectionAcceptor(int connections)
   {
      // acceptor thread is null until startAcceptingWebConnections() is called
      mAcceptorThread = null;
      
      // not accepting web connections yet
      mAcceptingWebConnections = false;
      
      // create the accepted web connection thread pool
      mAcceptedWebConnectionThreadPool = new JobThreadPool(connections);
      
      // set threads to expire automatically after they have been
      // idle for 5 minutes (300000 milliseconds)
      mAcceptedWebConnectionThreadPool.setJobThreadExpireTime(300000);
      
      // create web connection accepted delegate
      mWebConnectionAcceptedDelegate = new WebConnectionAcceptedDelegate();
   }
   
   /**
    * Gets the accepted web connection thread pool. 
    * 
    * @return the accepted web connection thread pool.
    */
   protected JobThreadPool getThreadPool()
   {
      return mAcceptedWebConnectionThreadPool;
   }

   /**
    * Accepts a web connection with the specified server socket. This
    * method is synchronized to ensure that only one connection is
    * accepted at a time.
    * 
    * This does not mean that multiple connections cannot be serviced at
    * the same time, only that they are accepted synchronously. This allows
    * for web connection acceptors to be locked when accepting connections
    * elsewhere -- which allows client IP addresses to be forwarded for proxy
    * connections, etc.
    * 
    * @param serverSocket the server socket to accept a connection with.
    * @param secure whether or not the connection should be considered secure.
    */
   public synchronized void acceptWebConnection(ServerSocket serverSocket,
                                                boolean secure)
   {
      // the worker socket that represents the accepted connection
      Socket workerSocket = null;
      
      try
      {
         serverSocket.setSoTimeout(10);
         
         // accept a connection
         try
         {
            workerSocket = serverSocket.accept();
         }
         catch(SocketTimeoutException ste)
         {
            // do nothing
         }
         
         // if a connection was accepted
         if(workerSocket != null)
         {
            // create a generic web connection
            WebConnection webConnection =
               new GenericWebConnection(workerSocket);
            webConnection.setSecure(secure);
            
            // handle the web connection on another thread
            AcceptedWebConnectionHandler handler =
               new AcceptedWebConnectionHandler(webConnection);
            getThreadPool().runJob(handler);
         }
      }
      catch(Throwable t)
      {
         getLogger().debug(Logger.getStackTrace(t));
      }
   }
   
   /**
    * Accepts a web connection with the specified server socket. This
    * method is synchronized to ensure that only one connection is
    * accepted at a time.
    * 
    * This does not mean that multiple connections cannot be serviced at
    * the same time, only that they are accepted synchronously. This allows
    * for web connection acceptors to be locked when accepting connections
    * elsewhere -- which allows client IP addresses to be forwarded for proxy
    * connections, etc.
    * 
    * @param serverSocket the server socket to accept a connection with.
    * @param originalWebConnection the original web connection to proxy.
    * @param secure whether or not the connection should be considered secure.
    */
   public synchronized void acceptWebProxyConnection(
      ServerSocket serverSocket, WebConnection originalWebConnection,
      boolean secure)
   {
      try
      {
         // provide plenty of time to connect
         serverSocket.setSoTimeout(30000);
         
         // create a new client socket
         Socket newClientSocket = new Socket(serverSocket.getInetAddress(),
                                             serverSocket.getLocalPort());
         
         // create the new web connection, a proxy connection will
         // automatically pipe data between this connection and the
         // original one
         WebConnection newWebConnection =
            new GenericWebConnection(newClientSocket);
         newWebConnection.setSecure(secure);
         
         // accept the connection, the accepting server will use this
         // worker socket to read/write to the proxy connection
         Socket workerSocket = serverSocket.accept();
         
         // create proxy web connection
         ProxyWebConnection proxyWebConnection =
            new ProxyWebConnection(workerSocket,
                                   originalWebConnection,
                                   newWebConnection);
         proxyWebConnection.setSecure(secure);
         
         // start the proxy
         proxyWebConnection.startProxy();
         
         // handle the web connection on another thread
         AcceptedWebConnectionHandler handler =
            new AcceptedWebConnectionHandler(proxyWebConnection);
         getThreadPool().runJob(handler);
      }
      catch(Throwable t)
      {
         getLogger().debug(Logger.getStackTrace(t));
      }
   }
   
   /**
    * Accepts web connections with the passed server socket.
    * 
    * @param serverSocket the server socket to accept connections with.
    * @param secure whether or not the connections should be considered
    *        secure.
    */
   public void acceptWebConnections(ServerSocket serverSocket, boolean secure)
   {
      getLogger().debug("WebConnectionAcceptor accepting web connections on " +
                        "port " + serverSocket.getLocalPort());
      
      // continue accepting connections until told otherwise or
      // if this thread is interrupted
      while(isAcceptingWebConnections() &&
            !Thread.currentThread().isInterrupted())
      {
         // accept a web connection
         acceptWebConnection(serverSocket, secure);
         
         try
         {
            // sleep to allow for other connections
            Thread.sleep(100);
         }
         catch(Throwable t)
         {
         }
      }
      
      getLogger().debug("WebConnectionAcceptor no longer accepting web " +
                        "connections on port " + serverSocket.getLocalPort());
   }
   
   /**
    * Starts accepting web connections with the specified server socket -- only
    * if this web connection acceptor isn't already accepting connections.
    * 
    * @param serverSocket the socket to start accepting connections with.
    * @param secure whether or not the connections should be considered
    *        secure.
    */
   public synchronized void startAcceptingWebConnections(
      ServerSocket serverSocket, boolean secure)
   {
      // only start accepting web connections if not already doing so
      if(!isAcceptingWebConnections())
      {
         // now accepting web connections
         mAcceptingWebConnections = true;
      
         // accept connections on another thread
         Object[] params = new Object[]{serverSocket, new Boolean(secure)};
         MethodInvoker mi =
            new MethodInvoker(this, "acceptWebConnections", params);
         mi.backgroundExecute();
         
         // set acceptor thread
         mAcceptorThread = mi;
      }
   }
   
   /**
    * Stops accepting web connections.
    */
   public synchronized void stopAcceptingWebConnections()
   {
      // stop accepting web connections
      mAcceptingWebConnections = false;
      
      // interrupt acceptor thread
      mAcceptorThread.interrupt();
      
      try
      {
         // join acceptor thread
         mAcceptorThread.join(1000);
      }
      catch(Throwable t)
      {
         getLogger().debug(Logger.getStackTrace(t));
      }
   }
   
   /**
    * Returns whether or not this web connection acceptor is currently
    * accepting web connections.
    * 
    * @return true if this web connection acceptor is accepting web connections,
    *         false if it is not.
    */
   public synchronized boolean isAcceptingWebConnections()
   {
      return mAcceptingWebConnections;      
   }
   
   /**
    * Terminates all current accepted web connections.
    */
   public void terminateAllWebConnections()
   {
      // terminate all threads in the pool
      getThreadPool().terminateAllThreads();
   }
   
   /**
    * Gets the web connection accepted delegate.
    * 
    * @return the web connection accepted delegate.
    */
   public WebConnectionAcceptedDelegate getWebConnectionAcceptedDelegate()
   {
      return mWebConnectionAcceptedDelegate;
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
    * This class is used to handle accepted web connections.
    * 
    * @author Dave Longley
    */
   public class AcceptedWebConnectionHandler implements Runnable
   {
      /**
       * The web connection to handle.
       */
      protected WebConnection mWebConnection;
      
      /**
       * Creates a new AcceptedWebConnectionHandler to handle the
       * passed accepted web connection.
       * 
       * @param webConnection the web connection handle.
       */
      public AcceptedWebConnectionHandler(WebConnection webConnection)
      {
         // store web connection
         mWebConnection = webConnection;
      }
      
      /**
       * Handles an accepted connection by firing a webConnectionAccepted
       * message -- if there are listeners for that message. Otherwise
       * the connection is shutdown.
       * 
       * @param webConnection the accepted web connection.
       */
      public void handleAcceptedWebConnection(WebConnection webConnection)
      {
         // true if a message was fired to handle the socket connection,
         // false if one was not
         boolean messageFired = false;
         
         // only fire message if acceptor thread is not interrupted
         if(!mAcceptorThread.isInterrupted())
         {
            String ip = webConnection.getRemoteIP();
            if(webConnection instanceof ProxyWebConnection)
            {
               getLogger().debug("proxy web connection accepted, ip=" + ip);
            }
            else
            {
               getLogger().debug("web connection accepted, ip=" + ip);
            }
         
            // fire message indicating that a web connection has been accepted
            if(getWebConnectionAcceptedDelegate().getListenerCount() > 0)
            {
               getWebConnectionAcceptedDelegate().
               fireWebConnectionAccepted(webConnection);
               messageFired = true;
            }
         }
         else
         {
            // web connection accepting has been interrupted, disconnect
            webConnection.disconnect();
         }
         
         // if a message was not fired to handle the connection,
         // then terminate it
         if(!messageFired)
         {
            getLogger().error("message was not fired to handle accepted " +
                              "web connection, terminating connection.");
            
            // disconnect web connection
            webConnection.disconnect();
         }
      }
      
      /**
       * Handles an accepted web connection.
       */
      public void run()
      {
         handleAcceptedWebConnection(mWebConnection);
      }
   }
}
