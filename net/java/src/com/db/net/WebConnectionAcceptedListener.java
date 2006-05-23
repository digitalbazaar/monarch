/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

/**
 * A class that implements this listener can receive notifications when
 * web connections are accepted.  
 * 
 * @author Dave Longley
 */
public interface WebConnectionAcceptedListener
{
   /**
    * Called when a web connection is accepted.
    * 
    * @param webConnection the accepted web connection.
    */
   public void webConnectionAccepted(WebConnection webConnection);
}
