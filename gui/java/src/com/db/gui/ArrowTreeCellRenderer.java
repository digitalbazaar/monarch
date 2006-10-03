/*
 * Copyright (c) 2005-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.image.BufferedImage;

import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.plaf.basic.BasicArrowButton;
import javax.swing.plaf.basic.BasicButtonUI;
import javax.swing.tree.DefaultTreeCellRenderer;

/**
 * Draws arrows next to tree cells to indicate expanded/collapsed states.
 * 
 * @author Dave Longley
 */
public class ArrowTreeCellRenderer extends DefaultTreeCellRenderer
{
   /**
    * The arrow image size.
    */
   protected int ARROW_IMAGE_SIZE = 20;
   
   /**
    * Creates a new arrow tree cell renderer.
    */
   public ArrowTreeCellRenderer()
   {
      // create the icons
      Image image = createOpenImage();
      ImageIcon openImageIcon = new ImageIcon(image); 
      
      image = createClosedImage();
      ImageIcon closedImageIcon = new ImageIcon(image);
      
      image = createLeafImage();
      ImageIcon leafImageIcon = new ImageIcon(image);

      // set the icons
      setOpenIcon(openImageIcon);
      setClosedIcon(closedImageIcon);
      setLeafIcon(leafImageIcon);
   }
   
   /**
    * Creates the open image.
    * 
    * @return the open image.
    */
   protected Image createOpenImage()
   {
      // setup the image
      BufferedImage image = new BufferedImage(ARROW_IMAGE_SIZE,
                                              ARROW_IMAGE_SIZE,
                                              BufferedImage.TYPE_4BYTE_ABGR);
      
      BasicArrowButton b = new BasicArrowButton(BasicArrowButton.SOUTH);
      b.setUI(new BasicButtonUI());
      b.setBackground(new Color(0, 0, 0, 0));
      b.setOpaque(false);
      b.setBorder(BorderFactory.createEmptyBorder());
      b.setBounds(0, 0, ARROW_IMAGE_SIZE, ARROW_IMAGE_SIZE);
      
      Graphics g = image.createGraphics();
      g.translate(0, 2);
      b.paint(g);
      
      return image;
   }
   
   /**
    * Creates the closed image.
    * 
    * @return the closed image.
    */
   protected Image createClosedImage()
   {
      // setup the image
      BufferedImage image = new BufferedImage(ARROW_IMAGE_SIZE,
                                              ARROW_IMAGE_SIZE,
                                              BufferedImage.TYPE_4BYTE_ABGR);
      
      BasicArrowButton b = new BasicArrowButton(BasicArrowButton.EAST);
      b.setUI(new BasicButtonUI());
      b.setBackground(new Color(0, 0, 0, 0));
      b.setOpaque(false);
      b.setBorder(BorderFactory.createEmptyBorder());
      b.setBounds(0, 0, ARROW_IMAGE_SIZE, ARROW_IMAGE_SIZE);
      
      Graphics g = image.createGraphics();
      g.translate(0, 2);
      b.paint(g);      
      
      return image;
   }
   
   /**
    * Creates the leaf image.
    * 
    * @return the leaf image.
    */
   protected Image createLeafImage()
   {
      // setup the image
      BufferedImage image = new BufferedImage(ARROW_IMAGE_SIZE / 4,
                                              ARROW_IMAGE_SIZE,
                                              BufferedImage.TYPE_4BYTE_ABGR);
      
      return image;
   }
}
