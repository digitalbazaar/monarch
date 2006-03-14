/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.net;

import java.util.Vector;

/**
 * An interface for rpc clients.
 * 
 * @author Dave Longley
 */
public interface RpcClient
{
   /**
    * Calls a remote method.
    * 
    * @param method the name of the remote method.
    * @param params the parameters for the remote method.
    * @return the return value from the remote method or null. 
    */
   public Object callRemoteMethod(String method, Vector params);
}
