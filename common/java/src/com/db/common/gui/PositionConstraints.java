/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.gui;

import java.awt.Dimension;
import java.awt.FontMetrics;
import java.awt.Point;
import java.awt.Rectangle;

import javax.swing.JTextArea;

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
   public static final int ANCHOR_LEFT = 0x01;
   
   /**
    * Anchor right type.
    */
   public static final int ANCHOR_RIGHT = 0x02;
   
   /**
    * Anchor top type.
    */
   public static final int ANCHOR_TOP = 0x04;

   /**
    * Anchor bottom type.
    */
   public static final int ANCHOR_BOTTOM = 0x08;
   
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
   
   /**
    * Creates a string representation of this constraints object.
    * 
    * The constraints bounds and anchoring are printed to a string.
    * 
    * @return a string representation of this constraints object.
    */
   public String toString()
   {
      StringBuffer sb = new StringBuffer();
      
      // class name
      sb.append(getClass().getName());
      sb.append('[');
      
      // location
      sb.append("x=");
      sb.append(location.x);
      sb.append(',');
      sb.append("y=");
      sb.append(location.y);
      sb.append(',');
      
      // size
      sb.append("width=");
      sb.append(size.width);
      sb.append(',');
      sb.append("height=");
      sb.append(size.height);
      sb.append(',');
      
      // anchor
      sb.append("anchor=[");
      
      if((anchor & ANCHOR_ALL) == ANCHOR_ALL)
      {
         sb.append("all");
      }
      else
      {
         boolean putComma = false;
         if((anchor & ANCHOR_TOP) == ANCHOR_TOP)
         {
            sb.append("top");
            putComma = true;
         }
         
         if((anchor & ANCHOR_LEFT) == ANCHOR_LEFT)
         {
            if(putComma)
            {
               sb.append(',');
            }
            
            sb.append("left");
            putComma = true;
         }

         if((anchor & ANCHOR_BOTTOM) == ANCHOR_BOTTOM)
         {
            if(putComma)
            {
               sb.append(',');
            }
            
            sb.append("bottom");
            putComma = true;
         }

         if((anchor & ANCHOR_RIGHT) == ANCHOR_RIGHT)
         {
            if(putComma)
            {
               sb.append(',');
            }
            
            sb.append("right");
         }
      }
      
      sb.append(']');
      
      // end
      sb.append(']');
      
      return sb.toString();
   }
   
   /**
    * Calculates the preferred height of a text area based on some 
    * width.
    * 
    * @param textArea the JTextArea to calculate the height of.
    * @param width the width for the text area.
    * 
    * @return the preferred height. 
    */
   public static int getTextAreaPreferredHeight(JTextArea textArea, int width)
   {
      int rval = 0;
      
      // get the font metrics, measure the row height
      FontMetrics metrics = textArea.getFontMetrics(textArea.getFont());
      int rowHeight = metrics.getHeight();
      
      try
      {
         // store the line overflow for each line in the text area
         double overflow = 0;
         int lines = textArea.getLineCount();
         for(int i = 0; i < lines; i++)
         {
            int startOffset = textArea.getLineStartOffset(i);
            int endOffset = textArea.getLineEndOffset(i);
            
            String line =
               textArea.getText(startOffset, endOffset - startOffset);
            int lineWidth = metrics.stringWidth(line);
            
            overflow += Math.max(0, lineWidth - width);
         }
         
         // determine the number of extra lines
         int extraLines = (int)Math.round(overflow / width);
         
         // determine the total height
         rval = rowHeight * (lines + extraLines + 1);
      }
      catch(Throwable ignore)
      {
      }
      
      return rval;
   }   
}
