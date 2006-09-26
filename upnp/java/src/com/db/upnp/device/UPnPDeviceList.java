/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.device;

import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.upnp.device.UPnPDevice;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A UPnPDeviceList represents a list of UPnPDevices.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * <pre>
 * 
 * <deviceList>
 *   Description of embedded devices defined by a UPnP Forum working
 *   committee (if any) go here
 *   Description of embedded devices added by UPnP vendor (if any) go here
 * </deviceList>
 * 
 * </pre>
 * 
 * deviceList
 * Required if and only if root device has embedded devices. Contains the
 * following sub elements:
 * 
 * device
 * Required. Repeat once for each embedded device defined by a UPnP Forum
 * working committee. If UPnP vendor differentiates device by embedding
 * additional UPnP devices, repeat once for each embedded device.
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class UPnPDeviceList extends AbstractXmlSerializer
implements Iterable<UPnPDevice>
{
   /**
    * The UPnPDevices for this list.
    */
   protected Vector<UPnPDevice> mDevices;
   
   /**
    * Creates a new UPnPDeviceList.
    */
   public UPnPDeviceList()
   {
      // create the devices list
      mDevices = new Vector<UPnPDevice>();
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "deviceList";
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
      XmlElement listElement = new XmlElement(getRootTag());
      listElement.setParent(parent);
      
      // convert each device to an xml element child
      for(UPnPDevice device: getDevices())
      {
         listElement.addChild(device.convertToXmlElement(listElement));
      }
      
      // return root element
      return listElement;
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
      
      // clear device list
      clear();
      
      // convert devices
      for(XmlElement deviceElement: element.getChildren("device"))
      {
         UPnPDevice device = new UPnPDevice();
         if(device.convertFromXmlElement(deviceElement))
         {
            addDevice(device);
         }
      }
      
      return rval;
   }
   
   /**
    * Adds a UPnPDevice to this list.
    * 
    * @param device the UPnPDevice to add.
    */
   public void addDevice(UPnPDevice device)
   {
      getDevices().add(device);
   }
   
   /**
    * Removes a UPnPDevice from this list.
    * 
    * @param device the UPnPDevice to remove.
    */
   public void removeDevice(UPnPDevice device)
   {
      getDevices().remove(device);
   }
   
   /**
    * Gets the first device from this list with the specified device type.
    * 
    * @param deviceType the device type of the UPnPDevice to retrieve. 
    * 
    * @return the retrieved UPnPDevice or null if no match was found.
    */
   public UPnPDevice getFirstDevice(String deviceType)
   {
      UPnPDevice rval = null;
      
      for(Iterator<UPnPDevice> i = iterator(); i.hasNext() && rval == null;)
      {
         UPnPDevice device = i.next();
         if(device.getDeviceType().equals(deviceType))
         {
            rval = device;
         }
      }
      
      return rval;
   }   
   
   /**
    * Gets the UPnPDevice for this list in a vector.
    * 
    * @return the UPnPDevice for this list in a vector.
    */
   public Vector<UPnPDevice> getDevices()
   {
      return mDevices;
   }
   
   /**
    * Clears the devices from this list.
    */
   public void clear()
   {
      getDevices().clear();
   }

   /**
    * Gets an iterator over the devices in this list.
    *
    * @return an iterator over the UPnPDevices in this list.
    */
   public Iterator<UPnPDevice> iterator()
   {
      return getDevices().iterator();
   }
   
   /**
    * Gets the number of devices in this list.
    * 
    * @return the number of devices in this list.
    */
   public int getDeviceCount()
   {
      return getDevices().size();
   }
   
   /**
    * Gets the logger for this UPnPDeviceList.
    * 
    * @return the logger for this UPnPDeviceList.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
