/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

/**
 * A WebConnectionSecurityManager is used to provide a particular security
 * profile for servicing web connections.
 * 
 * @author Dave Longley
 */
public interface WebConnectionSecurityManager
{
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
   throws SecurityException;
}
