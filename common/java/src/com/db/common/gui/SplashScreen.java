/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common.gui;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.Font;

import javax.swing.ImageIcon;
import javax.swing.JLabel;
import javax.swing.JProgressBar;
import javax.swing.JWindow;

/**
 * A splash screen.
 */
public class SplashScreen extends JWindow
{
   /**
    * The progress bar.
    */
   protected JProgressBar mProgressBar;
   
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
      Font pgFont = new Font("SansSerif", Font.PLAIN, 12);
      mProgressBar = new JProgressBar();
      mProgressBar.setStringPainted(true);
      mProgressBar.setString("");
      mProgressBar.setIndeterminate(true);
      mProgressBar.setFont(pgFont);

      panel.add(mImageLabel);
      panel.add(mProgressBar, BorderLayout.SOUTH);
      
      pack();
   }

   /**
    * Sets the progress bar's foreground and background color.
    * 
    * @param fgColor the foreground color for the progress bar.
    * @param bgColor the background color for the progress bar.
    */
   public void setProgressColor(Color fgColor, Color bgColor)
   {
      mProgressBar.setForeground(fgColor);
      mProgressBar.setBackground(bgColor);
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
      mProgressBar.setString(text);
   }
}
