/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net;

/**
 * Any class that implements this interface can service a web connection.
 * 
 * @author Dave Longley
 */
public interface WebConnectionServicer
{
   /**
    * Services a web connection.
    * 
    * The web connection should be serviced and disconnected when the
    * servicing is completed.
    *  
    * @param webConnection the web connection to be serviced.
    */
   public void serviceWebConnection(WebConnection webConnection);
}
