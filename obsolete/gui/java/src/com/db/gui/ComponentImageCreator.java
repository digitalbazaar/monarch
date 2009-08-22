/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.gui;

import java.awt.AlphaComposite;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;

/**
 * This class is used to create an Image of a gui component.
 * 
 * @author Dave Longley
 */
public class ComponentImageCreator
{
   /**
    * Creates an Image of the passed component.
    * 
    * @param component the component to create the Image of.
    * @param size the size of the Image to create.
    * 
    * @return the Image of the component.
    */
   public static Image createComponentImage(
      Component component, Dimension size)
   {
      return createComponentImage(component, size, 1.0F);
   }
   
   /**
    * Creates an Image of the passed component.
    * 
    * @param component the component to create the Image of.
    * @param size the size of the Image to create.
    * @param alpha the alpha setting for the image (0.0 to 1.0);
    * 
    * @return the Image of the component.
    */
   public static Image createComponentImage(
      Component component, Dimension size, float alpha)
   {
      Image rval = null;

      // create a buffered image with the appropriate size
      BufferedImage image = new BufferedImage(
         size.width, size.height, BufferedImage.TYPE_4BYTE_ABGR);

      // store the bounds of the component
      Rectangle rect = component.getBounds();
      
      // set the bounds of the component
      component.setBounds(0, 0, size.width, size.height);
      
      // create graphics
      Graphics2D g2 = image.createGraphics();
      
      g2.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC, alpha));
      
      // paint component onto image
      component.paint(g2);
      
      // dispose of graphics
      g2.dispose();
      
      // restore bounds of the component
      component.setBounds(rect);
      
      // set image as return value
      rval = image;
      
      // return image
      return rval;
   }
}
