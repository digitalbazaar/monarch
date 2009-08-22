/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

import java.util.LinkedList;
import java.util.List;

/**
 * An ExtensibleWebConnectionSecurityManager is a WebConnectionSecurityManager
 * that can be extended to use other WebConnectionSecurityManagers. Extensions
 * can be added to this manager, but never removed.
 * 
 * @author Dave Longley
 */
public class ExtensibleWebConnectionSecurityManager
implements WebConnectionSecurityManager
{
   /**
    * The extensions for this ExtensibleWebConnectionSecurityManager.
    */
   protected List<WebConnectionSecurityManager> mExtensions;
   
   /**
    * Creates a new ExtensibleWebConnectionSecurityManager.
    */
   public ExtensibleWebConnectionSecurityManager()
   {
      // create the extensions list
      mExtensions = new LinkedList<WebConnectionSecurityManager>();
   }
   
   /**
    * Adds an extension to this ExtensibleWebConnectionSecurityManager. Once
    * an extension has been added, it cannot be removed.
    * 
    * @param extension the WebConnectionSecurityManager extension to add.
    */
   public void addExtension(WebConnectionSecurityManager extension)
   {
      // add the extension
      mExtensions.add(extension);
   }
   
   /**
    * Runs a security check on the passed web connection and throws a
    * SecurityException if the web connection does not pass security.
    * 
    * This class runs a security check on the passed web connection using
    * every extension that has been added to this instance.
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
      // iterate through all of the extensions and run a security check
      for(WebConnectionSecurityManager extension: mExtensions)
      {
         extension.checkWebConnectionSecurity(webConnection);
      }
   }
}
