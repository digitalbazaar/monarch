/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.device;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

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
   public String getRootTag()   
   {
      return "root";
   }
   
   /**
    * Creates an XmlElement from this object.
    *
    * @param parent the parent XmlElement for the XmlElement being created
    *               (can be null). 
    * 
    * @return the XmlElement that represents this object.
    */
   public XmlElement convertToXmlElement(XmlElement parent)   
   {
      // create the root element
      XmlElement rootElement = new XmlElement(getRootTag());
      rootElement.setParent(parent);
      
      // FIXME:
      
      // return root element
      return rootElement;
   }
   
   /**
    * Converts this object from an XmlElement.
    *
    * @param element the XmlElement to convert from.
    * 
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXmlElement(XmlElement element)   
   {
      boolean rval = true;

      // FIXME:
      
      return rval;
   }
   
   /**
    * Gets the logger for this UPnPDeviceIcon.
    * 
    * @return the logger for this UPnPDeviceIcon.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
