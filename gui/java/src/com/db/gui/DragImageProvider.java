/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Image;
import java.awt.Point;

/**
 * A DragImageProvider provides an image and image offset to be used
 * while dragging (in a drag 'n drop environment).
 * 
 * @author Dave Longley
 */
public interface DragImageProvider
{
   /**
    * Gets the drag image. This is the image to display while dragging.
    * 
    * @return the drag image.
    */
   public Image getDragImage();
   
   /**
    * Gets the drag image offset. This is the offset, from the cursor,
    * to display the drag image at.
    * 
    * @return the drag image offset.
    */
   public Point getDragImageOffset();
}
