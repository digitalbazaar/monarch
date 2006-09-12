/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A ProxyPortWebConnectionServicer services a web connection by proxying it
 * to an internal web server.
 * 
 * @author Dave Longley
 */
public class ProxyPortWebConnectionServicer
extends AbstractWebConnectionServicer
{
   /**
    * A prioritized list of web connection handlers that is used to
    * determine which web connection handler should handle a new web
    * connection. The first web connection handler in the list that
    * supports the protocol of the accepted web connection will be the
    * web connection handler to have it proxied to it.
    */
   protected Vector mWebConnectionHandlerPriorityList;
   
   /**
    * A table mapping web connection handlers to the ports they accept
    * connections on.
    */
   protected HashMap mPortTable; 
   
   /**
    * Creates a new ProxyPortWebConnectionServicer.
    */
   public ProxyPortWebConnectionServicer()
   {
      // create the web connection handler priority list
      mWebConnectionHandlerPriorityList = new Vector();
      
      // create the port table
      mPortTable = new HashMap();
   }
   
   /**
    * A ProxyWebConnectionServicer services a web connection by 
    * running it through a connection analyzer to determine what proxy it
    * should connect to and then connects to it.
    * 
    * Services a web connection.
    * 
    * The web connection should be serviced and disconnected when the
    * servicing is completed.
    *  
    * @param webConnection the web connection to be serviced.
    */   
   public void serviceSecureWebConnection(WebConnection webConnection)
   {
      getLogger().debug(getClass(),
         "servicing web connection,ip=" + webConnection.getRemoteIP() + "...");
      
      // go through the priority list to determine which web connection
      // handler to use
      boolean servicing = false;
      Iterator i = mWebConnectionHandlerPriorityList.iterator();
      while(i.hasNext())
      {
         WebConnectionHandler wch = (WebConnectionHandler)i.next();
         
         // if protocol is supported, then accept proxy web connection
         if(wch.isWebConnectionProtocolSupported(webConnection))
         {
            // accept proxy connection
            wch.acceptProxyWebConnection(webConnection);
            servicing = true;
            break;
         }
      }
      
      if(servicing)
      {
         try
         {
            // while web connection is still connected, wait
            while(webConnection.isConnected())
            {
               Thread.sleep(10);
            }
         }
         catch(Throwable t)
         {
         }
      }
      else
      {
         // disconnect web connection if it isn't already disconnected
         webConnection.disconnect();
      }
      
      getLogger().debug(getClass(),
         "finished servicing web connection,ip=" +
         webConnection.getRemoteIP());
   }
   
   /**
    * Adds a web connection handler to the priority list. This list determines
    * which web connection handler will handle a proxied web connection.
    * 
    * @param wch the web connection handler to add to the priority list.
    * @param port the port that the web connection handler will listen on.
    */
   public void addPrioritizedWebConnectionHandler(WebConnectionHandler wch,
                                                  int port)
   {
      if(!hasPrioritizedWebConnectionHandler(wch))
      {
         // add the web connection handler to the priority list
         mWebConnectionHandlerPriorityList.add(wch);
      }
      
      // add a port entry for the web connection handler
      mPortTable.put(wch, "" + port);
   }
   
   /**
    * Removes a web connection handler from the priority list. The passed
    * web connection handler will no longer be considered for receiving
    * proxied web connections.
    * 
    * @param wch the web connection handler to remove from the priority list.
    */
   public void removePrioritizedWebConnectionHandler(WebConnectionHandler wch)
   {
      // remove the web connection handler from the priority list
      mWebConnectionHandlerPriorityList.remove(wch);
      
      // remove the port entry for the web connection handler
      mPortTable.remove(wch);
   }
   
   /**
    * Removes all web connection handlers from the priority list. No
    * web connection handlers will be considered for receiving proxied
    * web connections any longer.
    */
   public void removeAllPrioritizedWebConnectionHandlers()
   {
      // clear the web connection handler priority list
      mWebConnectionHandlerPriorityList.clear();
      
      // remove all port entries
      mPortTable.clear();
   }
   
   /**
    * Returns true if the passed web connection handler is in the priority
    * list, false if not.
    * 
    * @param wch the web connection handler to look for.
    * @return true if the passed web connection handler is in the priority
    *         list, false if not.
    */
   public boolean hasPrioritizedWebConnectionHandler(WebConnectionHandler wch)
   {
      return mWebConnectionHandlerPriorityList.contains(wch);
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
}
