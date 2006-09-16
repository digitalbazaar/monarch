/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.device;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A UPnPDevice represents a device that supports the UPnP protocol. It
 * provides UPnPServices that a UPnPControlPoint can use to control it.
 * 
 * @author Dave Longley
 */
public class UPnPDevice
{
   /**
    * The UPnPDeviceDescription for this UPnPDevice.
    */
   protected UPnPDeviceDescription mDescription;   
   
   /**
    * Creates a new UPnPDevice with no set UPnPDeviceDescription.
    */
   public UPnPDevice()
   {
      // set no device description
      setDescription(null);
   }
   
   /**
    * Sets the UPnPDeviceDescription for this UPnPDevice.
    * 
    * @param description the UPnPDeviceDescription for this UPnPDevice.
    */
   public void setDescription(UPnPDeviceDescription description)
   {
      mDescription = description;
   }

   /**
    * Gets the UPnPDeviceDescription for this UPnPDevice. This method
    * will return null if no description has been set for this UPnPDevice.
    * 
    * @return the UPnPDeviceDescription for this UPnPDevice (can be null).
    */
   public UPnPDeviceDescription getDescription()   
   {
      return mDescription;
   }
   
   /**
    * Gets the logger for this UPnPDevice.
    * 
    * @return the logger for this UPnPDevice.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
