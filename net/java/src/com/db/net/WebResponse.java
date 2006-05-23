/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import com.db.common.logging.Logger;
import com.db.common.logging.LoggerManager;

/**
 * A web response.
 * 
 * @author Dave Longley
 */
public abstract class WebResponse
{
   /**
    * The web connection used to send or receive this web response.
    */
   protected WebConnection mWebConnection;
   
   /**
    * The amount of time (in milliseconds) that can pass before sending
    * the response times out.
    */
   protected long mTimeout;
   
   /**
    * Creates a new web response.
    * 
    * @param webConnection the web connection used to send or receive
    *                      this web response.
    */
   public WebResponse(WebConnection webConnection)
   {
      mWebConnection = webConnection;
      mTimeout = 0;
   }   
   
   /**
    * Returns true if this web response is valid, false if it is not.
    * 
    * @return true if this web response is valid, false if it is not.
    */
   public abstract boolean isValid();
   
   /**
    * Sets the amount of time (in milliseconds) that can pass before the
    * response times out. A non-positive number indicates an infinite timeout. 
    * 
    * @param timeout the amount of time (in milliseconds) that can pass
    *                before the response times out. 
    */
   public void setTimeout(long timeout)
   {
      mTimeout = Math.max(0, timeout);
   }
   
   /**
    * Gets the amount of time (in milliseconds) that can pass before the
    * response times out. A non-positive number indicates an infinite timeout.
    * 
    * @return the amount of time (in milliseconds) that can pass before the
    *         response times out. 
    */
   public long getTimeout()
   {
      return mTimeout;
   }
   
   /**
    * Gets the web connection for this web response.
    * 
    * @return the web connection for this web response.
    */
   public WebConnection getWebConnection()
   {
      return mWebConnection;
   }
   
   /**
    * Gets the logger for this web response.
    * 
    * @return the logger for this web response.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbcommon");
   }
}
