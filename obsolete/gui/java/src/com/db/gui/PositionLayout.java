/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.LayoutManager2;
import java.awt.Rectangle;

import java.util.HashMap;
import java.util.Iterator;

/**
 * A position layout is a layout that uses relative positions to layout
 * components inside of a container. 
 * 
 * @author Dave Longley
 */
public class PositionLayout implements LayoutManager2
{
   /**
    * The original bounds of the container this layout is for.  
    */
   protected Rectangle mContainerBounds;
   
   /**
    * A mapping of component to constraints.
    */
   protected HashMap<Component, PositionConstraints> mConstraintsMap;

   /**
    * Creates a new position layout.
    * 
    * @param container the container to work with.
    * @param width the width to work with.
    * @param height the height to work with.
    */
   public PositionLayout(Container container, int width, int height)
   {
      this(container, new Rectangle(0, 0, width, height));
   }
   
   /**
    * Creates a new position layout.
    * 
    * @param container the container to work with.
    * @param bounds the bounds to work in.
    */
   public PositionLayout(Container container, Rectangle bounds)
   {
      // set bounds for the container this layout is for
      mContainerBounds = null;
      if(bounds != null)
      {
         // store container bounds
         mContainerBounds = bounds;
         
         if(container != null)
         {
            // set container bounds
            container.setBounds(bounds);
         }
      }
      
      // create the constraints map
      mConstraintsMap = new HashMap<Component, PositionConstraints>();
   }
   
   /**
    * Lays out the passed component inside of the passed container.
    * 
    * @param comp the component to layout.
    * @param container the container to lay the component out in.
    */
   protected void layoutComponent(Component comp, Container container)
   {
      // get the contraints for the component
      PositionConstraints constraints = getConstraints(comp);
      
      // get the insets for the container
      Insets insets = container.getInsets();
      if(insets == null)
      {
         insets = new Insets(0, 0, 0, 0);
      }
      
      // get container bounds, take into account insets
      Rectangle containerBounds = new Rectangle(mContainerBounds);
      containerBounds.x += insets.left;
      containerBounds.y += insets.top;
      containerBounds.width -= insets.right;
      containerBounds.height -= insets.bottom;
      
      // if constraints indicate horizontal filling, update them
      if(constraints.size.width == PositionConstraints.HORIZONTAL_FILL)
      {
         constraints.size.width =
            mContainerBounds.width - constraints.location.x;
      }
      
      // if constraints indicate vertical filling, update them
      if(constraints.size.height == PositionConstraints.VERTICAL_FILL)
      {
         constraints.size.height =
            mContainerBounds.height - constraints.location.y;
      }
      
      // get rectangle for updating constraints
      Rectangle rect = new Rectangle(
         constraints.location.x,
         constraints.location.y,
         constraints.size.width, constraints.size.height);
      
      // determine position changes
      int deltaX = containerBounds.x;
      int deltaY = containerBounds.y;
      
      // determine width resize changes
      int deltaWidth = container.getWidth() - mContainerBounds.width;
      deltaWidth -= insets.right;
      
      // determine height resize changes
      int deltaHeight = container.getHeight() - mContainerBounds.height;
      deltaHeight -= insets.bottom;
      
      // handle left/right anchoring
      if((constraints.anchor & PositionConstraints.ANCHOR_LEFT) != 0)
      {
         // if right anchoring is turned on as well, then resize
         if((constraints.anchor & PositionConstraints.ANCHOR_RIGHT) != 0)
         {
            // resize and move component
            rect.width += (deltaWidth - deltaX);
         }
      }
      else if((constraints.anchor & PositionConstraints.ANCHOR_RIGHT) != 0)
      {
         // move component according to width and position
         rect.x += deltaWidth - deltaX;
      }
      else
      {
         // move component according to position
         rect.x += deltaX;
      }

      // handle top/bottom anchoring
      if((constraints.anchor & PositionConstraints.ANCHOR_TOP) != 0)
      {
         // if bottom anchoring is turned on as well, then resize
         if((constraints.anchor & PositionConstraints.ANCHOR_BOTTOM) != 0)
         {
            // resize and move component
            rect.height += (deltaHeight - deltaY);
         }
      }
      else if((constraints.anchor & PositionConstraints.ANCHOR_BOTTOM) != 0)
      {
         // move component according to height and position
         rect.y += deltaHeight - deltaY;
      }
      else
      {
         // move component according to position
         rect.y += deltaY;
      }
      
      // handle left/right invert anchoring
      if((constraints.invertAnchor & PositionConstraints.ANCHOR_LEFT) != 0)
      {
         // if right anchoring is turned on as well, then resize
         if((constraints.invertAnchor &
             PositionConstraints.ANCHOR_RIGHT) != 0)
         {
            // proportions the same to the left and right of the component
            rect.x += Math.round((deltaWidth + deltaX) / 2.0D);
         }
      }
      else if((constraints.invertAnchor &
               PositionConstraints.ANCHOR_RIGHT) != 0)
      {
         // move component according to width and position
         rect.x += deltaWidth + deltaX;
      }
      else
      {
         // move component according to position
         rect.x += deltaX;
      }

      // handle top/bottom anchoring
      if((constraints.invertAnchor & PositionConstraints.ANCHOR_TOP) != 0)
      {
         // if bottom anchoring is turned on as well, then resize
         if((constraints.invertAnchor &
             PositionConstraints.ANCHOR_BOTTOM) != 0)
         {
            // proportions the same to the top and bottom of the component
            rect.y += Math.round((deltaHeight + deltaY) / 2.0D);
         }
      }
      else if((constraints.invertAnchor &
               PositionConstraints.ANCHOR_BOTTOM) != 0)
      {
         // move component according to height and position
         rect.y += deltaHeight + deltaY;
      }
      else
      {
         // move component according to position
         rect.y += deltaY;
      }
      
      // ensure that bounds rectangle isn't smaller than minimum size
      rect.width = Math.max(rect.width, comp.getMinimumSize().width);
      rect.height = Math.max(rect.height, comp.getMinimumSize().height);
      
      // ensure that bounds rectangle isn't larger than maximum size
      //rect.width = Math.min(rect.width, comp.getMaximumSize().width);
      //rect.height = Math.min(rect.height, comp.getMaximumSize().height);
      
      // set component bounds
      comp.setBounds(rect);
   }
   
