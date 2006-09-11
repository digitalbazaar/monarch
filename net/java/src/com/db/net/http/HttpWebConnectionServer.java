/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.http;

import com.db.net.GenericWebConnectionHandler;
import com.db.net.WebConnectionSecurityManager;
import com.db.net.WebConnectionServer;
import com.db.net.ssl.SSLWebConnectionHandler;

/**
 * An HttpWebConnectionServer is a web connection server that has generic
 * and ssl web connection handlers that service web connections using an http
 * web connection servicer.  
 * 
 * @author Dave Longley
 */
public class HttpWebConnectionServer extends WebConnectionServer
{
   /**
    * The http web connection servicer.
    */
   protected HttpWebConnectionServicer mHttpWebConnectionServicer;
   
   /**
    * The generic web connection handler.
    */
   protected GenericWebConnectionHandler mGenericWebConnectionHandler;
   
   /**
    * The ssl web connection handler.
    */
   protected SSLWebConnectionHandler mSSLWebConnectionHandler;
   
   /**
    * The non-secure http port for this server.
    */
   protected int mNonSecurePort;
   
   /**
    * The secure http port for this server.
    */
   protected int mSecurePort;
   
   /**
    * The default non-secure http port.
    */
   public static final int DEFAULT_NON_SECURE_HTTP_PORT = 80;
   
   /**
    * The default secure http port.
    */
   public static final int DEFAULT_SECURE_HTTP_PORT = 443;
   
   /**
    * Creates a new HttpWebConnectionServer.
    */
   public HttpWebConnectionServer()
   {
      // create http web connection servicer
      mHttpWebConnectionServicer = new HttpWebConnectionServicer();
      
      // create generic web connection handler
      mGenericWebConnectionHandler =
         new GenericWebConnectionHandler(mHttpWebConnectionServicer);
      
      // create ssl web connection handler
      mSSLWebConnectionHandler =
         new SSLWebConnectionHandler(mHttpWebConnectionServicer);
      
      // set default ports
      setNonSecurePort(DEFAULT_NON_SECURE_HTTP_PORT);
      setSecurePort(DEFAULT_SECURE_HTTP_PORT);
      
      // accept an infinite number of connections each by default
      setMaximumNonSecureConnections(0);
      setMaximumSecureConnections(0);
   }
   
   /**
    * Adds a non-secure http web request servicer to this server. If the path
    * specified does not begin and end with slashes ("/"), they will be
    * appended.
    * 
    * The path of the servicer will be set.
    * 
    * @param hwrs the http web request servicer to add.
    * @param path the path to the servicer.
    */
   public void addNonSecureHttpWebRequestServicer(
      HttpWebRequestServicer hwrs, String path)
   {
      mHttpWebConnectionServicer.addNonSecureHttpWebRequestServicer(hwrs, path);
   }
   
   /**
    * Adds a secure http web request servicer to this server. If the path
    * specified does not begin and end with slashes ("/"), they will be
    * appended.
    * 
    * The path of the servicer will be set.
    * 
    * @param hwrs the http request servicer to add.
    * @param path the path to the servicer.
    */
   public void addSecureHttpWebRequestServicer(
      HttpWebRequestServicer hwrs, String path)
   {
      mHttpWebConnectionServicer.addSecureHttpWebRequestServicer(hwrs, path);
   }
   
   /**
    * Removes a non-secure http web request servicer from this server.
    * 
    * @param path the path to the servicer.
    */
   public void removeNonSecureHttpWebRequestServicer(String path)
   {
      mHttpWebConnectionServicer.removeNonSecureHttpWebRequestServicer(path);
   }
   
   /**
    * Removes a secure http web request servicer from this server.
    * 
    * @param path the path to the servicer.
    */
   public void removeSecureHttpWebRequestServicer(String path)
   {
      mHttpWebConnectionServicer.removeSecureHttpWebRequestServicer(path);
   }
   
   /**
    * Starts this http web server using the default local ports
    * for non-secure and secure http traffic.
    */
   public synchronized void start()   
   {
      // if we aren't already running, start
      if(!isRunning())
      {
         // remove all web connection handlers
         removeAllWebConnectionHandlers();

         // add ssl web connection handler first
         if(getSecurePort() > 0)
         {
            // add ssl web connection handler for secure http port
            addWebConnectionHandler(mSSLWebConnectionHandler,
                                    getSecurePort());
         }
         
         // add generic web connection handler last 
         if(getNonSecurePort() > 0)
         {
            // add generic web connection handler for non-secure http port
            addWebConnectionHandler(mGenericWebConnectionHandler,
                                    getNonSecurePort());
         }
         
         // start server
         super.start();
      }
   }
   
   /**
    * Starts this http web server using the passed port. Only a non-secure
    * connection handler will be active (no SSL support).
    * 
    * @param nonSecurePort the port for non-secure http traffic.
    */
   public synchronized void startNonSecure(int nonSecurePort)
   {
      setNonSecurePort(nonSecurePort);
      setSecurePort(0);
      start();
   }
   
