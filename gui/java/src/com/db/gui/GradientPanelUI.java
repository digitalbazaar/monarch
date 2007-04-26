/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.GradientPaint;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Paint;
import java.awt.RenderingHints;

import javax.swing.JComponent;
import javax.swing.plaf.ComponentUI;
import javax.swing.plaf.basic.BasicPanelUI;

/**
 * Overloads BasicPanelUI to draw a gradient background.
 * 
 * The foreground of the Panel will fade to the background of the panel.
 * 
 * @author Dave Longley
 */
public class GradientPanelUI extends BasicPanelUI
{
   /**
    * The shared UI object.
    */
   protected static GradientPanelUI GRADIENT_PANEL_UI;
   
   /**
    * Creates a new GradientPanelUI.
    */
   protected GradientPanelUI()
   {
   }
   
   /**
    * Creates the shared UI object and returns it.
    * 
    * @param c the component to create the shared UI object for.
    * 
    * @return the shared UI object.
    */
   public static ComponentUI createUI(JComponent c)
   {
      if(GRADIENT_PANEL_UI == null)
      {
         GRADIENT_PANEL_UI = new GradientPanelUI();
      }
      
      return GRADIENT_PANEL_UI;
   }
   
   /**
    * Overloaded to draw a gradient background color.
    * 
    * @param g the graphics.
    * @param c the component to draw.
    */
   @Override
   public void update(Graphics g, JComponent c)
   {
      if(c.isOpaque())
      {
         Graphics2D g2 = (Graphics2D)g;
         
         // save old paint
         Paint oldPaint = g2.getPaint();
         
         // turn on text anti-aliasing
         g2.setRenderingHint(
            RenderingHints.KEY_TEXT_ANTIALIASING,
            RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
         
         // create gradient paint
         GradientPaint gradientPaint = new GradientPaint(
            0, 0, c.getForeground(), 0, c.getHeight(), c.getBackground());
         g2.setPaint(gradientPaint);
         
         // fill background
         g.fillRect(0, 0, c.getWidth(),c.getHeight());
         
         // restore old paint
         g2.setPaint(oldPaint);
      }
      
      paint(g, c);
   }
}
