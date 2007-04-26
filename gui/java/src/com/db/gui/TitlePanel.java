/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Color;
import java.awt.Font;
import java.awt.GridLayout;

import javax.swing.BorderFactory;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.UIManager;
import javax.swing.plaf.LabelUI;

/**
 * A very simple title panel. This class is used across different pages that 
 * need for a consistent title.
 * 
 * @author Dave Longley
 */
public class TitlePanel extends JPanel
{
   /**
    * Creates a new title panel with the given text.
    * 
    * @param text the title text.
    */
   public TitlePanel(String text)
   {
      // set layout
      setLayout(new GridLayout(1, 1));
      
      Color foreground = UIManager.getColor("TitlePanel.foreground");
      if(foreground == null)
      {
         foreground = new Color(0x3388bb);
      }
      
      Color background = UIManager.getColor("TitlePanel.background");
      if(background == null)
      {
         background = new Color(0x116688);
      }
      
      // set colors
      setForeground(foreground);
      setBackground(background);
      
      // set and border
      setBorder(BorderFactory.createLoweredBevelBorder());
      
      // create title label
      JLabel title = new JLabel(text);
      title.setUI((LabelUI)SmoothLabelUI.createUI(title));
      title.setForeground(Color.WHITE);
      title.setHorizontalAlignment(JLabel.CENTER);
      
      // create title font
      Font defaultFont = title.getFont();
      float titleSize = defaultFont.getSize2D() * 1.5f;
      Font titleFont = defaultFont.deriveFont(Font.BOLD, titleSize);
      
      // set title font
      title.setFont(titleFont);
      
      // create add title label
      add(title);
   }
   
   /**
    * Resets the UI property with a value from the current look and feel.
    */
   @Override
   public void updateUI()
   {
      setUI(GradientPanelUI.createUI(this));
   }
}
