/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.device;

import java.net.MalformedURLException;
import java.net.URL;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.net.http.HttpWebClient;
import com.db.net.http.HttpWebConnection;
import com.db.net.http.HttpWebRequest;
import com.db.net.http.HttpWebResponse;
import com.db.upnp.service.UPnPService;
import com.db.upnp.service.UPnPServiceDescription;

/**
 * A UPnPRootDevice is a root UPnP device that has a UPnP server that allows it
 * to make itself available to a UPnPControlPoint. It has a location URL that
 * points to its UPnPDeviceDescription and a Unique Service Name.  
 * 
 * @author Dave Longley
 */
public class UPnPRootDevice
{
   /**
    * The server for the device.
    */
   protected String mServer;
   
   /**
    * The location (a URL) for the device's description.
    */
   protected String mLocation;
   
   /**
    * The Unique Service Name for the device.
    */
   protected String mUsn;
   
   /**
    * The UPnPDeviceDescription for this device.
    */
   protected UPnPDeviceDescription mDescription;   
   
   /**
    * Creates a new UPnPRootDevice with no set UPnPDeviceDescription.
    */
   public UPnPRootDevice()
   {
      // set defaults
      setServer("");
      setLocation("");
      setUsn("");
      
      // set no device description
      setDescription(null);
   }
   
