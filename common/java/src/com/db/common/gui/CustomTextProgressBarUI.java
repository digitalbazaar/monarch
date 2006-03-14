/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.gui;

import java.awt.Color;

import javax.swing.UIManager;
import javax.swing.plaf.basic.BasicProgressBarUI;

/**
 * A class that allows a progress bar's text features (i.e. color) to be set.
 * 
 * @author Dave Longley
 */
public class CustomTextProgressBarUI extends BasicProgressBarUI
{
   /**
    * The filled text color.
    */
   protected Color mFilledTextColor;
   
   /**
    * The unfilled text color.
    */
   protected Color mUnfilledTextColor;
   
   /**
    * Creates a new CustomTextProgressBarUI.
    */
   public CustomTextProgressBarUI()
   {
      // reset filled/unfilled text colors
      resetTextColors();
   }
   
   /**
    * Overloaded to get the color of the text over a filled area of the
    * progress bar.
    * 
    * @return the filled text color.
    */
   protected Color getSelectionForeground()
   {
      return mFilledTextColor;
   }
   
   /**
    * Overloaded to get the color of the text over an unfilled area of the
    * progress bar.
    * 
    * @return the unfilled text color.
    */
   protected Color getSelectionBackground()
   {
      return mUnfilledTextColor;
   }   
   
   /**
    * Sets the color of the text when it is painted over a filled area of
    * the progress bar.
    * 
    * @param c the new filled text color.
    */
   public void setFilledTextColor(Color c)
   {
      mFilledTextColor = c;
   }
   
   /**
    * Sets the color of the text when it is painted over an unfilled area of
    * the progress bar.
    * 
    * @param c the new unfilled text color.
    */
   public void setUnfilledTextColor(Color c)
   {
      mUnfilledTextColor = c;
   }
   
   /**
    * Resets the filled and unfilled text colors to the default.
    */
   public void resetTextColors()
   {
      mFilledTextColor =
         UIManager.getColor("ProgressBar.selectionForeground");
      mUnfilledTextColor =
         UIManager.getColor("ProgressBar.selectionBackground");
   }
}
