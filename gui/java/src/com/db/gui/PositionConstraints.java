/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

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
    * Anchor none type.
    */
   public static final int ANCHOR_NONE = 0x00;
   
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
    * Anchors top and left.
    */
   public static final int ANCHOR_TOP_LEFT =
      ANCHOR_TOP | ANCHOR_LEFT;
   
   /**
    * Anchors top and right.
    */
   public static final int ANCHOR_TOP_RIGHT =
      ANCHOR_TOP | ANCHOR_RIGHT;
   
   /**
    * Anchors top, left, and right.
    */
   public static final int ANCHOR_TOP_LEFT_RIGHT =
      ANCHOR_TOP | ANCHOR_LEFT | ANCHOR_RIGHT;

   /**
    * Anchors in all directions.
    */
   public static final int ANCHOR_ALL =
      ANCHOR_LEFT | ANCHOR_RIGHT | ANCHOR_TOP | ANCHOR_BOTTOM;
   
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
    * can be anchored.
    * 
    * When one of the component's sides is anchored, it means
    * that when its parent component is resized, the distance between the
    * anchored side(s) and the parent component's edge does not change --
    * resulting in the resizing of the component.
    * 
    * For an inverted anchor:
    * 
    * When one of the component's sides is anchored, it means
    * that when its parent component is resized, the distance between the
    * anchored side(s) and the parent component's edge will be added to
    * the component's position, resulting in the repositioning of
    * the component.
    */
   public int anchor;
   
   /**
    * The invert anchor constraint. This constraint indicates how the
    * component ought to be invert-anchored. A component's top, left, right,
    * and/or bottom can be anchored.
    * 
    * When one of the component's sides is invert-anchored, it means
    * that when its parent component is resized, the distance between the
    * invert-anchored side(s) and the parent component's edge will be added to
    * the component's position, resulting in the repositioning of
    * the component.
    * 
    * An invertAnchor works the "opposite" way an anchor does -- by effectively
    * resizing the areas around a component rather than resizing the
    * component itself. To "resize the areas around the component" it moves
    * the component in a proportional way so that the area around the component
    * increases, but the component appears to remain in the same
    * relative position.
    */
   public int invertAnchor;
   
   /**
    * Creates new position constraints.
    */
   public PositionConstraints()
   {
      location = new Point(0, 0);
      size = new Dimension(0, 0);
      anchor = ANCHOR_LEFT | ANCHOR_TOP;
      invertAnchor = ANCHOR_NONE;
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
      clone.invertAnchor = invertAnchor;
      
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
      
      if(anchor == ANCHOR_NONE)
      {
         sb.append("none");
      }
      else if((anchor & ANCHOR_ALL) == ANCHOR_ALL)
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
      sb.append(',');
      
      // inverted anchor
      sb.append("invertAnchor=[");
      
      if(invertAnchor == ANCHOR_NONE)
      {
         sb.append("none");
      }
      else if((anchor & ANCHOR_ALL) == ANCHOR_ALL)
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
      
      // subtract text area insets of 4
      width -= 4;

      try
      {
         if(textArea.getWrapStyleWord())
         {
            // store the line overflow for each line in the text area
            StringBuffer sb = new StringBuffer();
            int lines = textArea.getLineCount();
            int rows = 0;
            for(int i = 0; i < lines; i++)
            {
               int startOffset = textArea.getLineStartOffset(i);
               int endOffset = textArea.getLineEndOffset(i);
               
               // get the line
               String line = textArea.getText(
                  startOffset, endOffset - startOffset);
               
               // clear string buffer
               sb.setLength(0);
               
               // get words
               String[] words = line.split(" ");

               // add words until the line doesn't fit any longer
               for(int n = 0; n < words.length; n++)
               {
                  // add space as necessary
                  if(sb.length() != 0)
                  {
                     sb.append(" ");
                  }
                  
                  // add next word
                  sb.append(words[n]);
                  
                  // measure string
                  int lineWidth = metrics.stringWidth(sb.toString());
                  
                  // if the line meets or goes over the width, increment
                  // rows and set buffer to current word
                  if(lineWidth >= width)
                  {
                     // determine overflow
                     int overflow = lineWidth / width;
                     
                     rows += overflow;
                     sb.setLength(0);
                     sb.append(words[n]);
                  }
                  
                  if(n == (words.length - 1))
                  {
                     // increment rows
                     rows++;
                  }
               }
            }
            
            // determine the total height
            rval = rowHeight * rows + metrics.getDescent();
         }
         else
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
            rval = rowHeight * (lines + extraLines) + metrics.getDescent();
         }
      }
      catch(Throwable ignore)
      {
      }
      
      return rval;
   }   
}
