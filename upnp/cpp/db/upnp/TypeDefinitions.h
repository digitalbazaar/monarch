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

// defines for device and service types
#define UPNP_DEVICE_TYPE_IGD \
   "urn:schemas-upnp-org:device:InternetGatewayDevice:1"
#define UPNP_DEVICE_TYPE_WAN \
   "urn:schemas-upnp-org:device:WANDevice:1"
#define UPNP_DEVICE_TYPE_WAN_CONNECTION \
   "urn:schemas-upnp-org:device:WANConnectionDevice:1"
#define UPNP_SERVICE_TYPE_WAN_IP_CONNECTION \
   "urn:schemas-upnp-org:service:WANIPConnection:1"

// defines for errors
/**
 * One of following: not enough IN arguments, too many IN arguments,
 * no IN argument by that name, one or more IN arguments are of the
 * wrong data type.
 */
#define UPNP_ERROR_InvalidArgs                      402

/**
 * The specified array index was invalid.
 */
#define UPNP_ERROR_SpecifiedArrayIndexInvalid       713

/**
 * There was no entry to delete that matched the passed parameters.
 */
#define UPNP_ERROR_NoSuchEntryInArray               714

/**
 * The source IP address cannot be wild-carded.
 */
#define UPNP_ERROR_WildCardNotPermittedInSrcIP      715

/**
 * The external port cannot be wild-carded.
 */
#define UPNP_ERROR_WildCardNotPermittedInExtPort    716

/**
 * The port mapping entry specified conflicts with a mapping assigned
 * previously to another client.
 */
#define UPNP_ERROR_ConflictInMappingEntry           718

/**
 * Internal and External port values must be the same.
 */
#define UPNP_ERROR_SamePortValuesRequired           724

/**
 * The NAT implementation only supports permanent lease times on port
 * mappings.
 */
#define UPNP_ERROR_OnlyPermanentLeasesSupported     725

/**
 * RemoteHost must be a wildcard and cannot be a specific IP address or
 * DNS name.
 */
#define UPNP_ERROR_RemoteHostOnlySupportsWildcard   726

/**
 * ExternalPort must be a wildcard and cannot be a specific port value.
 */
#define UPNP_ERROR_ExternalPortOnlySupportsWildcard 727

/**
 * Type definition information obtained from: http://www.upnp.org 
 */

/**
 * A UPnP device represents a device that supports the UPnP protocol. It
 * provides UPnP services that a UPnP control point can use to control it.
 * 
 * Any UPnP device whose description has been fetched as the following fields:
 * 
 * All fields (inclusive) "deviceType"-"UPC".
 * devices      : an array of sub-devices.
 * services     : an array of services.
 * rootURL      : the root URL to the device (includes schema+host+port)
 * 
 * A root UPnP device has the additional fields:
 * 
 * server      : OS and product version information about the device.
 * location    : a URL to the device's description.
 * usn         : a Unique Service Name for the device.
 * 
 * Below is the full XML description for a device. It is used to build
 * a simpler device object with the above properties.
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

/**
 * A UPnP Service represents a service that supports the UPnP protocol. A
 * UPnP Service is provided by a UPnP Device to give a UPnP ControlPoint
 * access to its functionality.
 * 
 * Any UPnP service has as the following fields (see below for docs):
 * serviceType
 * serviceId
 * SCPDURL
 * controlURL
 * eventSubURL
 * rootURL      : the root URL to the service (includes schema+host+port)
 * 
 * Any UPnP service whose description has been fetched as the following fields:
 * actions     : a map of Action name to Action.
 * 
 * Below is the full XML description for a service. It is used to build
 * a simpler service object with the above properties.
 * -----------------------------------------------------------------------
 * <pre>
 * 
 * <service>
 *   <serviceType>urn:schemas-upnp-org:service:serviceType:v</serviceType>
 *   <serviceId>urn:upnp-org:serviceId:serviceID</serviceId>
 *   <SCPDURL>URL to service description</SCPDURL>
 *   <controlURL>URL for control</controlURL>
 *   <eventSubURL>URL for eventing</eventSubURL>
 * </service>
 *     
 * </pre>
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
 * Required. URL for service description (see Service Control Protocol
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
 * -----------------------------------------------------------------------
 */
typedef db::rt::DynamicObject Service;
typedef db::rt::DynamicObject ServiceList;
typedef db::rt::DynamicObjectIterator ServiceIterator;

