/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.Rectangle;

import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.LookAndFeel;
import javax.swing.UIManager;
import javax.swing.plaf.ComponentUI;
import javax.swing.plaf.PanelUI;

import com.sun.java.swing.SwingUtilities2;

/**
 * This class draws the ui for a FastProgressBar.
 * 
 * @author Dave Longley
 */
public class FastProgressBarUI extends PanelUI
{
   /**
    * Shared ui object.
    */
   protected static FastProgressBarUI smFastProgressBarUI;
   
   /**
    * The meter color.
    */
   protected Color mMeterColor;
   
   /**
    * The empty meter text color.
    */
   protected Color mEmptyMeterTextColor;
   
   /**
    * The full meter text color.
    */
   protected Color mFullMeterTextColor;
   
   /**
    * Creates a new FastProgressBarUI.
    */
   public FastProgressBarUI()
   {
   }
   
   /**
    * Creates a new FastProgressBarUI for a specific fast progress bar.
    * 
    * @param progressBar the progress bar to create the ui for.
    */
   public FastProgressBarUI(FastProgressBar progressBar)
   {
      // install defaults
      installDefaults(progressBar);
   }

   /**
    * Creates a new FastProgressBarUI for the passed component.
    * 
    * @param c the component to create the ui for.
    */
   public static ComponentUI createUI(JComponent c)
   {
      if(smFastProgressBarUI == null)
      {
         smFastProgressBarUI = new FastProgressBarUI();
      }
      
      return smFastProgressBarUI;
   }

   /**
    * Overridden to call install ui defaults on a progress bar.
    * 
    * @param p the panel to install ui defaults on.
    */
   protected void installDefaults(JPanel p)
   {
      installDefaults((FastProgressBar)p);
   }
   
   /**
    * Installs default values for this ui onto the passed progress bar.
    * 
    * @param progressBar the progressBar to install defaults on.
    */
   protected void installDefaults(FastProgressBar progressBar)
   {
      LookAndFeel.installProperty(progressBar, "opaque", Boolean.TRUE);
      LookAndFeel.installBorder(progressBar,"ProgressBar.border");
      LookAndFeel.installColorsAndFont(progressBar,
         "ProgressBar.background", "ProgressBar.foreground",
         "ProgressBar.font");
      
      // reset text and meter colors
      resetTextAndMeterColors();
   }
   
   /**
    * Gets the bounding box for painting the progress meter.
    * 
    * @param progressBar the progress bar to paint the meter for.
    * 
    * @return the bounding box for painting the progress meter.
    */
   protected Rectangle getProgressBarBounds(FastProgressBar progressBar)
   {
      Rectangle rect = progressBar.getBounds();

      // get insets
      Insets insets = progressBar.getInsets();
      
      // subtract insets
      rect.x += insets.left;
      rect.y += insets.top;
      rect.width -= insets.right;
      rect.height -= insets.bottom;
      
      return rect;
   }
   
   /**
    * Gets the indeterminate box bounds for the passed progress bar.
    * 
    * @param progressBarBounds the bounds for the progress bar.
    * @param percentage the percentage of the indeterminate value for the
    *                   progress bar.
    * 
    * @return the indeterminate box bounds.
    */
   protected Rectangle getBoxBounds(
      Rectangle progressBarBounds, double percentage)
   {
      Rectangle rect = new Rectangle();
      
      // set the width and height of the box
      rect.width = progressBarBounds.width / 10;  
      rect.height = progressBarBounds.height;
      
      // map the indeterminate value to the progress bar bounds less
      // the size of the box and store it as the box's x coordinate
      rect.x = (int)Math.round(percentage *
         (progressBarBounds.width - rect.width));
      
      return rect;
   }

   /**
    * Gets the determinate meter bounds for the passed progress bar.
    * 
    * @param progressBarBounds the bounds for the progress bar.
    * @param percentage the percentage of the current value for the
    *                   progress bar.
    * 
    * @return the determinate meter bounds.
    */
   protected Rectangle getMeterBounds(
      Rectangle progressBarBounds, double percentage)
   {
      Rectangle rect = new Rectangle();
      
      // set the height of the meter
      rect.height = progressBarBounds.height;
      
      // map the current value to the progress bar bounds and store
      // it as the meter's width
      rect.width = (int)Math.round(percentage * progressBarBounds.width);
      
      return rect;
   }
   
