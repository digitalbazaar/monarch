/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.device;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;
import com.db.xml.XmlException;

/**
 * A UPnPDeviceIcon represents an Icon for a UPnPDevice.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * <pre>
 * 
 * <icon>
 *   <mimetype>image/format</mimetype>
 *   <width>horizontal pixels</width>
 *   <height>vertical pixels</height>
 *   <depth>color depth</depth>
 *   <url>URL to icon</url>
 * </icon>
 * 
 * </pre>
 * 
 * icon
 * Recommended. Icon to depict device in a control point UI. May be localized
 * (cf. ACCEPT-/CONTENT-LANGUAGE headers). Recommend one icon in each of the
 * following sizes (width x height x depth): 16x16x1, 16x16x8, 32x32x1,
 * 32x32x8, 48x48x1, 48x48x8. Contains the following sub elements:
 * 
 * mimetype
 * Required. Icon's MIME type (cf. RFC 2387). Single MIME image type.
 * 
 * width
 * Required. Horizontal dimension of icon in pixels. Integer.
 * 
 * height
 * Required. Vertical dimension of icon in pixels. Integer.
 * 
 * depth
 * Required. Number of color bits per pixel. Integer.
 * 
 * url
 * Required. Pointer to icon image. (XML does not support direct embedding of
 * binary data. See note below.) Retrieved via HTTP. May be relative to base
 * URL. Specified by UPnP vendor. Single URL.
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class UPnPDeviceIcon extends AbstractXmlSerializer
{
   /**
    * The MIME type for this icon.
    */
   protected String mMimeType;
   
   /**
    * The width (in horizontal pixels) for this icon.
    */
   protected int mWidth;
   
   /**
    * The height (in vertical pixels) for this icon.
    */
   protected int mHeight;
   
   /**
    * The color depth (number of color bits per pixel) for this icon.
    */
   protected int mDepth;
   
   /**
    * The URL to the icon image (retrieved via HTTP).
    */
   protected String mIconUrl;
   
   /**
    * Creates a new UPnPDeviceIcon.
    */
   public UPnPDeviceIcon()
   {
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   @Override
   public String getRootTag()   
   {
      return "icon";
   }
   
   /**
    * Creates an XmlElement from this object.
    *
    * @param parent the parent XmlElement for the XmlElement being created
    *               (can be null). 
    * 
    * @return the XmlElement that represents this object.
    */
   @Override
   public XmlElement convertToXmlElement(XmlElement parent)   
   {
      // create the root element
      XmlElement iconElement = new XmlElement(getRootTag());
      iconElement.setParent(parent);
      
      // create the mime type element
      XmlElement mimeTypeElement = new XmlElement("mimeType");
      mimeTypeElement.setValue(getMimeType());
      iconElement.addChild(mimeTypeElement);
      
      // create the width element
      XmlElement widthElement = new XmlElement("width");
      widthElement.setValue(getWidth());
      iconElement.addChild(widthElement);
      
      // create the height element
      XmlElement heightElement = new XmlElement("height");
      heightElement.setValue(getHeight());
      iconElement.addChild(heightElement);
      
      // create the depth element
      XmlElement depthElement = new XmlElement("depth");
      depthElement.setValue(getDepth());
      iconElement.addChild(depthElement);
      
      // create the icon url element
      XmlElement urlElement = new XmlElement("url");
      urlElement.setValue(getIconUrl());
      iconElement.addChild(urlElement);
      
      // return root element
      return iconElement;
   }
   
   /**
    * Converts this object from an XmlElement.
    *
    * @param element the XmlElement to convert from.
    * 
    * @exception XmlException thrown if this object could not be converted from
    *                         xml.
    */
   @Override
   public void convertFromXmlElement(XmlElement element) throws XmlException
   {
      super.convertFromXmlElement(element);
      
      // get the mime type element
      setMimeType(element.getFirstChildValue("mimeType"));
      
      // get the width element
      setWidth(Integer.parseInt(element.getFirstChildValue("width")));
      
      // get the height element
      setHeight(Integer.parseInt(element.getFirstChildValue("height")));
      
      // get the depth element
      setDepth(Integer.parseInt(element.getFirstChildValue("depth")));
      
      // get the icon url element
      setIconUrl(element.getFirstChildValue("url"));
   }
   
   /**
    * Set the MIME type for this icon.
    * 
    * @param mimeType the MIME type for this icon.
    */
   public void setMimeType(String mimeType)
   {
      mMimeType = mimeType;
   }
   
   /**
    * Get the MIME type for this icon.
    * 
    * @return the MIME type for this icon.
    */
   public String getMimeType()
   {
      return mMimeType;
   }
   
   /**
    * Sets the width (in horizontal pixels) for this icon.
    * 
    * @param width the width (in horizontal pixels) for this icon.
    */
   public void setWidth(int width)
   {
      mWidth = width;
   }
   
   /**
    * Gets the width (in horizontal pixels) for this icon.
    * 
    * @return the width (in horizontal pixels) for this icon.
    */
   public int getWidth()
   {
      return mWidth;
   }
   
   /**
    * Sets the height (in vertical pixels) for this icon.
    * 
    * @param height the height (in vertical pixels) for this icon.
    */
   public void setHeight(int height)
   {
      mHeight = height;
   }
   
   /**
    * Gets the height (in vertical pixels) for this icon.
    * 
    * @return the height (in vertical pixels) for this icon.
    */
   public int getHeight()
   {
      return mHeight;
   }
   
   /**
    * Sets the color depth (number of color bits per pixel) for this icon.
    * 
    * @param depth the color depth (number of color bits per pixel) for this
    *              icon.
    */
   public void setDepth(int depth)
   {
      mDepth = depth;
   }
   
   /**
    * Gets the color depth (the number of color bits per pixel) for this icon.
    * 
    * @return the color depth (number of color bits per pixel) for this icon.
    */
   public int getDepth()
   {
      return mDepth;
   }   
   
   /**
    * Sets the URL to the icon image (retrieved via HTTP).
    * 
    * @param iconUrl the URL to the icon image.
    */
   public void setIconUrl(String iconUrl)
   {
      mIconUrl = iconUrl;
   }
   
   /**
    * Gets the URL to the icon image (retrieved via HTTP).
    * 
    * @return the URL to the icon image.
    */
   public String getIconUrl()
   {
      return mIconUrl;
   }
   
   /**
    * Gets the logger for this UPnPDeviceIcon.
    * 
    * @return the logger for this UPnPDeviceIcon.
    */
   @Override
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
