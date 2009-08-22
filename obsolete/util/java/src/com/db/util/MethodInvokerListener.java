/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

/**
 * Any object that wishes to receive a message when an MethodInvoker
 * finishes invoking its method must implement this interface.
 * 
 * @author Dave Longley
 */
public interface MethodInvokerListener
{
   /**
    * Called when a MethodInvoker finishes invoking its method.
    * 
    * @param mim a message with information about the method that was invoked.
    */
   public void methodInvoked(MethodInvokedMessage mim);
}
