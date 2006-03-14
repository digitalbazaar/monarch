/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.net;

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
