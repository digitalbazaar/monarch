/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.event;

/**
 * A generic event listener.
 * 
 * @author Dave Longley
 */
public interface EventListener
{
   /**
    * Called when an event occurs.
    * 
    * @param event the event that occurred.
    */
   public void eventOccurred(EventObject event);
}