   /**
    * Paints this ui for the passed progress bar. The ui will paint
    * an indeterminate progress box.
    * 
    * @param g the graphics to paint with.
    * @param progressBar the progress bar to paint for.
    */
   protected void paintIndeterminateBox(
      Graphics2D g, FastProgressBar progressBar)
   {
      // get the progress bar bounds
      Rectangle progressBarBounds = progressBar.getBounds();
      
      // get the percentage
      double percentage = progressBar.getIndeterminatePercentage();
      
      // paint indeterminate box
      Rectangle rect = getBoxBounds(progressBarBounds, percentage);
      g.setColor(getMeterColor());
      g.fill(rect);
      
      // paint text
      paintText(progressBar, g, progressBarBounds, rect);
   }
   
   /**
    * Paints this ui for the passed progress bar. The ui will paint
    * a determinate progress meter.
    * 
    * @param g the graphics to paint with.
    * @param progressBar the progress bar to paint for.
    */
   protected void paintDeterminateMeter(
      Graphics2D g, FastProgressBar progressBar)
   {
      // get the progress bar bounds
      Rectangle progressBarBounds = progressBar.getBounds();
      
      // get the indeterminate percentage
      double percentage = progressBar.getPercentage();
      
      // paint determinate meter
      Rectangle rect = getMeterBounds(progressBarBounds, percentage);
      g.setColor(getMeterColor());
      g.fill(rect);
      
      // paint text
      paintText(progressBar, g, progressBarBounds, rect);
   }
   
   /**
    * Paints the text for the passed progress bar on top of an existing
    * indeterminate box or determinate meter.
    * 
    * @param progressBar the progress bar to paint the text for.
    * @param g the graphics to paint with. 
    * @param progressBarBounds the bounds for the progress bar.
    * @param boxOrMeterBounds the bounds for the existing box or meter.
    */
   protected void paintText(
      FastProgressBar progressBar, Graphics2D g,
      Rectangle progressBarBounds, Rectangle boxOrMeterBounds)
   {
      // get text, ensure it is not null
      String text = progressBar.getText();
      if(text != null)
      {
         // get the old clipping bounds
         Rectangle oldClipBounds = g.getClipBounds();
         
         // set font
         g.setFont(progressBar.getFont());
         
         // get the text width
         FontMetrics fontMetrics =
            progressBar.getFontMetrics(progressBar.getFont());
         int textWidth = SwingUtilities2.stringWidth(
            progressBar, fontMetrics, text);
         
         // get progress bar width and height
         double width = progressBarBounds.width;
         double height = progressBarBounds.height;
            
         // set the text x position
         int x = (int)Math.round((width - textWidth) / 2);
         
         // get the text height
         int textHeight = fontMetrics.getHeight() + fontMetrics.getDescent();
         
         // get the distance to the text
         double dist = height - textHeight;
         if(dist != 0)
         {
            dist /= 2.0D;
         }
         
         // set the text y position
         int y = progressBarBounds.y + (int)Math.round(height / 2.0D - dist);

         // draw empty meter text
         g.setColor(getEmptyMeterTextColor());
         SwingUtilities2.drawString(progressBar, g, text, x, y);
         
         // set clipping bounds
         g.setClip(boxOrMeterBounds);
         
         // draw full meter text
         g.setColor(getFullMeterTextColor());
         SwingUtilities2.drawString(progressBar, g, text, x, y);
         
         // restore the old clipping bounds
         g.setClip(oldClipBounds);
      }
   }
   
   /**
    * Paints this ui for the passed progress bar. Either paints it
    * an indeterminate progress box or as a determinate progress meter.
    * 
    * @param g the graphics to paint with.
    * @param progressBar the progress bar to paint.
    */
   public void paint(Graphics g, FastProgressBar progressBar)
   {
      // get 2d graphics
      Graphics2D g2d = (Graphics2D)g;
      
      if(progressBar.isIndeterminate())
      {
         paintIndeterminateBox(g2d, progressBar);
      }
      else
      {
         paintDeterminateMeter(g2d, progressBar);
      }
   }
   
