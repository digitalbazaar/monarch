/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.net.ServerSocket;

import com.db.logging.Logger;

/**
 * This class handles generic web connections (no special connection protocols).
 * 
 * @author Dave Longley
 */
public class GenericWebConnectionHandler extends AbstractWebConnectionHandler
{
   /**
    * Creates a new GenericWebConnectionHandler.
    * 
    * @param servicer the web connection servicer for this
    *                 generic web connection handler. 
    */
   public GenericWebConnectionHandler(WebConnectionServicer servicer)
   {
      // pass web connection servicer for servicing web connections
      super(servicer);
   }
   
   /**
    * Creates a new server socket for listening on a port. 
    * 
    * @param port the port the server socket will listen on.
    * @return the new server socket.
    */
   protected ServerSocket createServerSocket(int port)
   {
      ServerSocket serverSocket = null; 
      
      try
      {
         // create a generic server socket
         serverSocket = new ServerSocket(port);
      }
      catch(Throwable t)
      {
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return serverSocket;
   }
   
   /**
    * Indicates whether or not connections handled by this web
    * connection handler should be considered secure.
    * 
    * Web connections handled by a generic web connection handler are
    * not considered secure. 
    * 
    * @return true if the connections handled by this web connection
    *         handler should be considered secure, false if they should
    *         not be considered secure. 
    */
   public boolean webConnectionsSecure()
   {
      // generic connections not considered secure
      return false;
   }
   
   /**
    * Indicates whether or not the passed web connection's protocol is
    * supported by this web connection handler. This method may
    * read from the web connection as long as it unreads whatever
    * data was read. This method is useful for determining whether or not
    * a web connection handler should accept a proxy web connection. 
    *
    * @param webConnection the web connection to inspect.
    * @return true if the web connection's protocol is supported, false
    *         if it is not.
    */
   public boolean isWebConnectionProtocolSupported(WebConnection webConnection)
   {
      // all protocols are supported
      return true;
   }
}
