/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

/**
 * A SoapSecurityManager is used to provide a particular security profile
 * for servicing soap requests.
 * 
 * @author Dave Longley
 */
public interface SoapSecurityManager
{
   /**
    * Runs a security check on the passed soap message and throws a
    * SecurityException if the soap message does not pass security.
    * 
    * Implementing classes should implement this method to set their
    * particular security profile.
    * 
    * When this method is called, a client is connected and awaiting a soap
    * response. Information about the client (i.e. its IP address) can be
    * checked in this method and a security exception can be thrown if
    * appropriate.
    * 
    * If there is a particular security policy for certain soap methods this
    * method should handle that policy and throw a security exception if
    * appropriate.
    * 
    * A soap fault will be raised that indicates that the client was not
    * authenticated if a security exception is thrown from this method. If
    * the passed soap message is not set to a soap fault, a default soap
    * fault will be set indicating that the client was not authenticated.
    * 
    * @param sm the soap message sent by the client. This soap message should
    *           be set to a soap fault if the soap message/client fail the
    *           security check.
    * 
    * @exception SecurityException thrown if the soap message/client fail the
    *                              security check.
    */
   public void checkSoapSecurity(RpcSoapMessage sm) throws SecurityException;
   
   /**
    * Returns true if the passed permission is allowed, false if not.
    *
    * @param permission a SoapPermission to check.
    *
    * @return true if the passed permission is allowed, false if not.
    */
   public boolean checkSoapPermission(SoapPermission permission);
}