   /**
    * Paints this ui for the passed component. The component will
    * be cast to a FastProgressBar.
    * 
    * @param g the graphics to paint with.
    * @param component the component to paint.
    */
   public void paint(Graphics g, JComponent component)
   {
      paint(g, (FastProgressBar)component);
   }
   
   /**
    * Gets the minimum size for the passed component.
    * 
    * @param c the component to get the minimum size for.
    * 
    * @return the component's minimum size.
    */
   public Dimension getMinimumSize(JComponent c)
   {
      return new Dimension(10, 10);
   }
   
   /**
    * Gets the maximum size for the passed component.
    * 
    * @param c the component to get the maximum size for.
    * 
    * @return the component's maximum size.
    */
   public Dimension getMaximumSize(JComponent c)
   {
      return new Dimension(Integer.MAX_VALUE, Integer.MAX_VALUE);
   }
   
   /**
    * Gets the preferred size for the passed component.
    * 
    * @param c the component to get the prefererd size for.
    * 
    * @return the component's preferred size.
    */
   public Dimension getPreferredSize(JComponent c)
   {
      Dimension size = getMinimumSize(c);
      
      // cast component to a progress bar
      FastProgressBar progressBar = (FastProgressBar)c;
      
      // get font metrics for progress bar
      FontMetrics fontMetrics =
         progressBar.getFontMetrics(progressBar.getFont());
      
      // get text size
      Dimension textSize = new Dimension(0, 0);
      
      // always set preferred height to text height (add descent to ensure
      // there is enough space for text at the bottom)
      textSize.height = fontMetrics.getHeight() + fontMetrics.getDescent();
      
      // set preferred width to text width, if there is text
      if(progressBar.getText() != null) 
      {
         textSize.width = SwingUtilities2.stringWidth(
            progressBar, fontMetrics, progressBar.getText());
      }
      
      // increase size as necessary
      size.width = Math.max(size.width, textSize.width);
      size.height = Math.max(size.height, textSize.height);
      
      // get progress bar insets
      Insets insets = progressBar.getInsets();
      
      // add insets
      size.width += insets.left + insets.right;
      size.height += insets.top + insets.bottom;
      
      return size;
   }
   
   /**
    * Sets the meter color.
    * 
    * @param color the meter color.
    */
   public void setMeterColor(Color color)
   {
      mMeterColor = color;
   }

   /**
    * Gets the meter color.
    * 
    * @return the meter color.
    */
   public Color getMeterColor()
   {
      return mMeterColor;
   }

   /**
    * Sets the empty meter text color.
    * 
    * @param color the empty meter text color.
    */
   public void setEmptyMeterTextColor(Color color)
   {
      mEmptyMeterTextColor = color;
   }

   /**
    * Gets the empty meter text color.
    * 
    * @return the empty meter text color.
    */
   public Color getEmptyMeterTextColor()
   {
      return mEmptyMeterTextColor;
   }
   
   /**
    * Sets the full meter text color.
    * 
    * @param color the full meter text color.
    */
   public void setFullMeterTextColor(Color color)
   {
      mFullMeterTextColor = color;
   }

   /**
    * Gets the full meter text color.
    * 
    * @return the full meter text color.
    */
   public Color getFullMeterTextColor()
   {
      return mFullMeterTextColor;
   }
   
   /**
    * Resets the meter color to its default setting.
    */
   public void resetMeterColor()
   {
      // set the meter color
      setMeterColor(UIManager.getColor("ProgressBar.selectionBackground"));
   }
   
   /**
    * Resets the text colors to their default settings.
    */
   public void resetTextColors()
   {
      // set the text colors
      setEmptyMeterTextColor(
         UIManager.getColor("ProgressBar.selectionBackground"));
      setFullMeterTextColor(
         UIManager.getColor("ProgressBar.selectionForeground"));
   }

   /**
    * Resets the text and meter colors to their default settings.
    */
   public void resetTextAndMeterColors()
   {
      // reset meter color
      resetMeterColor();
      
      // reset text colors
      resetTextColors();
   }
}
