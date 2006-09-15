/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.device;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A UPnPDeviceDescription is an XML document that describes a UPnPDevice.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * The UPnP description for a device contains several pieces of vendor-specific
 * information, definitions of all embedded devices, URL for presentation of
 * the device, and listings for all services, including URLs for control and
 * eventing. In addition to defining non-standard devices, UPnP vendors may add
 * embedded devices and services to standard devices. To illustrate these,
 * below is a listing with placeholders (in italics) for actual elements and
 * values. Some of these placeholders would be specified by a UPnP Forum
 * working committee (colored red) or by a UPnP vendor (purple). For a
 * non-standard device, all of these placeholders would be specified by a
 * UPnP vendor. (Elements defined by the UPnP Device Architecture are colored 
 * green for later reference.) Immediately following the listing is a detailed
 * explanation of the elements, attributes, and values.
 * 
 * <pre>
 * 
 * <?xml version="1.0"?>
 * <root xmlns="urn:schemas-upnp-org:device-1-0">
 *  <specVersion>
 *   <major>1</major>
 *   <minor>0</minor>
 *  </specVersion>
 *  <URLBase>base URL for all relative URLs</URLBase>
 *  <device>
 *    <deviceType>urn:schemas-upnp-org:device:deviceType:v</deviceType>
 *    <friendlyName>short user-friendly title</friendlyName>
 *    <manufacturer>manufacturer name</manufacturer>
 *    <manufacturerURL>URL to manufacturer site</manufacturerURL>
 *    <modelDescription>long user-friendly title</modelDescription>
 *    <modelName>model name</modelName>
 *    <modelNumber>model number</modelNumber>
 *    <modelURL>URL to model site</modelURL>
 *    <serialNumber>manufacturer's serial number</serialNumber>
 *    <UDN>uuid:UUID</UDN>
 *    <UPC>Universal Product Code</UPC>
 *    <iconList>
 *      <icon>
 *        <mimetype>image/format</mimetype>
 *        <width>horizontal pixels</width>
 *        <height>vertical pixels</height>
 *        <depth>color depth</depth>
 *        <url>URL to icon</url>
 *      </icon>
 *      XML to declare other icons, if any, go here
 *    </iconList>
 *    <serviceList>
 *      <service>
 *        <serviceType>urn:schemas-upnp-org:service:serviceType:v</serviceType>
 *        <serviceId>urn:upnp-org:serviceId:serviceID</serviceId>
 *        <SCPDURL>URL to service description</SCPDURL>
 *        <controlURL>URL for control</controlURL>
 *        <eventSubURL>URL for eventing</eventSubURL>
 *      </service>
 *      Declarations for other services defined by a UPnP Forum working
 *      committee (if any) go here
 *      Declarations for other services added by UPnP vendor (if any) go here
 *    </serviceList>
 *    <deviceList>
 *      Description of embedded devices defined by a UPnP Forum working
 *      committee (if any) go here
 *      Description of embedded devices added by UPnP vendor (if any) go here
 *    </deviceList>
 *    <presentationURL>URL for presentation</presentationURL>
 *  </device>
 * </root>
 * 
 * </pre>
 * 
 * root
 * Required. Must have urn:schemas-upnp-org:device-1-0 as the value for the
 * xmlns attribute; this references the UPnP Template Language (described
 * below). Case sensitive. Contains all other elements describing the root
 * device, i.e., contains the following sub elements:
 * 
 * specVersion
 * Required. Contains the following sub elements:
 * 
 * major
 * Required. Major version of the UPnP Device Architecture. Must be 1.
 * 
 * minor
 * Required. Minor version of the UPnP Device Architecture. Must be 0.
 * 
 * URLBase
 * Optional. Defines the base URL. Used to construct fully-qualified URLs. All
 * relative URLs that appear elsewhere in the description are appended to this
 * base URL. If URLBase is empty or not given, the base URL is the URL from
 * which the device description was retrieved. Specified by UPnP vendor.
 * Single URL.
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
 * 
 * serviceList
 * Required. Contains the following sub elements:
 * 
 * service
 * Required. Repeated once for each service defined by a UPnP Forum working 
 * committee. If UPnP vendor differentiates device by adding additional,
 * standard UPnP services, repeated once for additional service.
 * Contains the following sub elements:
 * 
 * serviceType
 * Required. UPnP service type. Must not contain a hash character (#, 23 Hex
 * in UTF-8).
 * 
 * For standard service types defined by a UPnP Forum working committee, must
 * begin with urn:schemas-upnp-org:service: followed by a service type suffix,
 * colon, and an integer service version (as shown in the listing above).
 * For non-standard service types specified by UPnP vendors, must begin with
 * urn:, followed by an ICANN domain name owned by the vendor, followed by
 * :service:, followed by a service type suffix, colon, and an integer service
 * version, i.e., urn:domain-name:service:serviceType:v.
 * The service type suffix defined by a UPnP Forum working committee or
 * specified by a UPnP vendor must be <= 64 characters, not counting the
 * version suffix and separating colon. Single URI.
 * 
 * serviceId
 * Required. Service identifier. Must be unique within this device description.
 * For standard services defined by a UPnP Forum working committee, must begin
 * with urn:upnp-org:serviceId: followed by a service ID suffix (as shown in
 * the listing above). (Note that upnp-org is used instead of schemas-upnp-org
 * in this case because an XML schema is not defined for each service ID.)
 * For non-standard services specified by UPnP vendors, must begin with urn:,
 * followed by an ICANN domain name owned by the vendor, followed by
 * :serviceId:, followed by a service ID suffix, i.e.,
 * urn:domain-name:serviceId:serviceID.
 * The service ID suffix defined by a UPnP Forum working committee or
 * specified by a UPnP vendor must be <= 64 characters. Single URI.
 * 
 * SCPDURL
 * Required. URL for service description (nee Service Control Protocol
 * Definition URL). (cf. section below on service description.) May be relative
 * to base URL. Specified by UPnP vendor. Single URL.
 * 
 * controlURL
 * Required. URL for control (cf. section on Control). May be relative to base
 * URL. Specified by UPnP vendor. Single URL.
 * 
 * eventSubURL
 * Required. URL for eventing (cf. section on Eventing). May be relative to
 * base URL. Must be unique within the device; no two services may have the
 * same URL for eventing. If the service has no evented variables, it should
 * not have eventing (cf. section on Eventing); if the service does not have
 * eventing, this element must be present but should be empty, i.e.,
 * <eventSubURL></eventSubURL>. Specified by UPnP vendor. Single URL.
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
 * 
 * For future extensibility, when processing XML like the listing above, as 
 * specified by the Flexible XML Processing Profile (FXPP), devices and control
 * points must ignore: (a) any unknown elements and their sub elements or
 * content, and (b) any unknown attributes and their values. 
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class UPnPDeviceDescription extends AbstractXmlSerializer
{
   /**
    * Creates a new UPnPDeviceDescription.
    */
   public UPnPDeviceDescription()
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
    * Gets the logger for this UPnPDeviceDescription.
    * 
    * @return the logger for this UPnPDeviceDescription.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }   
}
