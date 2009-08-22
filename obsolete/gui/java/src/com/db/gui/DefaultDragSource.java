/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;
import java.awt.dnd.DragGestureListener;
import java.awt.dnd.DragGestureRecognizer;
import java.awt.dnd.DragSource;

/**
 * An DefaultDragSource allows objects to be dragged from some component.  
 * 
 * @author Dave Longley
 */
public class DefaultDragSource
{
   /**
    * The drag source. This object allows dragging from a component.
    */
   protected DragSource mDragSource;
   
   /**
    * The drag gesture recognizer. This object is used to recognize drag
    * gestures.
    */
   protected DragGestureRecognizer mDragGestureRecognizer;
   
   /**
    * Creates a new DefaultDragSource.
    * 
    * @param component the component this drag source is for.
    * @param action the permitted drag 'n drop action:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    *
    * @param dragGestureListener the drag gesture listener.
    */
   public DefaultDragSource(
      Component component, int action,
      DragGestureListener dragGestureListener)
   {
      // create the drag source
      mDragSource = DragSource.getDefaultDragSource();
      
      // create the drag gesture recognizer
      mDragGestureRecognizer = mDragSource.createDefaultDragGestureRecognizer(
         component, action, dragGestureListener);
   }
   
   /**
    * Gets the component this drag source is for.
    * 
    * @return the component this drag source is for.
    */
   public Component getComponent()
   {
      return mDragGestureRecognizer.getComponent();
   }
   
   /**
    * Gets the underlying drag source.
    * 
    * @return the underlying drag source.
    */
   public DragSource getDragSource()
   {
      return mDragSource;
   }
   
   /**
    * Gets the DragGestureRecognizer for this drag source.
    * 
    * @return the DragGestureRecognizer for this drag source.
    */
   public DragGestureRecognizer getDragGestureRecognizer()
   {
      return mDragGestureRecognizer;
   }
}
