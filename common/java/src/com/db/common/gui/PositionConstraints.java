/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.gui;

import java.awt.Dimension;
import java.awt.Point;
import java.awt.Rectangle;

/**
 * Position constraints are used by a PositionLayout to determine
 * how to layout a component.
 * 
 * @author Dave Longley
 */
public class PositionConstraints implements Cloneable
{
   /**
    * Anchor left type.
    */
   public static final int ANCHOR_LEFT = 1;
   
   /**
    * Anchor right type.
    */
   public static final int ANCHOR_RIGHT = 2;
   
   /**
    * Anchor top type.
    */
   public static final int ANCHOR_TOP = 4;

   /**
    * Anchor bottom type.
    */
   public static final int ANCHOR_BOTTOM = 8;
   
   /**
    * Anchors in all directions.
    */
   public static final int ANCHOR_ALL =
      ANCHOR_LEFT | ANCHOR_RIGHT | ANCHOR_TOP | ANCHOR_BOTTOM;
   
   /**
    * Anchors top, left, and right.
    */
   public static final int ANCHOR_TOP_LEFT_RIGHT =
      ANCHOR_TOP | ANCHOR_LEFT | ANCHOR_RIGHT;
   
   /**
    * Indicates that a component should fill its parent container in a
    * horizontal direction. 
    */
   public static final int HORIZONTAL_FILL = -2;
   
   /**
    * Indicates that a component should fill its parent container in
    * a vertical direction. 
    */
   public static final int VERTICAL_FILL = -2;
   
   /**
    * The location constraint. This is the top left location for a component,
    * in pixels, relative to its parent's location. 
    */
   public Point location;
   
   /**
    * This size constraint. This is the starting size for a component,
    * in pixels. This size can change if the component has its anchor
    * constraint set.
    */
   public Dimension size;
   
   /**
    * The anchor constraint. This constraint indicates how the component
    * ought to be anchored. A component's top, left, right, and/or bottom
    * can be anchored. When one of the component's sides is anchored, it means
    * that when its parent component is resized, the distance between the
    * anchored side(s) and the parent component's edge does not change --
    * resulting in the resizing of the component.
    */
   public int anchor;
   
   /**
    * Creates new position constraints.
    */
   public PositionConstraints()
   {
      location = new Point(0, 0);
      size = new Dimension(0, 0);
      anchor = ANCHOR_LEFT | ANCHOR_TOP;
   }
   
   /**
    * Clones these constraints.
    * 
    * @return a copy of these constraints.
    */
   public Object clone()
   {
      PositionConstraints clone = new PositionConstraints();
      
      clone.location = new Point(location.x, location.y);
      clone.size = new Dimension(size.width, size.height);
      clone.anchor = anchor;
      
      return clone;
   }
   
   /**
    * Gets the left position for this constraints object.
    * 
    * @return the left position for this constraints object.
    */
   public int getLeft()
   {
      return location.x;
   }
   
   /**
    * Gets the right position for this constraints object.
    * 
    * @return the right position for this constraints object.
    */
   public int getRight()
   {
      return location.x + size.width;
   }
   
   /**
    * Gets the top position for this constraints object.
    * 
    * @return the top position for this constraints object.
    */
   public int getTop()
   {
      return location.y;
   }
   
   /**
    * Gets the bottom position for this constraints object.
    * 
    * @return the bottom position for this constraints object.
    */
   public int getBottom()
   {
      return location.y + size.height;
   }
   
   /**
    * Sets the bounds for this constraints object.
    * 
    * @param bounds the bounds for this constraints object.
    */
   public void setBounds(Rectangle bounds)
   {
      location = new Point(bounds.getLocation());
      size = new Dimension(bounds.getSize());
   }

   /**
    * Gets the bounds for this constraints object.
    * 
    * @return the bounds for this constraints object.
    */
   public Rectangle getBounds()
   {
      return new Rectangle(location, size);
   }
}
