/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * An AbstractWebConnectionServicer provides WebConnectionSecurityManager
 * functionality for a SecureWebConnectionServicer.
 * 
 * Any extending class can install a WebConnectionSecurityManager to provide
 * a particular security profile for this web connection servicer.
 * 
 * @author Dave Longley
 */
public abstract class AbstractWebConnectionServicer
implements SecureWebConnectionServicer
{
   /**
    * The web connection security manager for this web connection servicer. 
    */
   protected WebConnectionSecurityManager mWebConnectionSecurityManager;
   
   /**
    * Creates a new AbstractWebConnectionServicer with no installed
    * WebConnectionSecurityManager.
    */
   public AbstractWebConnectionServicer()
   {
      // set no security manager
      setWebConnectionSecurityManager(null);
   }

   /**
    * Runs a security check on the passed web connection and throws a
    * SecurityException if the web connection does not pass security.
    * 
    * Implementing classes should implement this method to set their
    * particular security profile.
    * 
    * When this method is called, a client is connected using the passed
    * web connection and awaiting connection servicing. Information about
    * the client (i.e. its IP address) can be checked in this method and a
    * security exception can be thrown if appropriate.
    * 
    * The web connection will be disconnected once the security exception
    * has been thrown by this method. This method may send some data over
    * the web connection or delegate to some other class to send data
    * over the web connection before throwing its exception if that
    * kind of behavior is appropriate for whatever security policy is
    * implemented.
    * 
    * @param webConnection the WebConnection to check the security for.
    * 
    * @exception SecurityException thrown if the WebConnection fails the
    *                              security check.
    */
   public void checkWebConnectionSecurity(WebConnection webConnection)
   throws SecurityException   
   {
      // get the security manager, if one exists
      if(getWebConnectionSecurityManager() != null)
      {
         // check security
         getWebConnectionSecurityManager().checkWebConnectionSecurity(
            webConnection);
      }      
   }
   
   /**
    * Services a web connection by first running a security check on it. If
    * the connection fails the security check, an exception is thrown and
    * it will be disconnected.
    * 
    * @param webConnection the web connection to be serviced.
    */
   public void serviceWebConnection(WebConnection webConnection)
   {
      try
      {
         // check security before servicing connection
         checkWebConnectionSecurity(webConnection);
         
         // service web connection
         serviceSecureWebConnection(webConnection);
      }
      catch(SecurityException e)
      {
         // log error
         getLogger().error(getClass(),
            "Exception thrown while servicing web connection!," +
            "exception= " + e);
         getLogger().debug(getClass(), Logger.getStackTrace(e));
         
         // disconnect web connection
         webConnection.disconnect();
      }
   }
   
   /**
    * Services a web connection that has passed a security check.
    * 
    * The web connection should be serviced and disconnected when the
    * servicing is completed.
    *  
    * @param webConnection the web connection to be serviced.
    */
   public abstract void serviceSecureWebConnection(WebConnection webConnection);     
   
   /**
    * Sets the web connection security manager for this web connection
    * servicer. The web connection security manager can only be set once.
    * 
    * @param securityManager the web connection sercurity manager for this
    *                        web connection servicer.
    */
   public void setWebConnectionSecurityManager(
      WebConnectionSecurityManager securityManager)
   {
      // only set web connection security manager if it is null
      if(getWebConnectionSecurityManager() == null)
      {
         mWebConnectionSecurityManager = securityManager;
      }
   }
   
   /**
    * Gets the web connection security manager for this web connection
    * servicer.
    * 
    * @return the web connection security manager for this web connection
    *         servicer (can be null if there is no installed security manager).
    */
   public WebConnectionSecurityManager getWebConnectionSecurityManager()
   {
      return mWebConnectionSecurityManager;
   }
   
   /**
    * Gets the logger for this web connection servicer.
    * 
    * @return the logger for this web connection servicer.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }   
}
