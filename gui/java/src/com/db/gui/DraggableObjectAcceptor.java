/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;

/**
 * A class that implements DraggableObjectAcceptor can accept a
 * draggable Object from a component with a DraggableObjectSource. This
 * interface is used in conjunction with a DraggableObjectDestination.
 * 
 * @author Dave Longley
 */
public interface DraggableObjectAcceptor
{
   /**
    * Returns true if the passed object can be accepted by the
    * given destination component.
    * 
    * @param obj the object to transfer.
    * @param action the drag 'n drop action:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    * 
    * @param destination the destination component.
    * 
    * @return true if the passed object can be accepted, false if not.
    */
   public boolean canAcceptDraggableObject(
      Object obj, int action, Component destination);
   
   /**
    * Accepts the passed object for the given destination component.
    * 
    * @param obj the object to accept.
    * @param action the drag 'n drop action:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    * 
    * @param destination the destination component.
    */
   public void acceptDraggableObject(
      Object obj, int action, Component destination);
}
