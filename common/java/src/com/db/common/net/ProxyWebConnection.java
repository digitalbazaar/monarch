/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.net;

import java.net.Socket;

import com.db.common.MethodInvoker;
import com.db.common.logging.Logger;

/**
 * This class acts as a proxy between an incoming web connection and
 * a different server than it was originally connected to.
 * 
 * When writing to this web connection, data will be sent to the new
 * web connection socket (the client side). Any data written to that socket
 * will then be piped to the original web connection. Whenever reading
 * from this web connection, data will be piped from the original
 * web connection through the new web connection to this web connection.
 * 
 * @author Dave Longley
 */
public class ProxyWebConnection extends GenericWebConnection
{
   /**
    * The original web connection.
    */
   protected WebConnection mOriginalWebConnection;

   /**
    * The new web connection.
    */
   protected WebConnection mNewWebConnection;
   
   /**
    * The thread for reading from the original web connection and
    * writing to the new web connection. 
    */
   protected Thread mOriginalToNewProxyThread;
   
   /**
    * The thread for reading from the new web connection and
    * writing to the original connection.
    */
   protected Thread mNewToOriginalProxyThread;
   
   /**
    * Creates a new ProxyWebConnection.
    * 
    * @param workerSocket the worker socket for this web connection.
    * @param originalWebConnection the original web connection.
    * @param newWebConnection the new web connection.
    */
   public ProxyWebConnection(Socket workerSocket,
                             WebConnection originalWebConnection,
                             WebConnection newWebConnection)
   {
      super(workerSocket);
      
      // set original and new web connections
      mOriginalWebConnection = originalWebConnection;
      mNewWebConnection = newWebConnection;
      
      // set the remote IP to the original connection's IP
      setRemoteIP(mOriginalWebConnection.getRemoteIP());
      
      // set the remote port to the original connection's port
      setRemotePort(mOriginalWebConnection.getRemotePort());
   }
   
   /**
    * Proxies data from one connection to another. This method is called
    * automatically on two separate threads once this proxy web connection
    * is started.
    * 
    * @param readConnection the web connection to read from.
    * @param writeConnection the web connection to write to.
    */
   public void proxyData(WebConnection readConnection,
                         WebConnection writeConnection)
   {
      try
      {
         // read from connection until end of stream
         byte[] buffer = new byte[65536];
         int numBytes = -1;
         while((numBytes = readConnection.read(buffer, 0, buffer.length)) != -1)
         {
            if(writeConnection.isConnected())
            {
               // write to write connection
               writeConnection.write(buffer, 0, numBytes);
            }
         }
         
         // close down streams
         readConnection.getReadStream().close();
         writeConnection.getWriteStream().close();
      }
      catch(Throwable t)
      {
         String message = t.getMessage().toLowerCase();
         
         if(message.indexOf("socket closed") != -1)
         {
            // socket closed
         }
         else if(message.indexOf("connection reset") == -1)
         {
            // not connection reset, there was an error, so display error
            getLogger().error(Logger.getStackTrace(t));
         }
      }
   }
   
   /**
    * Starts proxying data for this proxy web connection.
    */
   public synchronized void startProxy()
   {
      // make sure proxy hasn't already been started
      if(mOriginalToNewProxyThread == null && mNewToOriginalProxyThread == null)
      {
         // read from original web connection and write to new web connection
         Object[] params1 = new Object[]{mOriginalWebConnection,
                                         mNewWebConnection};
         MethodInvoker originalToNew =
             new MethodInvoker(this, "proxyData", params1);
      
         // read from new web connection and write to original web connection
         Object[] params2 = new Object[]{mNewWebConnection,
                                         mOriginalWebConnection};
         MethodInvoker newToOriginal =
            new MethodInvoker(this, "proxyData", params2);
      
         // store threads
         mOriginalToNewProxyThread = originalToNew;
         mNewToOriginalProxyThread = newToOriginal;
         
         // invoke methods in background
         originalToNew.backgroundExecute();
         newToOriginal.backgroundExecute();
      }
   }
   
   /**
    * Disconnects the proxy (closes the sockets).
    */
   public synchronized void disconnect()
   {
      boolean previouslyDisconnected = !isConnected();
      
      // disconnect
      super.disconnect();
      
      if(!previouslyDisconnected)
      {
         getLogger().debug("proxy web connection disconnected,ip=" +
                           getRemoteIP());
      }
   }
}
