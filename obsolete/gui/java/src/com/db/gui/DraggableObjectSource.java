/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.datatransfer.Transferable;
import java.awt.dnd.DragGestureEvent;
import java.awt.dnd.DragGestureListener;
import java.awt.dnd.DragSource;
import java.awt.dnd.DragSourceDragEvent;
import java.awt.dnd.DragSourceDropEvent;
import java.awt.dnd.DragSourceEvent;
import java.awt.dnd.DragSourceListener;
import java.awt.geom.AffineTransform;

import javax.swing.JComponent;
import javax.swing.SwingUtilities;

/**
 * A DraggableObjectSource is a source for draggable Objects. It allows
 * a gui component to provide local JVM Objects to be dragged from it
 * with ease.
 * 
 * @author Dave Longley
 */
public class DraggableObjectSource
implements DragGestureListener, DragSourceListener
{
   /**
    * The default drag source.
    */
   protected DefaultDragSource mDragSource;
   
   /**
    * The DraggableObjectPool that provides the draggable objects.
    */
   protected DraggableObjectPool mDraggableObjectPool;
   
   /**
    * The drag image provider. This object provides the drag image
    * and drag image offset for dragging.
    */
   protected DragImageProvider mDragImageProvider;   
   
   /**
    * The previous cursor location for a drag. Used to repaint the area of the
    * component under the drag image. 
    */
   protected Point mPreviousLocation;
   
   /**
    * Creates a new DraggableObjectSource.
    * 
    * @param component the component this drag source is for.
    * @param action the permitted drag 'n drop action:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    *
    * @param draggableObjectPool the object that provides a
    *                            draggable object when a drag begins.
    */
   public DraggableObjectSource(
      Component component, int action,
      DraggableObjectPool draggableObjectPool)
   {
      this(component, action, draggableObjectPool, null);
   }
   
   /**
    * Creates a new DraggableObjectSource.
    * 
    * @param component the component this drag source is for.
    * @param action the permitted drag 'n drop action:
    * 
    * DnDConstants.ACTION_NONE,
    * DnDConstants.ACTION_COPY,
    * DnDConstants.ACTION_MOVE,
    * DnDConstants.ACTION_COPY_OR_MOVE
    *
    * @param draggableObjectPool the object that provides a
    *                            draggable object when a drag begins.
    * @param dragImageProvider the object that provides the drag image
    *                          (can be null).
    */
   public DraggableObjectSource(
      Component component, int action,
      DraggableObjectPool draggableObjectPool,
      DragImageProvider dragImageProvider)
   {
      // create the drag source
      mDragSource = new DefaultDragSource(component, action, this);
      
      // store the draggable object pool
      mDraggableObjectPool = draggableObjectPool;
      
      // store the drag image provider
      mDragImageProvider = dragImageProvider;
      
      // previous location for a drag is 0,0 to start
      mPreviousLocation = new Point();
   }
   
   /**
    * Gets the top parent of the passed component.
    * 
    * @param component the component to get the top parent of.
    * 
    * @return the top parent of the passed component.
    */
   protected Component getTopParent(Component component)
   {
      Component rval = null;
      
      if(component.getParent() != null)
      {
         rval = getTopParent(component.getParent());
      }
      else
      {
         rval = component;
      }
      
      return rval;
   }
   
   /**
    * Redraws the passed component under a drag image where necessary.
    * 
    * @param component the component to paint under a drag image.
    * @param image the drag image.
    * @param offset the drag image offset.
    * @param location the location of the drag cursor.
    */
   protected void paintComponentUnderDragImage(
      Component component, Image image, Point offset, Point location)
   {
      // get component graphics
      Graphics2D g2 = (Graphics2D)component.getGraphics(); 
      
      // paint the parent under the previous location
      if(component instanceof JComponent)
      {
         JComponent c = (JComponent)component;
         
         // get the old rectangle
         Rectangle oldRect = new Rectangle(
            mPreviousLocation.x + offset.x, mPreviousLocation.y + offset.y,
            image.getWidth(null), image.getHeight(null));
         
         // get the new rectangle
         Rectangle newRect = new Rectangle(
            location.x + offset.x, location.y + offset.y,
            image.getWidth(null), image.getHeight(null));
         
         // get the intersect rectangle
         Rectangle intersectRect = oldRect.intersection(newRect);
         
         // build 2 rectangles to paint
         Rectangle topRect = new Rectangle(
            oldRect.x, oldRect.y, oldRect.width,
            oldRect.height - intersectRect.height);
         Rectangle bottomRect = new Rectangle(
            oldRect.x, oldRect.y + intersectRect.height,
            oldRect.width, oldRect.height);

         if(oldRect.y < newRect.y)
         {
            bottomRect.width -= intersectRect.width;

            if(newRect.x <= oldRect.x)
            {
               bottomRect.x += intersectRect.width;
            }
         }
         else
         {
            topRect.width -= intersectRect.width;

            if(newRect.x <= oldRect.x)
            {
               topRect.x += intersectRect.width;
            }
         }
         
         // paint rectangles
         c.paintImmediately(topRect);
         c.paintImmediately(bottomRect);
      }
      else
      {
         // paint the whole parent (no other option)
         component.paint(g2);
      }
   }
   
   /**
    * Called as the cursor's hotspot enters a platform-dependent drop site.
    * This method is invoked when all the following conditions are true:
    * <UL>
    * <LI>The cursor's hotspot enters the operable part of a platform- 
    * dependent drop site.
    * <LI>The drop site is active.
    * <LI>The drop site accepts the drag.
    * </UL>
    * 
    * @param dsde the <code>DragSourceDragEvent</code>
    */
   public void dragEnter(DragSourceDragEvent dsde)
   {
      // do nothing
   }

   /**
    * Called as the cursor's hotspot moves over a platform-dependent drop site.
    * This method is invoked when all the following conditions are true:
    * <UL>
    * <LI>The cursor's hotspot has moved, but still intersects the
    * operable part of the drop site associated with the previous
    * dragEnter() invocation. 
    * <LI>The drop site is still active.
    * <LI>The drop site accepts the drag.
    * </UL>
    * 
    * @param dsde the <code>DragSourceDragEvent</code>
    */
   public void dragOver(DragSourceDragEvent dsde)
   {
      // draw the drag image if it is not automatically supported
      if(!DragSource.isDragImageSupported() && mDragImageProvider != null)
      {
         // get the cursor location
         Point location = dsde.getLocation();
         
         // get the top parent of the component
         Component parent = getTopParent(getComponent());
         
         // convert screen coordinates to component coordinates
         SwingUtilities.convertPointFromScreen(location, parent);

         // see if the previous location has changed
         if(!mPreviousLocation.equals(location))
         {
            // get the transferable
            Transferable transferable = dsde.getDragSourceContext().
               getTransferable();
            
            // wrap the transferable
            TransferableWrapper wrapper = new TransferableWrapper(transferable);
            
            // get the object being dragged
            Object obj = wrapper.getObject();
            
            // get the drag image from the provider
            Image image = mDragImageProvider.
               getDragImage(obj, dsde.getDropAction(), getComponent());
            
            // get the drag image offset from the provider
            Point offset = mDragImageProvider.
               getDragImageOffset(obj, dsde.getDropAction(), getComponent());
            
            // ensure the drag image and offset are not null
            if(image != null && offset != null)
            {
               // get the graphics for the parent
               Graphics2D g2 = (Graphics2D)parent.getGraphics();
               
               // paint parent under drag image
               paintComponentUnderDragImage(parent, image, offset, location);               
               
               // get the image position
               int x = location.x + offset.x;
               int y = location.y + offset.y;
               
               // get the translation transform
               AffineTransform transform =
                  AffineTransform.getTranslateInstance(x, y);
               
               // draw the image
               g2.drawImage(image, transform, null);
            }
         
            // store previous location
            mPreviousLocation = location;
         }
      }
   }

   /**
    * Called when the user has modified the drop gesture.
    * This method is invoked when the state of the input
    * device(s) that the user is interacting with changes.
    * Such devices are typically the mouse buttons or keyboard
    * modifiers that the user is interacting with.
    * 
    * @param dsde the <code>DragSourceDragEvent</code>
    */
   public void dropActionChanged(DragSourceDragEvent dsde)
   {
      // do nothing
   }

   /**
    * Called as the cursor's hotspot exits a platform-dependent drop site.
    * This method is invoked when any of the following conditions are true:
    * <UL>
    * <LI>The cursor's hotspot no longer intersects the operable part
    * of the drop site associated with the previous dragEnter() invocation.
    * </UL>
    * OR
    * <UL>
    * <LI>The drop site associated with the previous dragEnter() invocation
    * is no longer active.
    * </UL>
    * OR
    * <UL>
    * <LI> The drop site associated with the previous dragEnter() invocation
    * has rejected the drag.
    * </UL>
    * 
    * @param dse the <code>DragSourceEvent</code>
    */
   public void dragExit(DragSourceEvent dse)
   {
      // handle the drag image if it is not automatically supported
      if(!DragSource.isDragImageSupported() && mDragImageProvider != null)
      {
         // get the top parent
         Component parent = getTopParent(getComponent());         
         
         // paint the parent
         if(parent instanceof JComponent)
         {
            JComponent component = (JComponent)parent;
            component.paintImmediately(
               0, 0, component.getWidth(), component.getHeight());
         }
         else
         {
            // get the graphics for the component
            Graphics2D g2 = (Graphics2D)parent.getGraphics();
            
            // paint the whole component (no other option)
            parent.paint(g2);
         }
      }
   }

   /**
    * This method is invoked to signify that the Drag and Drop
    * operation is complete. The getDropSuccess() method of
    * the <code>DragSourceDropEvent</code> can be used to 
    * determine the termination state. The getDropAction() method
    * returns the operation that the drop site selected 
    * to apply to the Drop operation. Once this method is complete, the
    * current <code>DragSourceContext</code> and
    * associated resources become invalid.
    * 
    * @param dsde the <code>DragSourceDropEvent</code>
    */
   public void dragDropEnd(DragSourceDropEvent dsde)
   {
      // if the drop is successful, proceed
      if(dsde.getDropSuccess())
      {
         // get the transferable
         Transferable transferable =
            dsde.getDragSourceContext().getTransferable();
         
         // wrap the transferable
         TransferableWrapper wrapper = new TransferableWrapper(transferable);
         
         // notify pool of dropped object
         mDraggableObjectPool.draggableObjectDropped(
            wrapper.getObject(), dsde.getDropAction(), getComponent());
         
         // handle the drag image if it is not automatically supported
         if(!DragSource.isDragImageSupported() && mDragImageProvider != null)
         {
            // get the top parent
            Component parent = getTopParent(getComponent());         
            
            // paint the parent
            if(parent instanceof JComponent)
            {
               JComponent component = (JComponent)parent;
               component.paintImmediately(
                  0, 0, component.getWidth(), component.getHeight());
            }
            else
            {
               // get the graphics for the component
               Graphics2D g2 = (Graphics2D)parent.getGraphics();
               
               // paint the whole component (no other option)
               parent.paint(g2);
            }
         }
      }
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
      // get the draggable object from the pool
      Object obj = mDraggableObjectPool.getDraggableObject(getComponent());
      
      // make sure the object isn't null
      if(obj != null)
      {
         // create a TransferableWrapper
         TransferableWrapper wrapper = new TransferableWrapper(obj);
         
         // get the drag image and offset
         Image dragImage = null;
         Point dragImageOffset = null;
         
         if(mDragImageProvider != null)
         {
            // get the drag image from the provider
            dragImage = mDragImageProvider.
               getDragImage(obj, dge.getDragAction(), getComponent());
            
            // get the drag image offset from the provider
            dragImageOffset = mDragImageProvider.
               getDragImageOffset(obj, dge.getDragAction(), getComponent());
         }
         
         // start the drag
         dge.startDrag(null, dragImage, dragImageOffset, wrapper, this);
      }
   }
   
   /**
    * Gets the component this draggable object source is for.
    * 
    * @return the component this draggable object source is for.
    */
   public Component getComponent()
   {
      return mDragSource.getComponent();
   }
}
