/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;
import java.awt.datatransfer.Transferable;
import java.awt.dnd.DragSourceListener;

/**
 * A DraggableObjectSource is a source for draggable Objects.
 * 
 * @author Dave Longley
 */
public class DraggableObjectSource implements TransferableProvider
{
   /**
    * The default drag source.
    */
   protected DefaultDragSource mDragSource;
   
   /**
    * The DraggableObjectProvider that provides the draggable object.
    */
   protected DraggableObjectProvider mDraggableObjectProvider;
   
   /**
    * Creates a new DraggableObjectSource.
    * 
    * @param component the component this drag source is for.
    * @param dragAction the action to use when dragging:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    *
    * @param draggableObjectProvider the object that provides the
    *                                draggable object when a drag begins.
    */
   public DraggableObjectSource(
      Component component, int dragAction,
      DraggableObjectProvider draggableObjectProvider)   
   {
      this(component, dragAction, draggableObjectProvider, null, null);
   }
   
   /**
    * Creates a new DraggableObjectSource.
    * 
    * @param component the component this drag source is for.
    * @param dragAction the action to use when dragging:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    *
    * @param draggableObjectProvider the object that provides the
    *                                draggable object when a drag begins.
    * @param dragImageProvider the object that provides the drag image
    *                          (can be null).
    */
   public DraggableObjectSource(
      Component component, int dragAction,
      DraggableObjectProvider draggableObjectProvider,
      DragImageProvider dragImageProvider)
   {
      this(component, dragAction, draggableObjectProvider,
         dragImageProvider, null);
   }
   
   /**
    * Creates a new DraggableObjectSource.
    * 
    * @param component the component this drag source is for.
    * @param dragAction the action to use when dragging:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    *
    * @param draggableObjectProvider the object that provides the
    *                                draggable object when a drag begins.
    * @param dragImageProvider the object that provides the drag image
    *                          (can be null).
    * @param dragSourceListener the drag source listener (can be null).
    */
   public DraggableObjectSource(
      Component component, int dragAction,
      DraggableObjectProvider draggableObjectProvider,
      DragImageProvider dragImageProvider,
      DragSourceListener dragSourceListener)
   {
      // create the drag source
      mDragSource = new DefaultDragSource(
         component, dragAction, this, dragImageProvider, dragSourceListener);
      
      // store the draggable object provider
      mDraggableObjectProvider = draggableObjectProvider;
   }
   
   /**
    * Gets a Transferrable object for transfer. 
    * 
    * @param component the component to acquire the Transferrable from.
    * 
    * @return the Transferrable object.
    */
   public Transferable getTransferable(Component component)   
   {
      Transferable rval = null;
      
      // get the draggable object
      Object obj = mDraggableObjectProvider.getDraggableObject(component);
      
      // create ObjectTransferer
      rval = new ObjectTransferer(obj);
      
      return rval;
   }
}