   /**
    * Starts this http web server using the passed port. Only a secure
    * connection handler will be active (no non-SSL support).
    * 
    * @param securePort the port for secure http traffic.
    */
   public synchronized void startSecure(int securePort)
   {
      setNonSecurePort(0);
      setSecurePort(securePort);
      start();
   }
   
   /**
    * Starts this http web server using the passed ports.
    * 
    * @param nonSecurePort the port for non-secure http traffic.
    * @param securePort the port for secure http traffic.
    */
   public synchronized void start(
      int nonSecurePort, int securePort)   
   {
      setNonSecurePort(nonSecurePort);
      setSecurePort(securePort);
      start();
   }
   
   /**
    * Sets the server name for the internal http web server.
    * 
    * @param serverName the server name for the internal http web server.
    */
   public void setServerName(String serverName)
   {
      mHttpWebConnectionServicer.setDefaultServerName(serverName);
   }
   
   /**
    * Sets the non-secure http port for this http web server. The port will
    * only be set if the server is not running. If a non-positive value
    * is passed, then a non-secure port will not be opened.
    * 
    * @param nonSecurePort the non-secure port for this http web server.
    */
   public void setNonSecurePort(int nonSecurePort)
   {
      if(!isRunning())
      {
         // save maximum connections
         int connections = getMaximumNonSecureConnections();
         
         // set port
         mNonSecurePort = nonSecurePort;
         
         // update maximum connections
         setMaximumNonSecureConnections(connections);
      }
   }
   
   /**
    * Gets the non-secure port for this http web server.
    * 
    * @return the non-secure port for this http web server.
    */
   public int getNonSecurePort()
   {
      return mNonSecurePort;
   }
   
   /**
    * Sets the secure http port for this http web server. The port will
    * only be set if the server is not running. If a non-positive value
    * is passed, then a secure port will not be opened.
    * 
    * @param securePort the secure port for this http web server.
    */
   public void setSecurePort(int securePort)
   {
      if(!isRunning())
      {
         // save maximum connections
         int connections = getMaximumSecureConnections();
         
         // set port
         mSecurePort = securePort;

         // update maximum connections
         setMaximumSecureConnections(connections);
      }
   }
   
   /**
    * Gets the secure port for this http web server.
    * 
    * @return the secure port for this http web server.
    */
   public int getSecurePort()
   {
      return mSecurePort;
   }
   
   /**
    * Sets an ssl certificate (from a keystore) for this web server.
    * 
    * @param keystore the name of the keystore file.
    * @param password the password to unlock the keystore.
    * @return true if the ssl certificate was successfully loaded,
    *         false if not.
    */
   public boolean setSSLKeystore(String keystore, String password)
   {
      return mSSLWebConnectionHandler.setSSLKeystore(keystore, password);
   }
   
   /**
    * Sets the maximum number of connections for the non-secure port.
    * 
    * If connections is 0, then there will be no maximum.
    * 
    * @param connections the maximum number of connections for the
    *                    non-secure port.
    */
   public void setMaximumNonSecureConnections(int connections)
   {
      mGenericWebConnectionHandler.setMaximumConnections(
         getNonSecurePort(), connections);
   }
   
   /**
    * Gets the maximum number of connections for the non-secure port.
    * 
    * If connections is 0, then there is no maximum.
    * 
    * @return the maximum number of connections for the non-secure port.
    */
   public int getMaximumNonSecureConnections()
   {
      return mGenericWebConnectionHandler.getMaximumConnections(
         getNonSecurePort());
   }
   
   /**
    * Sets the maximum number of connections for the secure port.
    * 
    * If connections is 0, then there will be no maximum.
    * 
    * @param connections the maximum number of connections for the
    *                    secure port.
    */
   public void setMaximumSecureConnections(int connections)
   {
      mSSLWebConnectionHandler.setMaximumConnections(
         getSecurePort(), connections);
   }
   
   /**
    * Gets the maximum number of connections for the secure port.
    * 
    * If connections is 0, then there is no maximum.
    * 
    * @return the maximum number of connections for the secure port.
    */
   public int getMaximumSecureConnections()
   {
      return mSSLWebConnectionHandler.getMaximumConnections(getSecurePort());
   }
   
   /**
    * Sets the web connection security manager for this HttpWebServer. The
    * web connection security manager can only be set once.
    * 
    * @param securityManager the web connection sercurity manager for this
    *                        HttpWebServer.
    */
   public void setWebConnectionSecurityManager(
      WebConnectionSecurityManager securityManager)
   {
      mHttpWebConnectionServicer.setWebConnectionSecurityManager(
         securityManager);
   }
   
   /**
    * Gets the web connection security manager for this HttpWebServer.
    * 
    * @return the web connection security manager for this HttpWebServer
    *         (can be null if there is no installed security manager).
    */
   public WebConnectionSecurityManager getWebConnectionSecurityManager()
   {
      return mHttpWebConnectionServicer.getWebConnectionSecurityManager();
   }   
}
