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
    * Begins accepting web connections on the given port.
    * 
    * @param port the port to start accepting web connections on.
    */
   public void startAcceptingWebConnections(int port);
   
   /**
    * Stops accepting all web connections. 
    */
   public void stopAcceptingWebConnections();
   
   /**
    * Stops accepting all web connections on the specified port.
    * 
    * @param port the port to stop accepting web connections on.
    */
   public void stopAcceptingWebConnections(int port);
   
   /**
    * Accepts a single web connection on the specified port.
    * 
    * @param port the port to accept the web connection on.
    */
   public void acceptWebConnection(int port);

   /**
    * Accepts a single proxy web connection on the specified port.
    * 
    * @param port the port to accept the web connection on.
    * @param originalWebConnection the original web connection to proxy.
    */
   public void acceptProxyWebConnection(
      int port, WebConnection originalWebConnection);
   
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
    * Returns true if this web connection handler is accepting web connections
    * on the specified port, false if it is not.
    * 
    * @param port the port to check for web connections being accepted.
    * 
    * @return true if this connection handler is accepting connections on
    *         the specified port, false if not.
    */
   public boolean isAcceptingWebConnections(int port);
   
   /**
    * sets the maximum number of connections to handle concurrently
    * on a given port.
    *
    * @param port the port to set the maximum number of concurrent connections
    *             for.
    * @param connections the maximum number of concurrent connections allowed.
    */
   public void setMaximumConnections(int port, int connections);
   
   /**
    * Gets the maximum number of connections to handle concurrently
    * on a given port.
    *
    * @param port the port to get the maximum number of concurrent connections
    *             for.
    * 
    * @return the maximum number of connections to handle concurrently
    *         on a given port.
    */
   public int getMaximumConnections(int port);
   
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
}
