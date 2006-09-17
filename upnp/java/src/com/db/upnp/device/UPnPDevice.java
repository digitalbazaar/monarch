/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.device;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.upnp.service.UPnPServiceList;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A UPnPDevice represents a device that supports the UPnP protocol. It
 * provides UPnPServices that a UPnPControlPoint can use to control it.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * <pre>
 * 
 * <device>
 *   <deviceType>urn:schemas-upnp-org:device:deviceType:v</deviceType>
 *   <friendlyName>short user-friendly title</friendlyName>
 *   <manufacturer>manufacturer name</manufacturer>
 *   <manufacturerURL>URL to manufacturer site</manufacturerURL>
 *   <modelDescription>long user-friendly title</modelDescription>
 *   <modelName>model name</modelName>
 *   <modelNumber>model number</modelNumber>
 *   <modelURL>URL to model site</modelURL>
 *   <serialNumber>manufacturer's serial number</serialNumber>
 *   <UDN>uuid:UUID</UDN>
 *   <UPC>Universal Product Code</UPC>
 *   <iconList>
 *     <icon>
 *       <mimetype>image/format</mimetype>
 *       <width>horizontal pixels</width>
 *       <height>vertical pixels</height>
 *       <depth>color depth</depth>
 *       <url>URL to icon</url>
 *     </icon>
 *     XML to declare other icons, if any, go here
 *   </iconList>
 *   <serviceList>
 *     <service>
 *       <serviceType>urn:schemas-upnp-org:service:serviceType:v</serviceType>
 *       <serviceId>urn:upnp-org:serviceId:serviceID</serviceId>
 *       <SCPDURL>URL to service description</SCPDURL>
 *       <controlURL>URL for control</controlURL>
 *       <eventSubURL>URL for eventing</eventSubURL>
 *     </service>
 *     Declarations for other services defined by a UPnP Forum working
 *     committee (if any) go here
 *     Declarations for other services added by UPnP vendor (if any) go here
 *   </serviceList>
 *   <deviceList>
 *     Description of embedded devices defined by a UPnP Forum working
 *     committee (if any) go here
 *     Description of embedded devices added by UPnP vendor (if any) go here
 *   </deviceList>
 *   <presentationURL>URL for presentation</presentationURL>
 * </device>
 * 
 * device
 * Required. Contains the following sub elements:
 * 
 * deviceType
 * Required. UPnP device type.
 * 
 * For standard devices defined by a UPnP Forum working committee, must begin
 * with urn:schemas-upnp-org:device: followed by a device type suffix, colon,
 * and an integer device version (as shown in the listing above).
 * 
 * For non-standard devices specified by UPnP vendors, must begin with urn:,
 * followed by an ICANN domain name owned by the vendor, followed by :device:,
 * followed by a device type suffix, colon, and an integer version, i.e.,
 * urn:domain-name:device:deviceType:v.
 * 
 * The device type suffix defined by a UPnP Forum working committee or
 * specified by a UPnP vendor must be <= 64 chars, not counting the version
 * suffix and separating colon. Single URI.
 * 
 * friendlyName
 * Required. Short description for end user. Should be localized (cf.
 * ACCEPT-/CONTENT-LANGUAGE headers). Specified by UPnP vendor. String.
 * Should be < 64 characters.
 * 
 * manufacturer
 * Required. Manufacturer's name. May be localized (cf. 
 * ACCEPT-/CONTENT-LANGUAGE headers). Specified by UPnP vendor. String.
 * Should be < 64 characters.
 * 
 * manufacturerURL
 * Optional. Web site for Manufacturer. May be localized (cf.
 * ACCEPT-/CONTENT-LANGUAGE headers). May be relative to base URL. Specified by
 * UPnP vendor. Single URL.
 * 
 * modelDescription
 * Recommended. Long description for end user. Should be localized (cf.
 * ACCEPT-/CONTENT-LANGUAGE headers). Specified by UPnP vendor. String.
 * Should be < 128 characters.
 * 
 * modelName
 * Required. Model name. May be localized (cf. ACCEPT-/CONTENT-LANGUAGE
 * headers). Specified by UPnP vendor. String. Should be < 32 characters.
 * 
 * modelNumber
 * Recommended. Model number. May be localized (cf. ACCEPT-/CONTENT-LANGUAGE
 * headers). Specified by UPnP vendor. String. Should be < 32 characters.
 * 
 * modelURL
 * Optional. Web site for model. May be localized (cf. ACCEPT-/CONTENT-LANGUAGE
 * headers). May be relative to base URL. Specified by UPnP vendor. Single URL.
 * 
 * serialNumber
 * Recommended. Serial number. May be localized (cf. ACCEPT-/CONTENT-LANGUAGE
 * headers). Specified by UPnP vendor. String. Should be < 64 characters.
 * 
 * UDN
 * Required. Unique Device Name. Universally-unique identifier for the device,
 * whether root or embedded. Must be the same over time for a specific device
 * instance (i.e., must survive reboots). Must match the value of the NT header
 * in device discovery messages. Must match the prefix of the USN header in all
 * discovery messages. (The section on Discovery explains the NT and USN
 * headers.) Must begin with uuid: followed by a UUID suffix specified by a
 * UPnP vendor. Single URI.
 * 
 * UPC
 * Optional. Universal Product Code. 12-digit, all-numeric code that identifies
 * the consumer package. Managed by the Uniform Code Council. Specified by UPnP
 * vendor. Single UPC.
 * 
 * iconList
 * Required if and only if device has one or more icons. Specified by UPnP
 * vendor. Contains the following sub elements:
 * 
 * icon
 * Recommended. Icon to depict device in a control point UI. May be localized
 * (cf. ACCEPT-/CONTENT-LANGUAGE headers). Recommend one icon in each of the
 * following sizes (width x height x depth): 16x16x1, 16x16x8, 32x32x1,
 * 32x32x8, 48x48x1, 48x48x8.
 * 
 * serviceList
 * Required. Contains the following sub elements:
 * 
 * service
 * Required. Repeated once for each service defined by a UPnP Forum working 
 * committee. If UPnP vendor differentiates device by adding additional,
 * standard UPnP services, repeated once for additional service.
 * 
 * deviceList
 * Required if and only if root device has embedded devices. Contains the
 * following sub elements:
 * 
 * device
 * Required. Repeat once for each embedded device defined by a UPnP Forum
 * working committee. If UPnP vendor differentiates device by embedding
 * additional UPnP devices, repeat once for each embedded device. Contains sub
 * elements as defined above for root sub element device.
 * 
 * presentationURL
 * Recommended. URL to presentation for device (cf. section on Presentation).
 * May be relative to base URL. Specified by UPnP vendor. Single URL.
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class UPnPDevice extends AbstractXmlSerializer
{
   /**
    * The device type (a URI) for this device.
    */
   protected String mDeviceType;
   
   /**
    * The friendly name for this device. This is a short, user-friendly name.
    */
   protected String mFriendlyName;
   
   /**
    * The manufacturer name for this device.
    */
   protected String mManufacturerName;
   
   /**
    * The URL to the manufacturer's website.
    */
   protected String mManufacturerUrl;

   /**
    * The description for the model of the device. This is a long,
    * user-friendly title.
    */
   protected String mModelDescription;
   
   /**
    * The manufacturer's name for the model of this device.
    */
   protected String mModelName;
   
   /**
    * The manufacturer's model number for this device.
    */
   protected String mModelNumber;
   
   /**
    * The website for the model of this device.
    */
   protected String mModelUrl;
   
   /**
    * The manufacturer's serial number for this device.
    */
   protected String mSerialNumber;
   
   /**
    * The UDN (Unique Device Name) a universally unique identifier for the
    * device.
    */
   protected String mUdn;
   
   /**
    * The UPC (Universal Product Code) for the device.
    */
   protected String mUpc;
   
   /**
    * The icon list for this device.
    */
   protected UPnPDeviceIconList mIconList;
   
   /**
    * The service list for this device.
    */
   protected UPnPServiceList mServiceList;
   
   /**
    * The device list for this device.
    */
   protected UPnPDeviceList mDeviceList;
   
   /**
    * The presentation URL for this device.
    */
   protected String mPresentationUrl;
   
   /**
    * Creates a new UPnPDevice.
    */
   public UPnPDevice()
   {
      // set defaults
      setDeviceType("urn:schemas-upnp-org:device:deviceType:v");
      setFriendlyName("");
      setManufacturerName("");
      setManufacturerUrl("");
      setModelDescription("");
      setModelName("");
      setModelNumber("");
      setModelUrl("");
      setSerialNumber("");
      setUdn("uuid:UUID");
      setUpc("");
      setPresentationUrl("");
      
      // create icon list
      mIconList = new UPnPDeviceIconList();
      
      // create service list
      mServiceList = new UPnPServiceList();
      
      // create device list
      mDeviceList = new UPnPDeviceList();
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "device";
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
      XmlElement deviceElement = new XmlElement(getRootTag());
      deviceElement.setParent(parent);
      
      // create device type element
      XmlElement deviceTypeElement = new XmlElement("deviceType");
      deviceTypeElement.setValue(getDeviceType());
      deviceElement.addChild(deviceTypeElement);
      
      // create friendly name element
      XmlElement friendlyNameElement = new XmlElement("friendlyName");
      friendlyNameElement.setValue(getFriendlyName());
      deviceElement.addChild(friendlyNameElement);
      
      // create manufacturer element
      XmlElement manufacturerElement = new XmlElement("manufacturer");
      manufacturerElement.setValue(getManufacturerName());
      deviceElement.addChild(manufacturerElement);
      
      // create manufacturer URL element, if applicable
      if(!getManufacturerUrl().equals(""))
      {
         XmlElement manufacturerUrlElement = new XmlElement("manufacturerURL");
         manufacturerUrlElement.setValue(getManufacturerUrl());
         deviceElement.addChild(manufacturerUrlElement);
      }
      
      // create model description element, if applicable
      if(!getModelDescription().equals(""))
      {
         XmlElement modelDescriptionElement =
            new XmlElement("modelDescription");
         modelDescriptionElement.setValue(getModelDescription());
         deviceElement.addChild(modelDescriptionElement);
      }
      
      // create model name element
      XmlElement modelNameElement = new XmlElement("modelName");
      modelNameElement.setValue(getModelName());
      deviceElement.addChild(modelNameElement);
      
      // create model number element, if applicable
      if(!getModelNumber().equals(""))
      {
         XmlElement modelNumberElement = new XmlElement("modelNumber");
         modelNumberElement.setValue(getModelNumber());
         deviceElement.addChild(modelNumberElement);
      }
      
      // create model URL element, if applicable
      if(!getModelUrl().equals(""))
      {
         XmlElement modelUrlElement = new XmlElement("modelURL");
         modelUrlElement.setValue(getModelUrl());
         deviceElement.addChild(modelUrlElement);
      }
      
      // create serial number element, if applicable
      if(!getSerialNumber().equals(""))
      {
         XmlElement serialNumberElement = new XmlElement("serialNumber");
         serialNumberElement.setValue(getSerialNumber());
         deviceElement.addChild(serialNumberElement);
      }
      
      // create UDN element
      XmlElement udnElement = new XmlElement("UDN");
      udnElement.setValue(getUdn());
      deviceElement.addChild(udnElement);
      
      // create UPC element, if applicable
      if(!getUpc().equals(""))
      {
         XmlElement upcElement = new XmlElement("UPC");
         upcElement.setValue(getUpc());
         deviceElement.addChild(upcElement);
      }
      
      // convert icon list, if applicable
      if(getIconList().getIconCount() > 0)
      {
         deviceElement.addChild(
            getIconList().convertToXmlElement(deviceElement));
      }
      
      // convert service list, if applicable
      if(getServiceList().getServiceCount() > 0)
      {
         deviceElement.addChild(
            getServiceList().convertToXmlElement(deviceElement));
      }
      
      // convert device list, if applicable
      if(getDeviceList().getDeviceCount() > 0)
      {
         deviceElement.addChild(
            getDeviceList().convertToXmlElement(deviceElement));
      }
      
      // create presentation URL element, if applicable
      if(!getPresentationUrl().equals(""))
      {
         XmlElement presentationUrlElement = new XmlElement("presentationURL");
         presentationUrlElement.setValue(getPresentationUrl());
         deviceElement.addChild(presentationUrlElement);
      }
      
      // return root element
      return deviceElement;
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
      
      // get device type element
      setDeviceType(element.getFirstChildValue("deviceType"));
      
      // get friendly name element
      setFriendlyName(element.getFirstChildValue("friendlyName"));
      
      // get manufacturer element
      setManufacturerName(element.getFirstChildValue("manufacturer"));
      
      // get manufacturer URL element
      setManufacturerUrl(element.getFirstChildValue("manufacturerURL"));
      
      // get model description element, if applicable
      setModelDescription(element.getFirstChildValue("modelDescription"));
      
      // get model name element
      setModelName(element.getFirstChildValue("modelName"));
      
      // get model number element, if applicable
      setModelNumber(element.getFirstChildValue("modelNumber"));
      
      // get model URL element, if applicable
      setModelUrl(element.getFirstChildValue("modelURL"));
      
      // get serial number element, if applicable
      setSerialNumber(element.getFirstChildValue("serialNumber"));
      
      // get UDN element
      setUdn(element.getFirstChildValue("UDN"));
      
      // get UPC element, if applicable
      setUpc(element.getFirstChildValue("UPC"));
      
      // clear icon list
      getIconList().clear();
      
      // convert icon list, if applicable
      XmlElement iconListElement = element.getFirstChild("iconList");
      if(iconListElement != null)
      {
         getIconList().convertFromXmlElement(iconListElement);
      }
         
      // clear service list
      getServiceList().clear();
      
      // convert service list, if applicable
      XmlElement serviceListElement = element.getFirstChild("serviceList");
      if(serviceListElement != null)
      {
         getServiceList().convertFromXmlElement(serviceListElement);
      }
      
      // clear device list
      getDeviceList().clear();
      
      // convert device list, if applicable
      XmlElement deviceListElement = element.getFirstChild("deviceList");
      if(deviceListElement != null)
      {
         getDeviceList().convertFromXmlElement(deviceListElement);
      }
      
      // get presentation URL element, if applicable
      setPresentationUrl(element.getFirstChildValue("presentationURL"));
      
      return rval;
   }
   
   /**
    * Sets the device type (a URI) for this device.
    * 
    * @param deviceType the device type for this device.
    */
   public void setDeviceType(String deviceType)
   {
      mDeviceType = deviceType;
   }
   
   /**
    * Gets the device type (a URI) for this device.
    * 
    * @return the device type for this device.
    */
   public String getDeviceType()
   {
      return mDeviceType;
   }
   
   /**
    * Sets the friendly name for this device. This is a short, user-friendly
    * name.
    * 
    * @param friendlyName the friendly name for this device.
    */
   public void setFriendlyName(String friendlyName)
   {
      mFriendlyName = friendlyName;
   }
   
   /**
    * Gets the friendly name for this device. This is a short, user-friendly
    * name.
    * 
    * @return the friendly name for this device.
    */
   public String getFriendlyName()
   {
      return mFriendlyName;
   }   
   
   /**
    * Sets the manufacturer name for this device.
    * 
    * @param name the manufacturer name for this device.
    */
   public void setManufacturerName(String name)
   {
      mManufacturerName = name;
   }
   
   /**
    * Gets the manufacturer name for this device.
    * 
    * @return the manufacturer name for this device.
    */
   public String getManufacturerName()
   {
      return mManufacturerName;
   }
   
   /**
    * Sets the URL to the manufacturer's website.
    * 
    * @param url the URL to the manufacturer's website.
    */
   public void setManufacturerUrl(String url)
   {
      mManufacturerUrl = url;
   }
   
   /**
    * Gets the URL to the manufacturer's website.
    * 
    * @return the URL to the manufacturer's website.
    */
   public String getManufacturerUrl()
   {
      return mManufacturerUrl;
   }   

   /**
    * Sets the description for the model of the device. This is a long,
    * user-friendly title.
    * 
    * @param description the description for the model of the device. 
    */
   public void setModelDescription(String description)
   {
      mModelDescription = description;
   }
   
   /**
    * Gets the description for the model of the device. This is a long,
    * user-friendly title.
    * 
    * @return the description for the model of the device. 
    */
   public String getModelDescription()
   {
      return mModelDescription;
   }
   
   /**
    * Sets the manufacturer's name for the model of this device.
    * 
    * @param name the manufacturer's name for the model of this device.
    */
   public void setModelName(String name)
   {
      mModelName = name;
   }
   
   /**
    * Gets the manufacturer's name for the model of this device.
    * 
    * @return the manufacturer's name for the model of this device.
    */
   public String getModelName()
   {
      return mModelName;
   }
   
   /**
    * Sets the manufacturer's model number for this device.
    * 
    * @param modelNumber the manufacturer's model number for this device.
    */
   public void setModelNumber(String modelNumber)
   {
      mModelNumber = modelNumber;
   }
   
   /**
    * Gets the manufacturer's model number for this device.
    * 
    * @return the manufacturer's model number for this device.
    */
   public String getModelNumber()
   {
      return mModelNumber;
   }
   
   /**
    * Sets the website for the model of this device.
    * 
    * @param url the URL for the website for the model of this device. 
    */
   public void setModelUrl(String url)
   {
      mModelUrl = url;
   }
   
   /**
    * Gets the website for the model of this device.
    * 
    * @return the URL for the website for the model of this device. 
    */
   public String getModelUrl()
   {
      return mModelUrl;
   }
   
   /**
    * Sets the manufacturer's serial number for this device.
    * 
    * @param serialNumber the manufacturer's serial number for this device. 
    */
   public void setSerialNumber(String serialNumber)
   {
      mSerialNumber = serialNumber;
   }
   
   /**
    * Gets the manufacturer's serial number for this device.
    * 
    * @return the manufacturer's serial number for this device. 
    */
   public String getSerialNumber()
   {
      return mSerialNumber;
   }
   
   /**
    * Sets the UDN (Unique Device Name) a universally unique identifier for the
    * device.
    * 
    * @param udn the UDN (Unique Device Name) for the device.
    */
   public void setUdn(String udn)
   {
      mUdn = udn;
   }
   
   /**
    * Gets the UDN (Unique Device Name) a universally unique identifier for the
    * device.
    * 
    * @return the UDN (Unique Device Name) for the device.
    */
   public String getUdn()
   {
      return mUdn;
   }
   
   /**
    * Sets the UPC (Universal Product Code) for the device.
    * 
    * @param upc the UPC (Universal Product Code) for the device.
    */
   public void setUpc(String upc)
   {
      mUpc = upc;
   }
   
   /**
    * Gets the UPC (Universal Product Code) for the device.
    * 
    * @return the UPC (Universal Product Code) for the device.
    */
   public String getUpc()
   {
      return mUpc;
   }
   
   /**
    * Gets the icon list for this device.
    * 
    * @return the UPnPDeviceIconList for this device.
    */
   public UPnPDeviceIconList getIconList()
   {
      return mIconList;
   }
   
   /**
    * The service list for this device.
    * 
    * @return the UPnPServiceList for this device.
    */
   public UPnPServiceList getServiceList()
   {
      return mServiceList;
   }
   
   /**
    * The device list for this device. This is the list of embedded devices
    * for this device.
    * 
    * @return the UPnPDeviceList for this device.
    */
   public UPnPDeviceList getDeviceList()   
   {
      return mDeviceList;
   }
   
   /**
    * Sets the presentation URL for this device.
    * 
    * @param url the URL used to present this device. 
    */
   public void setPresentationUrl(String url)
   {
      mPresentationUrl = url;
   }
   
   /**
    * Gets the presentation URL for this device.
    * 
    * @return the URL used to present this device. 
    */
   public String getPresentationUrl()
   {
      return mPresentationUrl;
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
