/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A UPnPServiceDescription is an XML document that describes a UPnPService. 
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
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
 * 
 * @author Dave Longley
 */
public class UPnPServiceDescription extends AbstractXmlSerializer
{
   /**
    * The UPnPServiceActionList for this service description. If this
    * action list is empty, then the service this description is for
    * doesn't have any actions -- this is legal.
    */
   protected UPnPServiceActionList mActionList;
   
   /**
    * The UPnPServiceStateTable for this service description. Every
    * service MUST have a state table that has at least 1 state variable.
    */
   protected UPnPServiceStateTable mStateTable;
   
   /**
    * The XML namespace for this service descrption.
    */
   public static final String XML_NAMESPACE =
      "urn:schemas-upnp-org:service-1-0";
   
   /**
    * The major spec version for this service descrption.
    */
   public static final String MAJOR_VERSION = "1";
   
   /**
    * The minor spec version for this service descrption.
    */
   public static final String MINOR_VERSION = "0";
   
   /**
    * Creates a new UPnPServiceDescription.
    */
   public UPnPServiceDescription()
   {
      // create a new action list for this description
      mActionList = new UPnPServiceActionList();
      
      // create a new state table for this description
      mStateTable = new UPnPServiceStateTable();
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "scpd";
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
      XmlElement scpdElement = new XmlElement(getRootTag());
      scpdElement.setParent(parent);
      
      // add namespace attribute
      scpdElement.addAttribute("xmlns", XML_NAMESPACE);
      
      // add spec version element
      XmlElement specVersionElement = new XmlElement("specVersion");
      scpdElement.addChild(specVersionElement);
      
      // add major element
      XmlElement majorElement = new XmlElement("major");
      majorElement.setValue(MAJOR_VERSION);
      specVersionElement.addChild(majorElement);
      
      // add minor element
      XmlElement minorElement = new XmlElement("minor");
      minorElement.setValue(MINOR_VERSION);
      specVersionElement.addChild(minorElement);
      
      // convert the action list, if it has actions
      if(getActionList().getActionCount() > 0)
      {
         scpdElement.addChild(getActionList().convertToXmlElement(scpdElement));
      }
      
      // convert state table
      scpdElement.addChild(getStateTable().convertToXmlElement(scpdElement));      
      
      // return root element
      return scpdElement;
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
      boolean rval = false;
      
      // check namespace and spec version
      String namespace = element.getAttributeValue("xmlns");
      if(namespace.equals(XML_NAMESPACE) && element.hasChild("specVersion"))
      {
         // check major and minor spec versions
         XmlElement specElement = element.getFirstChild("specVersion");
         String major = specElement.getFirstChildValue("major");
         String minor = specElement.getFirstChildValue("minor");
            
         if(major.equals(MAJOR_VERSION) && minor.equals(MINOR_VERSION))
         {
            rval = true;
            
            // clear action list and state table
            getActionList().clear();
            getStateTable().clear();
            
            // get action list element, if any
            XmlElement actionListElement = element.getFirstChild("actionList");
            if(actionListElement != null)
            {
               rval = getActionList().convertFromXmlElement(actionListElement);
            }
            
            if(rval)
            {
               // get state table element
               XmlElement stateTableElement =
                  element.getFirstChild("serviceStateTable");
               rval = getStateTable().convertFromXmlElement(stateTableElement);
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the action list for this service description.
    * 
    * @return the UPnPServiceActionList for this service description.
    */
   public UPnPServiceActionList getActionList()
   {
      return mActionList;
   }
   
   /**
    * Gets the state table for this service description.
    * 
    * @return the UPnPServiceStateTable for this service description.
    */
   public UPnPServiceStateTable getStateTable()
   {
      return mStateTable;
   }
   
   /**
    * Gets the logger for this UPnPServiceDescription.
    * 
    * @return the logger for this UPnPServiceDescription.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
