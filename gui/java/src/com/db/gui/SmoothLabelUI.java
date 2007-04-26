/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;

import javax.swing.JComponent;
import javax.swing.plaf.ComponentUI;
import javax.swing.plaf.basic.BasicLabelUI;

/**
 * Overloads BasicLabelUI to use anti-aliasing.
 * 
 * @author Dave Longley
 */
public class SmoothLabelUI extends BasicLabelUI
{
   /**
    * The shared UI object.
    */
   protected static SmoothLabelUI SMOOTH_LABEL_UI;
   
   /**
    * Creates a new SmoothLabelUI.
    */
   protected SmoothLabelUI()
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
      if(SMOOTH_LABEL_UI == null)
      {
         SMOOTH_LABEL_UI = new SmoothLabelUI();
      }
      
      return SMOOTH_LABEL_UI;
   }   
   
   /** 
    * Overloaded to enable anti-aliasing.
    * 
    * @param g the graphics.
    * @param c the component to paint.
    */
   @Override
   public void paint(Graphics g, JComponent c)      
   {
      Graphics2D g2 = (Graphics2D)g;
      
      // turn on text anti-aliasing
      g2.setRenderingHint(
         RenderingHints.KEY_TEXT_ANTIALIASING,
         RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
      
      super.paint(g, c);
   }
}
