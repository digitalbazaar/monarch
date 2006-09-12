/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

/**
 * A generic web connection handler. A class that implements this interface
 * can handle web connections.
 * 
 * @author Dave Longley
 */
public interface WebConnectionHandler
{
   /**
    * Starts accepting web connections on the given port. If this web
    * connection handler is already listening on another port, this method
    * should return false. If it is already listening on the passed port,
    * it should have no effect and return true.
    * 
    * @param port the port to start accepting web connections on.
    * 
    * @return true if this web connection handler is now listening on the
    *         specified port, false if not.
    */
   public boolean startAcceptingWebConnections(int port);
   
   /**
    * Stops accepting all web connections. 
    */
   public void stopAcceptingWebConnections();
   
   /**
    * Accepts a single proxy web connection. If this web connection handler
    * is not accepting connections, then this method should return false.
    * 
    * @param originalWebConnection the original web connection to proxy.
    * 
    * @return true if the proxy web connection was accepted, false if not.
    */
   public boolean acceptProxyWebConnection(WebConnection originalWebConnection);
   
   /**
    * Indicates whether or not connections handled by this web
    * connection handler should be considered secure.
    * 
    * @return true if the connections handled by this web connection
    *         handler should be considered secure, false if they should
    *         not be considered secure. 
    */
   public boolean webConnectionsSecure();
   
   /**
    * Indicates whether or not the passed web connection's protocol is
    * supported by this web connection handler. This method may
    * read from the web connection as long as it unreads whatever
    * data was read. This method is useful for determining whether or not
    * a web connection handler should accept a proxy web connection. 
    *
    * @param webConnection the web connection to inspect.
    * 
    * @return true if the web connection's protocol is supported, false
    *         if it is not.
    */
   public boolean isWebConnectionProtocolSupported(WebConnection webConnection);

   /**
    * Returns true if this web connection handler is accepting web connections,
    * false if it is not.
    * 
    * @return true if this connection handler is accepting connections,
    *         false if not.
    */
   public boolean isAcceptingWebConnections();
   
   /**
    * Sets the maximum number of connections to handle concurrently.
    *
    * @param connections the maximum number of concurrent connections allowed.
    */
   public void setMaxConcurrentConnections(int connections);
   
   /**
    * Gets the maximum number of connections to handle concurrently.
    *
    * @return the maximum number of connections to handle concurrently.
    */
   public int getMaxConcurrentConnections();
   
   /**
    * Terminates all web connections this web connection handler is handling.
    * Tries to shutdown web connections gracefully.
    */
   public void terminateWebConnections();
   
   /**
    * Disconnects all web connections this web connection handler is
    * handling immediately.
    */
   public void disconnectWebConnections();
   
   /**
    * Returns the number of web connections this web connection handler is
    * currently servicing.
    * 
    * @return the number of web connections currently being serviced.
    */
   public int webConnectionsBeingServiced();
   
   /**
    * Gets the port that this web connection handler is accepting web
    * connections on.
    * 
    * @return the port that this web connection handler is accepting web
    *         connections on, or 0 if it is not accepting connections.
    */
   public int getPort();
}
