/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;
import java.awt.datatransfer.Transferable;
import java.awt.dnd.DropTarget;
import java.awt.dnd.DropTargetDragEvent;
import java.awt.dnd.DropTargetDropEvent;
import java.awt.dnd.DropTargetEvent;
import java.awt.dnd.DropTargetListener;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A DraggableObjectDestination is a drop target that accepts draggable
 * Objects. It allows a gui component to receive local JVM Objects that
 * have been dragged from another gui component. 
 * 
 * @author Dave Longley
 */
public class DraggableObjectDestination implements DropTargetListener
{
   /**
    * The drop target. This object allows dropping into a component.
    */
   protected DropTarget mDropTarget;
   
   /**
    * The draggable object acceptor that will accept draggable objects.
    */
   protected DraggableObjectAcceptor mAcceptor;
   
   /**
    * Creates a new DraggableObjectDestination.
    * 
    * @param component the component this drop target is for.
    * @param action the permitted drag 'n drop action:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    * 
    * @param acceptor the object that can accept the draggable object when
    *                 it is dropped.
    */
   public DraggableObjectDestination(
      Component component, int action, DraggableObjectAcceptor acceptor) 
   {
      // create the drop target
      mDropTarget = new DropTarget(component, action, this);
      
      // set the component's drop target
      component.setDropTarget(mDropTarget);
      
      // store the acceptor
      mAcceptor = acceptor;
   }
   
   /**
    * Called when a drag enters the drop area.
    *
    * @param dtde the event that represents the drag.
    */
   public void dragEnter(DropTargetDragEvent dtde)
   {
      // get a transferable wrapper
      TransferableWrapper wrapper = new TransferableWrapper(
         dtde.getTransferable());
      
      // get the draggable object
      Object obj = wrapper.getObject();         
      
      // ensure there is an object to transfer
      if(obj != null)
      {
         // get the drop action
         int action = dtde.getDropAction();
         
         // get the destination component
         Component destination = mDropTarget.getComponent();
         
         // determine whether or not the object can be accepted
         if(mAcceptor.canAcceptDraggableObject(obj, action, destination))      
         {
            // accept the drag
            dtde.acceptDrag(dtde.getDropAction());
         }
         else
         {
            // reject the drag
            dtde.rejectDrag();
         }
      }
      else
      {
         // reject the drag
         dtde.rejectDrag();
      }
   }

   /**
    * Called when a drag exits the drop area. 
    *
    * @param dte the event that represents the drag.
    */
   public void dragExit(DropTargetEvent dte)
   {
      // do nothing
   }

   /**
    * Called when a drag is over the drop area. 
    *
    * @param dtde the event that represents the drag.
    */
   public void dragOver(DropTargetDragEvent dtde)
   {
      // do nothing
   }

   /**
    * Called when a drop occurs.
    *
    * @param dtde the event that represents the drop.
    */
   public synchronized void drop(DropTargetDropEvent dtde)
   {
      try
      {
         // get the transferable interface to retrieve the data
         Transferable transferable = dtde.getTransferable();
         
         // get a transferable wrapper
         TransferableWrapper wrapper = new TransferableWrapper(transferable);
         
         // get the draggable object
         Object obj = wrapper.getObject();         
         
         // ensure there is an object to transfer
         if(obj != null)
         {
            // get the drop action
            int action = dtde.getDropAction();
            
            // get the destination component
            Component destination = mDropTarget.getComponent();
            
            // determine whether or not the object can be accepted
            if(mAcceptor.canAcceptDraggableObject(obj, action, destination))
            {
               // accept the drop
               dtde.acceptDrop(action);
               
               try
               {
                  // accept the object
                  mAcceptor.acceptDraggableObject(obj, action, destination);
               }
               catch(Throwable t)
               {
                  getLogger().error(getClass(),
                     "An exception occurred while accepting a draggable " +
                     "object!,exception= " + t);
                  getLogger().debug(getClass(), Logger.getStackTrace(t));
               }
               
               // drop is complete
               dtde.dropComplete(true);
            }
            else
            {
               // object cannot be transferred, so reject drop 
               dtde.rejectDrop();
            }
         }
         else
         {
            // no object to transfer, reject drop 
            dtde.rejectDrop();
         }
      }
      catch(Throwable t)
      {
         // reject drop
         dtde.rejectDrop();

         // if there's a problem, reject the drop
         getLogger().error(getClass(),
            "Exception thrown during drag 'n drop!,exception= " + t);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
   }

   /**
    * Called when a drop action is changed. 
    *
    * @param dtde the event that represents the drop.
    */
   public void dropActionChanged(DropTargetDragEvent dtde)
   {
      // do nothing
   }
   
   /**
    * Gets the component that can receive dropped draggable objects.
    * 
    * @return the draggable object destination component.
    */
   public Component getComponent()
   {
      return mDropTarget.getComponent();
   }
   
   /**
    * Gets the logger for this object.
    *
    * @return the logger for this object.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbgui");
   }
}
