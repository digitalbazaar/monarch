/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

/**
 * A WebConnectionClient is a basic interface for a client that provides
 * WebConnections to WebConnectionServers.
 * 
 * @author Dave Longley
 */
public interface WebConnectionClient
{
   /**
    * Attempts to connect to the passed url.
    * 
    * @param url the url to connect to.
    * 
    * @return the web connection to the url or null if failure.
    */
   public WebConnection connect(String url);   
}
