/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.util.Iterator;
import java.util.Vector;

/**
 * An ExtensibleSoapSecurityManager is a SoapSecurityManager that can be
 * extended to use other SoapSecurityManager. Extensions can be added to
 * this manager, but never removed.
 * 
 * @author Dave Longley
 */
public class ExtensibleSoapSecurityManager extends AbstractSoapSecurityManager
{
   /**
    * The extensions for this ExtensibleSoapSecurityManager.
    */
   protected Vector<SoapSecurityManager> mExtensions;
   
   /**
    * Creates a new ExtensibleSoapSecurityManager.
    */
   public ExtensibleSoapSecurityManager()
   {
      // create the extensions vector
      mExtensions = new Vector<SoapSecurityManager>();
   }
   
   /**
    * Adds an extension to this ExtensibleSoapSecurityManager. Once an
    * extension has been added, it cannot be removed.
    * 
    * @param extension the SoapSecurityManager extension to add.
    * 
    * @exception IllegalArgumentException thrown if the passed extension is
    *                                     null.
    */
   public void addExtension(SoapSecurityManager extension)
   {
      if(extension == null)
      {
         throw new IllegalArgumentException(
            "SoapSecurityManager extension must not be null.");
      }
      
      // add the extension
      mExtensions.add(extension);
   }
   
   /**
    * Runs a security check on the passed soap message and throws a
    * SecurityException if the soap message does not pass security.
    * 
    * This class runs a security check on the passed soap message using
    * every extension that has been added to this instance.
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
   @Override
   public void checkSoapSecurity(RpcSoapMessage sm) throws SecurityException   
   {
      // iterate through all of the extensions and run a security check
      for(SoapSecurityManager extension: mExtensions)
      {
         extension.checkSoapSecurity(sm);
      }
   }
   
   /**
    * Returns true if the passed permission is allowed, false if not.
    *
    * @param permission a SoapPermission to check.
    *
    * @return true if the passed permission is allowed, false if not.
    */
   @Override
   public boolean checkSoapPermission(SoapPermission permission)
   {
      boolean rval = true;
      
      // check this manager's permissions
      rval = super.checkSoapPermission(permission);
      
      // iterate through all of the extensions and check permissions
      for(Iterator<SoapSecurityManager> i = mExtensions.iterator();
          i.hasNext() && rval;)
      {
         SoapSecurityManager extension = i.next();
         rval = extension.checkSoapPermission(permission);
      }
      
      return rval;
   }
}
