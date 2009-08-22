/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Color;
import java.awt.Graphics;

import javax.swing.UIManager;
import javax.swing.plaf.basic.BasicArrowButton;

/**
 * A BasicArrowButton that allows for a different arrow color.
 * 
 * @author Dave Longley
 */
public class ColoredBasicArrowButton extends BasicArrowButton
{
   /**
    * The shadow color.
    */
   protected Color mShadow;
   
   /**
    * The dark shadow color.
    */
   protected Color mDarkShadow;
   
   /**
    * The highlight color.
    */
   protected Color mHighlight;
   
   /**
    * The enabled button color.
    */
   protected Color mEnabled;
   
   /**
    * The disabled button color.
    */
   protected Color mDisabled;
   
   /**
    * Creates a new ColoredBasicArrowButton.
    * 
    * @param d the direction for the button.
    * @param enabled the enabled button color.
    */
   public ColoredBasicArrowButton(int d, Color enabled)
   {
      this(d,
         UIManager.getColor("control"),
         UIManager.getColor("controlShadow"),
         UIManager.getColor("controlDkShadow"),
         UIManager.getColor("controlLtHighlight"),
         enabled, UIManager.getColor("TabbedPane.arrowButtonDisabled"));
   }
   
   /**
    * Creates a new ColoredBasicArrowButton.
    * 
    * @param d the direction for the button.
    * @param enabled the enabled button color.
    * @param disabled the disabled button color.
    */
   public ColoredBasicArrowButton(int d, Color enabled, Color disabled)
   {
      this(d,
         UIManager.getColor("control"),
         UIManager.getColor("controlShadow"),
         UIManager.getColor("controlDkShadow"),
         UIManager.getColor("controlLtHighlight"),
         enabled, disabled);
   }
   
   /**
    * Creates a new ColoredBasicArrowButton.
    * 
    * @param d the direction for the button.
    * @param background the background color for the button.
    * @param shadow the shadow color for the button.
    * @param darkShadow the dark shadow color for the button.
    * @param highlight the highlight color for the button.
    */
   public ColoredBasicArrowButton(
      int d, Color background, Color shadow,
      Color darkShadow, Color highlight)
   {
      this(d, background, shadow, darkShadow, highlight,
         UIManager.getColor("TabbedPane.arrowButtonEnabled"),
         UIManager.getColor("TabbedPane.arrowButtonDisabled"));
   }
   
   /**
    * Creates a new ColoredBasicArrowButton.
    * 
    * @param d the direction for the button.
    * @param background the background color for the button.
    * @param shadow the shadow color for the button.
    * @param darkShadow the dark shadow color for the button.
    * @param highlight the highlight color for the button.
    * @param enabled the enabled button color.
    * @param disabled the disabled button color.
    */
   public ColoredBasicArrowButton(
      int d, Color background, Color shadow,
      Color darkShadow, Color highlight, Color enabled, Color disabled)
   {
      super(d, background, shadow, darkShadow, highlight);
      
      // store colors
      mShadow = shadow;
      mDarkShadow = darkShadow;
      mHighlight = highlight;
      mEnabled = enabled;
      mDisabled = disabled;
      
      if(mEnabled == null)
      {
         mEnabled = mDarkShadow;
      }
      
      if(mDisabled == null)
      {
         mDisabled = mShadow;
      }
   }
   
   /**
    * Paints the button triangle using the given button colors.
    * 
    * @param g the graphics.
    * @param x the x coordinate for the triangle.
    * @param y the y coordinate for the triangle.
    * @param size the size of the triangle.
    * @param d the direction the triangle is facing.
    * @param isEnabled whether or not the triangle is enabled.
    */
   @Override
   public void paintTriangle(
      Graphics g, int x, int y, int size, int d, boolean isEnabled)
   {
      // NOTE: The following code is taken from the base class
      // (BasicArrowButton) except for the colors used -- this was
      // only necessary because there is no way to modify the private
      // color members and/or no methods that expose them that could
      // be overwritten
      Color oldColor = g.getColor();
      int mid, i, j;
      
      j = 0;
      size = Math.max(size, 2);
      mid = (size / 2) - 1;
      g.translate(x, y);
      
      if(isEnabled)
      {
         g.setColor(mEnabled);
      }
      else
      {
         g.setColor(mDisabled);
      }
      
      switch(d)
      {
         case NORTH:
            for(i = 0; i < size; i++)
            {
               g.drawLine(mid-i, i, mid+i, i);
            }
            
            if(!isEnabled)
            {
               g.setColor(mHighlight);
               g.drawLine(mid-i+2, i, mid+i, i);
            }
            break;
         case SOUTH:
            if(!isEnabled)
            {
               g.translate(1, 1);
               g.setColor(mHighlight);
               for(i = size-1; i >= 0; i--)
               {
                  g.drawLine(mid-i, j, mid+i, j);
                  j++;
               }
               
               g.translate(-1, -1);
               g.setColor(mShadow);
            }
            
            j = 0;
            
            for(i = size-1; i >= 0; i--)
            {
               g.drawLine(mid-i, j, mid+i, j);
               j++;
            }
            break;
         case WEST:
            for(i = 0; i < size; i++)
            {
               g.drawLine(i, mid-i, i, mid+i);
            }
            if(!isEnabled)
            {
               g.setColor(mHighlight);
               g.drawLine(i, mid-i+2, i, mid+i);
            }
            break;
         case EAST:
            if(!isEnabled)
            {
               g.translate(1, 1);
               g.setColor(mHighlight);
               
               for(i = size-1; i >= 0; i--)
               {
                  g.drawLine(j, mid-i, j, mid+i);
                  j++;
               }
               
               g.translate(-1, -1);
               g.setColor(mShadow);
            }
            
            j = 0;
            
            for(i = size-1; i >= 0; i--)
            {
               g.drawLine(j, mid-i, j, mid+i);
               j++;
            }
            break;
      }
      
      g.translate(-x, -y);   
      g.setColor(oldColor);
   }
}
