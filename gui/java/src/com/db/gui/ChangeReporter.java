/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import com.db.event.EventDelegate;

/**
 * Any class that wants to send changes via an event delegate can implement
 * this class. The source of the change event will be included in the event
 * data as "source".
 * 
 * @author Dave Longley
 * @author Mike Johnson
 */
public interface ChangeReporter
{
   /**
    * Gets the change delegate for this change reporter.
    *
    * @return the change delegate.
    */
   public EventDelegate getChangeDelegate();
}