   /**
    * Retrieves the UPnPDeviceDescription for this device from its location.
    * The description will be set to this device if it is successfully
    * retrieved.
    * 
    * This method will do an HTTP GET to retrieve the UPnP Device Description
    * from the URL set by setLocation().
    * 
    * @return true if the device description was retrieved successfully,
    *         false if not.
    * 
    * @exception MalformedURLException thrown if the URL from getLocation()
    *                                  is malformed.
    */
   public boolean retrieveDeviceDescription()
   throws MalformedURLException
   {
      boolean rval = false;
      
      // create http client and get the xml from the location 
      HttpWebClient client = new HttpWebClient();
      URL url = new URL(getLocation());
      String xml = client.getContent(url, url.getPath(), 0);
      if(xml != null)
      {
         // create a new UPnPDeviceDescription
         UPnPDeviceDescription description = new UPnPDeviceDescription();
         
         // convert the description from the retrieved xml
         if(description.convertFromXml(xml))
         {
            // set the description to this device
            setDescription(description);
            
            // see if the description has no set base URL
            if(description.getBaseUrl().equals(""))
            {
               // set the description's base URL to the location URL
               description.setBaseUrl(getLocation());
            }
            
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Retrieves the UPnPServiceDescription for the specified service from its
    * SCPD URL. The description will be set to the passed service if it is
    * successfully retrieved.
    * 
    * This method will do an HTTP GET to retrieve the UPnP Service Control
    * Protocol Description from the URL set by setSpcdUrl().
    * 
    * This method will fail if the description for this device is not yet
    * available. Make sure to set it or retrieve it via the method
    * retrieveDeviceDescription() before calling this method.
    * 
    * @param service the service to retrieve the description of.
    * 
    * @return true if the service description was retrieved successfully,
    *         false if not.
    * 
    * @exception MalformedURLException thrown if the URL from getScpdUrl()
    *                                  is malformed.
    */
   public boolean retrieveServiceDescription(UPnPService service)
   throws MalformedURLException
   {
      boolean rval = false;
      
      if(getDescription() != null)
      {
         // create http client and get the xml from the location 
         HttpWebClient client = new HttpWebClient();
         URL url = new URL(
            getDescription().getBaseUrl() + service.getScpdUrl());
         String xml = client.getContent(url, url.getPath(), 0);
         if(xml != null)
         {
            // create a new UPnPServiceDescription
            UPnPServiceDescription description = new UPnPServiceDescription();
            
            // convert the description from the retrieved xml
            if(description.convertFromXml(xml))
            {
               // set the description to the service
               service.setDescription(description);
               rval = true;
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Retrieves the UPnPServiceDescription for the specified service from its
    * SCPD URL. The description will be set to the passed service if it is
    * successfully retrieved.
    * 
    * This method will use the passed persistent http web connection to
    * perform an HTTP GET to retrieve the UPnP Service Control Protocol
    * Description from the URL set by setSpcdUrl().
    * 
    * This method will fail if the description for this device is not yet
    * available. Make sure to set it or retrieve it via the method
    * retrieveDeviceDescription() before calling this method.
    * 
    * @param service the service to retrieve the description of.
    * @param connection the persistent web connection retrieve the
    *                   description from.
    * 
    * @return true if the service description was retrieved successfully,
    *         false if not.
    */
   public boolean retrieveServiceDescription(
      UPnPService service, HttpWebConnection connection)
   {
      boolean rval = false;
      
      if(getDescription() != null)
      {
         // create an HttpWebRequest
         HttpWebRequest request = new HttpWebRequest(connection);
         
         request.getHeader().setVersion("HTTP/1.1");
         request.getHeader().setMethod("GET");
         request.getHeader().setPath(service.getScpdUrl());
         request.getHeader().setHost(connection.getRemoteHost());
         request.getHeader().setUserAgent(HttpWebClient.DEFAULT_USER_AGENT);
         request.getHeader().addHeader("Persist", connection.getRemoteHost());
         request.getHeader().setConnection("Keep-Alive, Persist");
         
         // send request
         if(request.sendHeader())
         {
            // create response
            HttpWebResponse response = request.createHttpWebResponse();
            
            // receive response header
            if(response.receiveHeader())
            {
               // receive response body
               String xml = response.receiveBodyString();
               if(xml != null)
               {
                  // create a new UPnPServiceDescription
                  UPnPServiceDescription description =
                     new UPnPServiceDescription();
                  
                  // convert the description from the retrieved xml
                  if(description.convertFromXml(xml))
                  {
                     // set the description to the service
                     service.setDescription(description);
                     rval = true;
                  }
               }
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Retrieves all of the service descriptions for the passed embeded device.
    * The descriptions will be retrieved if they haven't already been set. 
    * 
    * @param device the device to retrieve the service descriptions for.
    * 
    * @return true if all of the descriptions were retrieved successfully,
    *         false if not.
    */
   public boolean retrieveServiceDescriptions(UPnPDevice device)
   {
      boolean rval = true;
      
      // get all top level service descriptions for the device
      for(UPnPService service: device.getServiceList())
      {
         if(service.getDescription() == null)
         {
            try
            {
               rval &= retrieveServiceDescription(service);
            }
            catch(MalformedURLException ignore)
            {
            }
         }
      }
      
      // get all embedded device service descriptions for the device
      for(UPnPDevice embeddedDevice: device.getDeviceList())
      {
         rval &= retrieveServiceDescriptions(embeddedDevice);
      }
      
      return rval;
   }
   
   /**
    * Retrieves all of the service descriptions for the passed embeded device
    * using the passed persistent HttpWebConnection.
    * 
    * The descriptions will be retrieved if they haven't already been set. 
    * 
    * @param device the device to retrieve the service descriptions for.
    * @param connection the persistent web connection retrieve the
    *                   descriptions from.
    * 
    * @return true if all of the descriptions were retrieved successfully,
    *         false if not.
    */
   public boolean retrieveServiceDescriptions(
      UPnPDevice device, HttpWebConnection connection)
   {
      boolean rval = true;
      
      // get all top level service descriptions for the device
      for(UPnPService service: device.getServiceList())
      {
         if(service.getDescription() == null)
         {
            rval &= retrieveServiceDescription(service, connection);
         }
      }
      
      // get all embedded device service descriptions for the device
      for(UPnPDevice embeddedDevice: device.getDeviceList())
      {
         rval &= retrieveServiceDescriptions(embeddedDevice, connection);
      }
      
      return rval;
   }
   
   /**
    * Retrieves the description for this root device and all of the descriptions
    * for its services and its embedded devices' services if their descriptions
    * have not yet been retrieved.
    * 
    * @return true if all of the descriptions were retrieved successfully,
    *         false if not.
    */
   public boolean retrieveAllDescriptions()
   {
      boolean rval = false;
      
      try
      {
         // get the description for this device, if necessary
         if(getDescription() == null)
         {
            rval = retrieveDeviceDescription();
         }
         else
         {
            rval = true;
         }
         
         if(rval)
         {
            // determine if there are service descriptions to look up
            UPnPDevice rootDevice = getDescription().getDevice();
            if(rootDevice.getDeviceList().getDeviceCount() > 0 ||
               rootDevice.getServiceList().getServiceCount() > 0)
            {
               // retrieve all the root service descriptions
               for(UPnPService service: rootDevice.getServiceList()) 
               {
                  rval &= retrieveServiceDescription(service);
               }
               
               // retrieve all the embedded service descriptions
               for(UPnPDevice device: rootDevice.getDeviceList()) 
               {
                  rval &= retrieveServiceDescriptions(device);
               }
               
               // FUTURE CODE: persistent connections are not supported by
               // all devices (even though they are required by HTTP/1.1),
               // so we need to add code in here to check to see if a
               // persistent connection is permitted before executing
               // the code below to more rapidly retrieve descriptions
               /*
               // determine if the descriptione has a base URL or not
               if(!getDescription().getBaseUrl().equals(""))
               {
                  // since this description has a base URL, we can use one
                  // persistent http connection to get all of the descriptions
                  
                  HttpWebClient client = new HttpWebClient();
                  client.setUrl(getDescription().getBaseUrl());
                  
                  // get an http web connection
                  HttpWebConnection connection = client.connect();
                  if(connection != null)
                  {
                     // retrieve all of the root service descriptions
                     for(UPnPService service: rootDevice.getServiceList()) 
                     {
                        rval &= retrieveServiceDescription(service, connection);
                     }
                     
                     // retrieve all of the embedded device service descriptions
                     for(UPnPDevice device: rootDevice.getDeviceList()) 
                     {
                        rval &= retrieveServiceDescriptions(device, connection);
                     }
                     
                     // disconnect connection
                     connection.disconnect();
                  }
               }*/
            }
         }
      }
      catch(MalformedURLException e)
      {
         getLogger().error(getClass(),
            "Exception thrown while retrieving device/service descriptions!" +
            ",exception=" + e);
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
      
      return rval;
   }
   
   /**
    * Sets the server for this device.
    * 
    * @param server the server for this device.
    */
   public void setServer(String server)
   {
      mServer = server;
   }
   
   /**
    * Gets the server for this device.
    * 
    * @return the server for this device.
    */
   public String getServer()
   {
      return mServer;
   }
   
   /**
    * Sets the location (a URL) for this device's description.
    * 
    * @param location the location for this device's description.
    */
   public void setLocation(String location)   
   {
      mLocation = location;
   }
   
   /**
    * Gets the location (a URL) for this device's description.
    * 
    * @return the location for this device's description.
    */
   public String getLocation()
   {
      return mLocation;
   }
   
   /**
    * Sets the Unique Service Name for this device.
    * 
    * @param usn the Unique Service Name for this device.
    */
   public void setUsn(String usn)
   {
      mUsn = usn;
   }
   
   /**
    * Gets the Unique Service Name for this device.
    * 
    * @return the Unique Service Name for this device.
    */
   public String getUsn()
   {
      return mUsn;
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
    * Gets the string representation for this UPnPRootDevice.
    * 
    * @return the string representation for this UPnPRootDevice.
    */
   public String toString()
   {
      StringBuffer sb = new StringBuffer();
      
      sb.append("[UPnPRootDevice]");
      
      sb.append("\nserver=" + getServer());
      sb.append("\nlocation=" + getLocation());
      sb.append("\nusn=" + getUsn());
      
      sb.append("\n");
      
      return sb.toString();
   }
   
   /**
    * Gets the logger for this UPnPRootDevice.
    * 
    * @return the logger for this UPnPRootDevice.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
