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
public interface DraggableObjectPool
{
   /**
    * Gets an object that is to be dragged (and potentially dropped) from
    * a DraggableObjectSource.
    * 
    * @param source the component to that is being dragged from.
    * 
    * @return an object to be dragged from a DraggableObjectSource or null
    *         if no object can currently be dragged.
    */
   public Object getDraggableObject(Component source);
   
   /**
    * Called when a draggable Object from this pool is dropped. The dropped
    * object may or may not need to be removed from this pool depending on
    * the drag 'n drop action. 
    * 
    * @param obj the draggable object that was dropped.
    * @param action the drag 'n drop action:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    *  
    * @param source the component that the draggable Object was dragged from.
    */
   public void draggableObjectDropped(Object obj, int action, Component source);
}
