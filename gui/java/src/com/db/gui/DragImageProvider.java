/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;
import java.awt.Image;
import java.awt.Point;

/**
 * A DragImageProvider provides an image and image offset to be used
 * while dragging (in a drag 'n drop environment) from a component. This
 * interface is used in conjunction with a DraggableObjectSource.
 * 
 * @author Dave Longley
 */
public interface DragImageProvider
{
   /**
    * Gets the drag image. This is the image to display while dragging.
    * 
    * @param obj the object being dragged.
    * @param action the drag 'n drop action:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    * 
    * @param component the component the drag is over
    *                  (either a source or destination).
    *  
    * @return the drag image.
    */
   public Image getDragImage(Object obj, int action, Component component);
   
   /**
    * Gets the drag image offset. This is the offset, from the cursor,
    * to display the drag image at.
    * 
    * @param obj the object being dragged.
    * @param action the drag 'n drop action:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    * 
    * @param component the component the drag is over
    *                  (either a source or destination).
    * 
    * @return the drag image offset.
    */
   public Point getDragImageOffset(Object obj, int action, Component component);
}