/**
 * A UPnP ServiceDescription is an XML document that describes a UPnP Service.
 * It is a Service Procotol Control Definition (SCPD).
 * -----------------------------------------------------------------------
 * The UPnP description for a service defines actions and their arguments,
 * and state variables and their data type, range, and event characteristics.
 * 
 * Each service may have zero or more actions. Each action may have zero or
 * more arguments. Any combination of these arguments may be input or output
 * parameters. If an action has one or more output arguments, one these
 * arguments may be marked as a return value. Each argument should correspond
 * to a state variable. This direct-manipulation programming model reinforces
 * simplicity.
 * 
 * Each service must have one or more state variables.
 * 
 * In addition to defining non-standard services, UPnP vendors may add actions
 * and services to standard devices.
 * 
 * To illustrate these points, below is a listing with placeholders (in italics)
 * for actual elements and values. For a standard UPnP service, some of these
 * placeholders would be defined by a UPnP Forum working committee (colored red)
 * or specified by a UPnP vendor (purple). For a non-standard service, all of
 * these placeholders would be specified by a UPnP vendor. (Elements defined by
 * the UPnP Device Architecture are colored green for later reference.)
 * Immediately following the listing is a detailed explanation of the elements,
 * attributes, and values.
 * 
 * <pre>
 * 
 * <?xml version="1.0"?>
 * <scpd xmlns="urn:schemas-upnp-org:service-1-0">
 *  <specVersion>
 *    <major>1</major>
 *    <minor>0</minor>
 *  </specVersion>
 *  <actionList>
 *    <action>
 *      <name>actionName</name>
 *      <argumentList>
 *        <argument>
 *          <name>formalParameterName</name>
 *          <direction>in xor out</direction>
 *          <retval />
 *          <relatedStateVariable>stateVariableName</relatedStateVariable>
 *        </argument>
 *       Declarations for other arguments defined by UPnP Forum working
 *       committee (if any) go here
 *      </argumentList>
 *    </action>
 *    Declarations for other actions defined by UPnP Forum working committee
 *    (if any) go here
 *    Declarations for other actions added by UPnP vendor (if any) go here
 *  </actionList>
 *  <serviceStateTable>
 *    <stateVariable sendEvents="yes">
 *      <name>variableName</name>
 *      <dataType>variable data type</dataType>
 *      <defaultValue>default value</defaultValue>
 *      <allowedValueList>
 *        <allowedValue>enumerated value</allowedValue>
 *        Other allowed values defined by UPnP Forum working committee
 *        (if any) go here
 *      </allowedValueList>
 *    </stateVariable>
 *    <stateVariable sendEvents="yes">
 *      <name>variableName</name>
 *      <dataType>variable data type</dataType>
 *      <defaultValue>default value</defaultValue>
 *      <allowedValueRange>
 *        <minimum>minimum value</minimum>
 *        <maximum>maximum value</maximum>
 *        <step>increment value</step>
 *      </allowedValueRange>
 *    </stateVariable>
 *    Declarations for other state variables defined by UPnP Forum working
 *    committee (if any) go here
 *    Declarations for other state variables added by UPnP vendor (if any)
 *    go here
 *  </serviceStateTable>
 * </scpd>
 * 
 * </pre>
 * 
 * scpd 
 * Required. Must have urn:schemas-upnp-org:service-1-0 as the value for the
 * xmlns attribute; this references the UPnP Template Language (explained
 * below). Case sensitive. Contains all other elements describing the service,
 * i.e., contains the following sub elements: 
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
 * actionList 
 * Required if and only if the service has actions. (Each service may have >= 0
 * actions.) Contains the following sub element(s): 
 * 
 * action 
 * Required. Repeat once for each action defined by a UPnP Forum working
 * committee. If UPnP vendor differentiates service by adding additional
 * actions, repeat once for each additional action. Contains the following
 * sub elements: 
 * 
 * name 
 * Required. Name of action. Must not contain a hyphen character (-, 2D Hex in
 * UTF-8) nor a hash character (#, 23 Hex in UTF-8).  
 * 
 * For standard actions defined by a UPnP Forum working committee, must not
 * begin with X_ nor A_.
 * 
 * For non-standard actions specified by a UPnP vendor and added to a standard
 * service, must begin with X_.
 * 
 * String. Should be < 32 characters. 
 * 
 * argumentList 
 * Required if and only if parameters are defined for action. (Each action may
 * have >= 0 parameters.) Contains the following sub element(s): 
 * 
 * argument 
 * Required. Repeat once for each parameter. Contains the following sub
 * elements: 
 * 
 * name 
 * Required. Name of formal parameter. Should be name of a state variable that
 * models an effect the action causes. Must not contain a hyphen character
 * (-, 2D Hex in UTF-8). String. Should be < 32 characters. 
 * 
 * direction 
 * Required. Whether argument is an input or output parameter. Must be in xor
 * out. Any in arguments must be listed before any out arguments. 
 * 
 * retval 
 * Optional. Identifies at most one out argument as the return value. If
 * included, must be the first out argument. (Element only; no value.) 
 * 
 * relatedStateVariable 
 * Required. Must be the name of a state variable. 
 * 
 * serviceStateTable 
 * Required. (Each service must have > 0 state variables.) Contains the
 * following sub element(s): 
 * 
 * stateVariable 
 * Required. Repeat once for each state variable defined by a UPnP Forum
 * working committee. If UPnP vendor differentiates service by adding
 * additional state variables, repeat once for each additional variable.
 * sendEvents attribute defines whether event messages will be generated when
 * the value of this state variable changes; non-evented state variables have
 * sendEvents="no"; default is sendEvents="yes". Contains the following sub
 * elements: 
 * 
 * name 
 * Required. Name of state variable. Must not contain a hyphen character
 * (-, 2D Hex in UTF-8).  
 * 
 * For standard variables defined by a UPnP Forum working committee, must not
 * begin with X_ nor A_. 
 * 
 * For non-standard variables specified by a UPnP vendor and added to a
 * standard service, must begin with X_. 
 * 
 * String. Should be < 32 characters. 
 * 
 * dataType 
 * Required. Same as data types defined by XML Schema, Part 2: Datatypes.
 * Defined by a UPnP Forum working committee for standard state variables;
 * specified by UPnP vendor for extensions. Must be one of the following values:
 * 
 * ui1 
 * Unsigned 1 Byte int. Same format as int without leading sign. 
 * 
 * ui2 
 * Unsigned 2 Byte int. Same format as int without leading sign. 
 * 
 * ui4 
 * Unsigned 4 Byte int. Same format as int without leading sign. 
 * 
 * i1 
 * 1 Byte int. Same format as int. 
 * 
 * i2 
 * 2 Byte int. Same format as int. 
 * 
 * i4 
 * 4 Byte int. Same format as int. Must be between -2147483648 and 2147483647. 
 * 
 * int 
 * Fixed point, integer number. May have leading sign. May have leading zeros.
 * (No currency symbol.) (No grouping of digits to the left of the decimal,
 * e.g., no commas.) 
 * 
 * r4 
 * 4 Byte float. Same format as float. Must be between 3.40282347E+38 to
 * 1.17549435E-38. 
 * 
 * r8 
 * 8 Byte float. Same format as float. Must be between -1.79769313486232E308
 * and -4.94065645841247E-324 for negative values, and between
 * 4.94065645841247E-324 and 1.79769313486232E308 for positive values, i.e.,
 * IEEE 64-bit (8-Byte) double. 
 * 
 * number 
 * Same as r8. 
 * 
 * fixed.14.4 
 * Same as r8 but no more than 14 digits to the left of the decimal point and
 * no more than 4 to the right. 
 * 
 * float 
 * Floating point number. Mantissa (left of the decimal) and/or exponent may
 * have a leading sign. Mantissa and/or exponent may have leading zeros.
 * Decimal character in mantissa is a period, i.e., whole digits in mantissa
 * separated from fractional digits by period. Mantissa separated from
 * exponent by E. (No currency symbol.) (No grouping of digits in the
 * mantissa, e.g., no commas.) 
 * 
 * char 
 * Unicode string. One character long. 
 * 
 * string 
 * Unicode string. No limit on length. 
 * 
 * date 
 * Date in a subset of ISO 8601 format without time data. 
 * 
 * dateTime 
 * Date in ISO 8601 format with optional time but no time zone. 
 * 
 * dateTime.tz 
 * Date in ISO 8601 format with optional time and optional time zone. 
 * 
 * time 
 * Time in a subset of ISO 8601 format with no date and no time zone. 
 * 
 * time.tz 
 * Time in a subset of ISO 8601 format with optional time zone but no date. 
 * 
 * boolean 
 * 0, false, or no for false; 1, true, or yes for true. 
 * 
 * bin.base64 
 * MIME-style Base64 encoded binary BLOB. Takes 3 Bytes, splits them into
 * 4 parts, and maps each 6 bit piece to an octet. (3 octets are encoded as
 * 4.) No limit on size. 
 * 
 * bin.hex 
 * Hexadecimal digits representing octets. Treats each nibble as a hex digit
 * and encodes as a separate Byte. (1 octet is encoded as 2.) No limit on size. 
 * 
 * uri 
 * Universal Resource Identifier. 
 * 
 * uuid 
 * Universally Unique ID. Hexadecimal digits representing octets. Optional
 * embedded hyphens are ignored. 
 * 
 * defaultValue 
 * Recommended. Expected, initial value. Defined by a UPnP Forum working
 * committee or delegated to UPnP vendor. Must match data type. Must satisfy
 * allowedValueList or allowedValueRange constraints. 
 * 
 * allowedValueList 
 * Recommended. Enumerates legal string values. Prohibited for data types
 * other than string. At most one of allowedValueRange and allowedValueList
 * may be specified. Sub elements are ordered (e.g., see NEXT_STRING_BOUNDED).
 * Contains the following sub elements: 
 * 
 * allowedValue 
 * Required. A legal value for a string variable. Defined by a UPnP Forum
 * working committee for standard state variables; specified by UPnP vendor
 * for extensions. string. Should be < 32 characters. 
 * 
 * allowedValueRange 
 * Recommended. Defines bounds for legal numeric values; defines resolution 
 * for numeric values. Defined only for numeric data types. At most one of
 * allowedValueRange and allowedValueList may be specified. Contains the
 * following sub elements: 
 * 
 * minimum 
 * Required. Inclusive lower bound. Defined by a UPnP Forum working committee
 * or delegated to UPnP vendor. Single numeric value. 
 * 
 * maximum 
 * Required. Inclusive upper bound. Defined by a UPnP Forum working committee
 * or delegated to UPnP vendor. Single numeric value. 
 * 
 * step 
 * Recommended. Size of an increment operation, i.e., value of s in the
 * operation v = v + s. Defined by a UPnP Forum working committee or delegated
 * to UPnP vendor. Single numeric value.
 *  
 * For future extensibility, when processing XML like the listing above, as
 * specified by the Flexible XML Processing Profile (FXPP), devices and control
 * points must ignore: (a) any unknown elements and their sub elements or
 * content, and (b) any unknown attributes and their values.
 * -----------------------------------------------------------------------
 */
