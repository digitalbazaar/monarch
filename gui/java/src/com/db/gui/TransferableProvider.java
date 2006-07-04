/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;
import java.awt.datatransfer.Transferable;

/**
 * A class that implements this interface can provide an object for
 * transfer.
 * 
 * @author Dave Longley
 */
public interface TransferableProvider
{
   /**
    * Gets a Transferrable object for transfer. 
    * 
    * @param component the component that is being dragged from.
    * 
    * @return the Transferrable object.
    */
   public Transferable getTransferable(Component component);
}
