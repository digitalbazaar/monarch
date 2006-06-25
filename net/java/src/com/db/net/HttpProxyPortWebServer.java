/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

/**
 * An HttpProxyPortWebServer is a proxy port web server that has
 * generic and ssl web connection handlers that service connections using
 * an http web connection servicer.  
 * 
 * @author Dave Longley
 */
public class HttpProxyPortWebServer extends ProxyPortWebServer
{
   /**
    * Creates a new HttpProxyPortWebServer.
    */
   public HttpProxyPortWebServer()
   {
      // set internal web server to http web server
      super(new HttpWebServer());
      
      // accept an infinite number of connections each internally by default
      setMaximumNonSecureConnections(0);
      setMaximumSecureConnections(0);
      
      // accept an infinite number of proxy connections by default
      setMaximumProxyConnections(0);
   }
   
   /**
    * Gets the internal web server as an http web server.
    *
    * @return the internal web server as an http web server.
    */
   protected HttpWebServer getInternalHttpWebServer()
   {
      return (HttpWebServer)getInternalWebServer();
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
      getInternalHttpWebServer().addNonSecureHttpWebRequestServicer(hwrs, path);
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
      getInternalHttpWebServer().addSecureHttpWebRequestServicer(hwrs, path);
   }
   
   /**
    * Removes a non-secure http web request servicer from this server.
    * 
    * @param path the path to the servicer.
    */
   public void removeNonSecureHttpWebRequestServicer(String path)
   {
      getInternalHttpWebServer().removeNonSecureHttpWebRequestServicer(path);
   }
   
   /**
    * Removes a secure http web request servicer from this server.
    * 
    * @param path the path to the servicer.
    */
   public void removeSecureHttpWebRequestServicer(String path)
   {
      getInternalHttpWebServer().removeSecureHttpWebRequestServicer(path);
   }
   
   /**
    * Starts this http web server using the passed ports. Only a non-secure
    * connection handler will be active (no SSL support).
    * 
    * @param proxyPort the proxy port to accept connections on.
    * @param nonSecureHttpPort the port for non-secure http traffic.
    */
   public synchronized void startNonSecure(int proxyPort, int nonSecureHttpPort)
   {
      // set ports
      getInternalHttpWebServer().setNonSecurePort(nonSecureHttpPort);
      getInternalHttpWebServer().setSecurePort(0);
      
      // start server
      start(proxyPort);
   }
   
   /**
    * Starts this http web server using the passed ports. Only a secure
    * connection handler will be active (no non-SSL support).
    * 
    * @param proxyPort the proxy port to accept connections on.
    * @param secureHttpPort the port for secure http traffic.
    */
   public synchronized void startSecure(int proxyPort, int secureHttpPort)
   {
      // set ports
      getInternalHttpWebServer().setNonSecurePort(0);
      getInternalHttpWebServer().setSecurePort(secureHttpPort);

      // start server
      start(proxyPort);
   }
   
   /**
    * Starts this http proxy port web server using the passed ports.
    * 
    * @param proxyPort the proxy port to accept connections on.
    * @param nonSecureHttpPort the local port for non-secure http traffic.
    * @param secureHttpPort the local port for secure http traffic.
    */
   public synchronized void start(
      int proxyPort, int nonSecureHttpPort, int secureHttpPort)   
   {
      // set ports
      getInternalHttpWebServer().setNonSecurePort(nonSecureHttpPort);
      getInternalHttpWebServer().setSecurePort(secureHttpPort);
      
      // start server
      start(proxyPort);
   }
   
   /**
    * Sets the server name for the internal http web server.
    * 
    * @param serverName the server name for the internal http web server.
    */
   public void setServerName(String serverName)
   {
      getInternalHttpWebServer().setServerName(serverName);
   }
   
   /**
    * Sets an ssl certificate (from a keystore) for the internal
    * http web server.
    * 
    * @param keystore the name of the keystore file.
    * @param password the password to unlock the keystore.
    * @return true if the ssl certificate was successfully loaded,
    *         false if not.
    */
   public boolean setSSLKeystore(String keystore, String password)
   {
      return getInternalHttpWebServer().setSSLKeystore(keystore, password);
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
      getInternalHttpWebServer().setMaximumNonSecureConnections(connections);
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
      return getInternalHttpWebServer().getMaximumNonSecureConnections();
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
      getInternalHttpWebServer().setMaximumSecureConnections(connections);
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
      return getInternalHttpWebServer().getMaximumSecureConnections();
   }   
}
