/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * This class is a delegate that sends messages to
 * WebConnectionAcceptedListeners.
 * 
 * @author Dave Longley
 */
public class WebConnectionAcceptedDelegate
{
   /**
    * A list of web connection accepted listeners.
    */
   protected Vector mWebConnectionAcceptedListeners;

   /**
    * Constructs a new web connection accepted delegate.
    */
   public WebConnectionAcceptedDelegate()
   {
      // create vector for web connection accepted listeners
      mWebConnectionAcceptedListeners = new Vector();
   }

   /**
    * Adds a web connection accepted listener.
    *
    * @param listener the web connection accepted listener to add.
    * @return true if successfully added, false if not.
    */
   public synchronized boolean addWebConnectionAcceptedListener(
         WebConnectionAcceptedListener listener)
   {
      return mWebConnectionAcceptedListeners.add(listener);
   }
   
   /**
    * Removes a web connection accepted listener.
    * 
    * @param listener the web connection accepted listener to remove.
    * @return true if successfully removed, false if not.
    */
   public synchronized boolean removeWebConnectionAcceptedListener(
         WebConnectionAcceptedListener listener)
   {
      return mWebConnectionAcceptedListeners.remove(listener);
   }
   
   /**
    * Sends a web connection accepted message to all listeners.
    * 
    * @param webConnection the accepted web connection.
    */
   public void fireWebConnectionAccepted(WebConnection webConnection)
   {
      // send message to all listeners
      Iterator i = mWebConnectionAcceptedListeners.iterator();
      while(i.hasNext())
      {
         WebConnectionAcceptedListener listener =
            (WebConnectionAcceptedListener)i.next();
         listener.webConnectionAccepted(webConnection);
      }
   }
   
   /**
    * Gets the number of listeners of this delegate.
    * 
    * @return the number of listeners of this delegate.
    */
   public int getListenerCount()
   {
      return mWebConnectionAcceptedListeners.size();
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
