/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_upnp_TypeDefinitions_H
#define db_upnp_TypeDefinitions_H

#include "db/rt/DynamicObject.h"
#include "db/rt/DynamicObjectIterator.h"

namespace db
{
namespace upnp
{

/**
 * A UPnP device represents a device that supports the UPnP protocol. It
 * provides UPnP services that a UPnP control point can use to control it.
 * 
 * The xml element names are used as properties of the DynamicObject. A
 * root UPnP device has the additional fields:
 * 
 * server   : OS and product version information about the device.
 * location : a URL to the device's description.
 * usn      : a Unique Service Name for the device.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org
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
 */
typedef db::rt::DynamicObject Device;
typedef db::rt::DynamicObject DeviceList;
typedef db::rt::DynamicObjectIterator DeviceIterator;

} // end namespace upnp
} // end namespace db

#endif
