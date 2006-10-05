/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.Font;

import javax.swing.ImageIcon;
import javax.swing.JLabel;
import javax.swing.JWindow;

/**
 * A splash screen.
 */
public class SplashScreen extends JWindow
{
   /**
    * The fast progress bar.
    */
   protected FastProgressBar mProgressBar;
   
   /**
    * The display image that is used by the window.
    */
   protected JLabel mImageLabel;
   
   /**
    * Creates a new splash screen.
    */
   public SplashScreen()
   {
      Container panel = getContentPane();
      
      // set layout
      panel.setLayout(new BorderLayout());
      
      // setup icon
      mImageLabel = new JLabel("IMAGE ICON GOES HERE");
      mImageLabel.setHorizontalAlignment(JLabel.CENTER);
      
      // setup progress bar
      mProgressBar = new FastProgressBar();
      mProgressBar.setText("");
      mProgressBar.setIndeterminate(true);
      
      // set progress bar font
      Font pgFont = new Font("SansSerif", Font.PLAIN, 12);
      mProgressBar.setFont(pgFont);

      panel.add(mImageLabel);
      panel.add(mProgressBar, BorderLayout.SOUTH);
      
      pack();
   }

   /**
    * Sets the progress bar's foreground and background color.
    * 
    * @param backgroundColor the background color of the progress bar.
    * @param meterColor the color of the progress bar when filled.
    * @param fullTextColor the color of text when displayed against the
    *                      full meter.
    * @param emptyTextColor the color of text when displayed against the 
    *                       empty meter.
    */
   public void setProgressColor(
      Color backgroundColor, Color meterColor, Color fullTextColor,
      Color emptyTextColor)
   {
      mProgressBar.setBackground(backgroundColor);
      mProgressBar.getFastProgressBarUI().setMeterColor(meterColor);
      mProgressBar.getFastProgressBarUI().setFullMeterTextColor(
         fullTextColor);
      mProgressBar.getFastProgressBarUI().setEmptyMeterTextColor(
         emptyTextColor);
   }
   
   /**
    * Sets the image that the splash screen displays.
    * 
    * @param icon the icon that is used to display on the screen.
    */
   public void setImage(ImageIcon icon)
   {
      mImageLabel.setIcon(icon);
      mImageLabel.setText("");
      pack();
   }
   
   /**
    * Centers the splash screen.
    */
   public void center()
   {
      int x = (getGraphicsConfiguration().getBounds().width -
               getWidth()) / 2;
      int y = (getGraphicsConfiguration().getBounds().height -
               getHeight()) / 2;
        
      setLocation(x, y);
   }
   
   /**
    * Sets the progress bar text.
    *
    * @param text the text for the splash screen.
    */
   public void setProgressText(String text)
   {
      mProgressBar.setText(text);
   }
}