   /**
    * Lays out all components inside of the passed target container.
    * 
    * @param target the container to layout components inside of.
    */
   public void layoutComponents(Container target)
   {
      // iterate through all components
      Iterator i = mConstraintsMap.keySet().iterator();
      while(i.hasNext())
      {
         Component comp = (Component)i.next();
         
         // layout component
         layoutComponent(comp, target);
      }      
   }
   
   /**
    * Sets the constraints for a component. Null constraints are not permitted. 
    * 
    * @param comp the component to set the constraints for.
    * @param constraints the constraints to set.
    */
   public void setConstraints(Component comp, PositionConstraints constraints)
   {
      if(constraints == null)
      {
         // create default constraints
         constraints = new PositionConstraints();
      }
      
      if(constraints == null)
      {
         // null constraints are not permitted
         IllegalArgumentException e = 
            new IllegalArgumentException("Null constraints are not permitted.");
         throw e;
      }
      
      // add contraints to the map
      mConstraintsMap.put(comp, constraints);
   }
   
   /**
    * Gets the constraints for a component.
    * 
    * @param comp the component to get the constraints for.
    * @return the constraints for a component.
    */
   public PositionConstraints getConstraints(Component comp)
   {
      return mConstraintsMap.get(comp);
   }
   
   /**
    * This method is not permitted by PositionLayout. Constraints must be
    * provided.
    * 
    * If the layout manager uses a per-component string, adds the
    * component <code>comp</code> to the layout, associating it 
    * with the string specified by <code>name</code>.
    * 
    * @param name the string to be associated with the component.
    * @param comp the component to be added.
    */
   public void addLayoutComponent(String name, Component comp)
   {
      // set constraints for the component which will throw an exception
      setConstraints(comp, null);
   }
   
