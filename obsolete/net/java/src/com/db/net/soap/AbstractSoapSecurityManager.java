/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

/**
 * An AbstractSoapSecurityManager is a SoapSecurityManager that can be
 * adds a simple API for denying certain SoapPermissions.
 * 
 * @author Dave Longley
 */
public abstract class AbstractSoapSecurityManager
implements SoapSecurityManager
{
   /**
    * The permissions to deny with this AbstractSoapSecurityManager.
    */
   protected List<SoapPermission> mDeniedPermissions;
   
   /**
    * Creates a new AbstractSoapSecurityManager.
    */
   public AbstractSoapSecurityManager()
   {
      // create the denied permissions vector
      mDeniedPermissions = new LinkedList<SoapPermission>();
   }
   
   /**
    * Adds a permission to deny with this AbstractSoapSecurityManager. Once
    * a permission has been denied, it cannot be allowed.
    * 
    * @param permission the SoapPermission to deny.
    * 
    * @exception IllegalArgumentException thrown if the passed permission is
    *                                     null.
    */
   public void denyPermission(SoapPermission permission)
   {
      if(permission == null)
      {
         throw new IllegalArgumentException("SoapPermission must not be null.");
      }
      
      // add the permission
      mDeniedPermissions.add(permission);
   }
   
   /**
    * Runs a security check on the passed soap message and throws a
    * SecurityException if the soap message does not pass security.
    * 
    * Extending classes should implement this method to define their
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
   public abstract void checkSoapSecurity(RpcSoapMessage sm)
   throws SecurityException;   
   
   /**
    * Returns true if the passed permission is allowed, false if not.
    *
    * @param permission a SoapPermission to check.
    *
    * @return true if the passed permission is allowed, false if not.
    */
   public boolean checkSoapPermission(SoapPermission permission)
   {
      boolean rval = true;
      
      // iterate through all of the denied permissions
      for(Iterator<SoapPermission> i = mDeniedPermissions.iterator();
          i.hasNext() && rval;)
      {
         SoapPermission denied = i.next();
         if(denied.equals(permission))
         {
            rval = false;
         }
      }
      
      return rval;
   }
}