typedef db::rt::DynamicObject ServiceDescription;

/**
 * A UPnP ServiceAction is an XML serializable action that a UPnP Service can
 * perform.
 * 
 * An Action has these properties, as defined below:
 * name         : the name of the action.
 * arguments    : a map with "in", "out", and "retval" argument names
 * 
 * Below is the full XML description for an action. It is used to build
 * a simpler action object with the above properties.
 * -----------------------------------------------------------------------
 * <pre>
 * 
 * <action>
 *   <name>actionName</name>
 *   <argumentList>
 *     <argument>
 *       <name>formalParameterName</name>
 *       <direction>in xor out</direction>
 *       <retval />
 *       <relatedStateVariable>stateVariableName</relatedStateVariable>
 *     </argument>
 *    Declarations for other arguments defined by UPnP Forum working
 *    committee (if any) go here
 *   </argumentList>
 * </action>
 * 
 * </pre>
 * 
 * action 
 * Required. Repeat once for each action defined by a UPnP Forum working
 * committee. If UPnP vendor differentiates service by adding additional
 * actions, repeat once for each additional action. Contains the following
 * sub elements: 
 * 
 * name 
 * Required. Name of action. Must not contain a hyphen character (-, 2D Hex in
 * UTF-8) nor a hash character (#, 23 Hex in UTF-8).  
 * 
 * For standard actions defined by a UPnP Forum working committee, must not
 * begin with X_ nor A_.
 * 
 * For non-standard actions specified by a UPnP vendor and added to a standard
 * service, must begin with X_.
 * 
 * String. Should be < 32 characters. 
 * 
 * argumentList 
 * Required if and only if parameters are defined for action. (Each action may
 * have >= 0 parameters.) Contains the following sub element(s): 
 * -----------------------------------------------------------------------
 */
typedef db::rt::DynamicObject Action;
typedef db::rt::DynamicObjectIterator ActionIterator;

/**
 * An ActionResult is a map of named return values.
 */
typedef db::rt::DynamicObject ActionResult;

/**
 * A PortMapping object is used to add or remove a port mapping entry
 * on an internet gateway device. A blank IP address for the NewRemoteHost
 * will use the default external IP address of the internet gateway device.
 * 
 * If adding a port mapping, all parameters are used. If removing a port
 * mapping, only the first three (NewRemoteHost, NewExternalPort, NewProtocol)
 * are used.
 * 
 * @param NewRemoteHost the external IP address ("x.x.x.x" or "").
 * @param NewExternalPort the external port clients connect to.
 * @param NewProtocol either "TCP" or "UDP".
 * @param NewInternalPort the port on the internal server.
 * @param NewInternalClient the internal IP address to redirect ("x.x.x.x").
 * @param NewEnabled true or false.
 * @param NewPortMappingDescription a string describing the mapping.
 * @param NewLeaseDuration 0 for infinite, X seconds for lease.
 */
typedef db::rt::DynamicObject PortMapping;

} // end namespace upnp
} // end namespace db

#endif
