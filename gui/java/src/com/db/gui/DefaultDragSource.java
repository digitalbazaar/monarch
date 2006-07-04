/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;
import java.awt.Image;
import java.awt.Point;
import java.awt.datatransfer.Transferable;
import java.awt.dnd.DragGestureEvent;
import java.awt.dnd.DragGestureListener;
import java.awt.dnd.DragGestureRecognizer;
import java.awt.dnd.DragSource;
import java.awt.dnd.DragSourceListener;

/**
 * An DefaultDragSource allows objects to be dragged from some component
 * using a DefaultGestureRecognizer (based on the platform's interpretation
 * of drag gestures).  
 * 
 * @author Dave Longley
 */
public class DefaultDragSource implements DragGestureListener
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
    * The transferable provider. This object provides the Transferable
    * object when initiating a drag. 
    */
   protected TransferableProvider mTransferableProvider;
   
   /**
    * The drag image provider. This object provides the drag image
    * and drag image offset for dragging.
    */
   protected DragImageProvider mDragImageProvider;
   
   /**
    * The drag source listener. This object can be used to handle
    * specific behaviors during a drag.
    */
   protected DragSourceListener mDragSourceListener;
   
   /**
    * Creates a new DefaultDragSource.
    * 
    * @param component the component this drag source is for.
    * @param dragAction the action to use when dragging:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    *
    * @param transferableProvider the object that provides the Transferable
    *                             object when dragging.
    */
   public DefaultDragSource(
      Component component, int dragAction,
      TransferableProvider transferableProvider)   
   {
      this(component, dragAction, transferableProvider, null, null);
   }
   
   /**
    * Creates a new DefaultDragSource.
    * 
    * @param component the component this drag source is for.
    * @param dragAction the action to use when dragging:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    *
    * @param transferableProvider the object that provides the Transferable
    *                             object when dragging.
    * @param dragImageProvider the object that provides the drag image
    *                          (can be null).
    */
   public DefaultDragSource(
      Component component, int dragAction,
      TransferableProvider transferableProvider,
      DragImageProvider dragImageProvider)
   {
      this(component, dragAction, transferableProvider,
         dragImageProvider, null);
   }
   
   /**
    * Creates a new DefaultDragSource.
    * 
    * @param component the component this drag source is for.
    * @param dragAction the action to use when dragging:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    *
    * @param transferableProvider the object that provides the Transferable
    *                             object when dragging.
    * @param dragImageProvider the object that provides the drag image
    *                          (can be null).
    * @param dragSourceListener the drag source listener (can be null).
    */
   public DefaultDragSource(
      Component component, int dragAction,
      TransferableProvider transferableProvider,
      DragImageProvider dragImageProvider,
      DragSourceListener dragSourceListener)
   {
      // create the drag source
      mDragSource = DragSource.getDefaultDragSource();
      
      // create the drag gesture recognizer
      mDragGestureRecognizer = mDragSource.createDefaultDragGestureRecognizer(
         component, dragAction, this);
      
      // store the transferable provider
      mTransferableProvider = transferableProvider;
      
      // store the drag image provider
      mDragImageProvider = dragImageProvider;
      
      // store drag source listener
      mDragSourceListener = dragSourceListener;
   }
   
   /**
    * Called when a <code>DragGestureRecognizer</code> has detected a 
    * platform-dependent drag initiating gesture and is notifying 
    * this listener in order for it to initiate the action for the user.
    * 
    * The action for the user is to take the selected wares in a WareJList
    * and put them in a transferable wares object.
    * 
    * @param dge the <code>DragGestureEvent</code> describing 
    *            the gesture that has just occurred.
    */
   public void dragGestureRecognized(DragGestureEvent dge)      
   {
      // get the Transferable object from the TransferableProvider
      Transferable transferable = mTransferableProvider.getTransferable(
         dge.getComponent());
      
      // get the drag image and offset
      Image dragImage = null;
      Point dragImageOffset = null;
      
      if(mDragImageProvider != null)
      {
         dragImage = mDragImageProvider.getDragImage();
         dragImageOffset = mDragImageProvider.getDragImageOffset();
      }
      
      // start the drag
      dge.startDrag(null, dragImage, dragImageOffset,
         transferable, mDragSourceListener);
   }
}
