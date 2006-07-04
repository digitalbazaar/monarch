/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;

/**
 * A class that implements this interface can provide an object for
 * dragging. This interface is used in conjunction with a
 * DraggableObjectSource.
 * 
 * @author Dave Longley
 */
public interface DraggableObjectProvider
{
   /**
    * Gets the object that is to be dragged (and potentially dropped) from
    * a DraggableObjectSource.
    * 
    * @param component the component to that is being dragged from.
    * 
    * @return the object to be dragged from a DraggableObjectSource.
    */
   public Object getDraggableObject(Component component);
}
