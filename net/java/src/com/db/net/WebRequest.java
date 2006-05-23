/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A web request.
 * 
 * @author Dave Longley
 */
public abstract class WebRequest
{
   /**
    * The web connection used to send or receive this web request.
    */
   protected WebConnection mWebConnection;
   
   /**
    * The amount of time (in milliseconds) that can pass before sending
    * the request times out.
    */
   protected long mTimeout;
   
   /**
    * Creates a new web request.
    * 
    * @param webConnection the web connection used to send or receive
    *                      this web request.
    */
   public WebRequest(WebConnection webConnection)
   {
      mWebConnection = webConnection;
      mTimeout = 0;
   }   
   
   /**
    * Returns true if this web request is valid, false if it is not.
    * 
    * @return true if this web request is valid, false if it is not.
    */
   public abstract boolean isValid();
   
   /**
    * Gets the remote IP address for this request.
    * 
    * @return the remote IP address for this request.
    */
   public String getRemoteIP()
   {
      return getWebConnection().getRemoteIP();
   }
   
   /**
    * Sets the amount of time (in milliseconds) that can pass before the
    * request times out. A non-positive number indicates an infinite timeout.
    * 
    * @param timeout the amount of time (in milliseconds) that can pass
    *                before the request times out. 
    */
   public void setTimeout(long timeout)
   {
      mTimeout = Math.max(0, timeout);
   }
   
   /**
    * Gets the amount of time (in milliseconds) that can pass before the
    * request times out. A non-positive number indicates an infinite timeout.
    * 
    * @return the amount of time (in milliseconds) that can pass before the
    *         request times out. 
    */
   public long getTimeout()
   {
      return mTimeout;
   }

   /**
    * Gets the web connection for this web request.
    * 
    * @return the web connection for this web request.
    */
   public WebConnection getWebConnection()
   {
      return mWebConnection;
   }
   
   /**
    * Gets the logger for this web request.
    * 
    * @return the logger for this web request.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbcommon");
   }
}