   /**
    * Adds the specified component to the layout, using the specified
    * constraint object.
    * 
    * @param comp the component to be added.
    * @param constraints where/how the component is added to the layout.
    */
   public void addLayoutComponent(Component comp, Object constraints)
   {
      if(constraints == null)
      {
         // create default constraints
         constraints = new PositionConstraints();
      }
      
      if(!(constraints instanceof PositionConstraints))
      {
         // non-PositionConstraints are not permitted
         IllegalArgumentException e = 
            new IllegalArgumentException("Constraints must be of type " +
                                         "PositionConstraints.");
         throw e;
      }
      
      // set constraints for the component
      setConstraints(comp, (PositionConstraints)constraints);
   }

   /**
    * Removes the specified component from the layout.
    * 
    * @param comp the component to be removed.
    */
   public void removeLayoutComponent(Component comp)
   {
      // remove component from the constraints map
      mConstraintsMap.remove(comp);
   }

   /**
    * Calculates the preferred size dimensions for the specified 
    * container, given the components it contains.
    * 
    * @param target the container to be laid out.
    */
   public Dimension preferredLayoutSize(Container target)
   {
      Dimension size = new Dimension(0, 0);
      
      // layout container
      layoutContainer(target);
      
      // iterate through components looking for the largest width and height
      Iterator i = mConstraintsMap.values().iterator();
      while(i.hasNext())
      {
         PositionConstraints pc = (PositionConstraints)i.next();
         
         if(pc.getRight() > size.width)
         {
            size.width = pc.getRight();
         }
         
         if(pc.getBottom() > size.height)
         {
            size.height = pc.getBottom();
         }
      }
      
      // add insets for container
      Insets insets = target.getInsets();
      if(insets != null)
      {
         size.width += (insets.left + insets.right);
         size.height += (insets.top + insets.bottom);
      }
      
      return size;
   }

   /** 
    * Calculates the minimum size dimensions for the specified 
    * container, given the components it contains.
    * 
    * @param target the container to be laid out.
    */
   public Dimension minimumLayoutSize(Container target)
   {
      // use minimum size
      return new Dimension(0, 0);
   }
   
   /** 
    * Calculates the maximum size dimensions for the specified container,
    * given the components it contains.
    * 
    * @param target the container to determine the dimensions for.
    * @return the maximum size dimensions for the target container.
    */
   public Dimension maximumLayoutSize(Container target)
   {
      // use the maximum size
      return new Dimension(Integer.MAX_VALUE, Integer.MAX_VALUE);
   }
   
   /**
    * Returns the alignment along the x axis. This specifies how
    * the component would like to be aligned relative to other 
    * components. The value should be a number between 0 and 1
    * where 0 represents alignment along the origin, 1 is aligned
    * the furthest away from the origin, 0.5 is centered, etc.
    * 
    * @param target the container to determine alignment for.
    * @return the alignment along the x axis for the target container.
    */
   public float getLayoutAlignmentX(Container target)
   {
      // prefer center alignment
      return 0.5F;
   }

   /**
    * Returns the alignment along the y axis. This specifies how
    * the component would like to be aligned relative to other 
    * components. The value should be a number between 0 and 1
    * where 0 represents alignment along the origin, 1 is aligned
    * the furthest away from the origin, 0.5 is centered, etc.
    * 
    * @param target the container to determine alignment for.
    * @return the alignment along the y axis for the target container.
    */
   public float getLayoutAlignmentY(Container target)
   {
      // prefer center alignment
      return 0.5F;
   }
   
   /**
    * Invalidates the layout, indicating that if the layout manager
    * has cached information it should be discarded.
    * 
    * @param target the container to invalidate the layout for.
    */
   public void invalidateLayout(Container target)
   {
      // no information is cached, so nothing to do here
   }

   /** 
    * Lays out the specified container.
    * 
    * @param target the container to be laid out. 
    */
   public void layoutContainer(Container target)
   {
      // update stored container bounds if appropriate
      if(mContainerBounds == null)
      {
         mContainerBounds = target.getBounds();
      }
      
      // layout components
      layoutComponents(target);
   }
   
   /**
    * Sets the base parent boundaries to work off of.
    * 
    * @param bounds the base parent boundaries to work off of.
    */
   public void setBaseParentBounds(Rectangle bounds)
   {
      mContainerBounds = bounds;
   }
   
   /**
    * Gets the base parent boundaries to work off of.
    * 
    * @return the base parent boundaries to work off of.
    */
   public Rectangle getBaseParentBounds()
   {
      return mContainerBounds;
   }
}
