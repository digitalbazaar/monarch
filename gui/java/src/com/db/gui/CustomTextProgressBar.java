/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Color;

import javax.swing.JProgressBar;

/**
 * A progress bar that allows its text features (i.e. color) to be set.
 * 
 * @author Dave Longley
 */
public class CustomTextProgressBar extends JProgressBar
{
   /**
    * Creates a new JProgressBar.
    */
   public CustomTextProgressBar()
   {
      CustomTextProgressBarUI ui = new CustomTextProgressBarUI();
      setUI(ui);
   }
   
   /**
    * Sets the color of the text.
    * 
    * @param filled the color of the text when the progress bar is filled.
    * @param unfilled the color of the text when the progress bar is unfilled.
    */
   public void setTextColors(Color filled, Color unfilled)
   {
      setFilledTextColor(filled);
      setUnfilledTextColor(unfilled);
   }
   
   /**
    * Sets the color of the text.
    * 
    * @param c the color of the text when the progress bar is filled or
    *          unfilled.
    */
   public void setTextColors(Color c)
   {
      setFilledTextColor(c);
      setUnfilledTextColor(c);
   }
   
   /**
    * Sets the color of the text when it is painted over a filled area of
    * the progress bar.
    * 
    * @param c the new filled text color.
    */
   public void setFilledTextColor(Color c)
   {
      ((CustomTextProgressBarUI)getUI()).setFilledTextColor(c);
   }
   
   /**
    * Sets the color of the text when it is painted over an unfilled area of
    * the progress bar.
    * 
    * @param c the new unfilled text color.
    */
   public void setUnfilledTextColor(Color c)
   {
      ((CustomTextProgressBarUI)getUI()).setUnfilledTextColor(c);
   }   
   
   /**
    * Resets the filled and unfilled text colors to the default.
    */
   public void resetTextColors()
   {
      ((CustomTextProgressBarUI)getUI()).resetTextColors();
   }   
}
