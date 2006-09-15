/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A UPnPServiceStateTable is an XML serializable table for
 * UPnPServiceStateVariables.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * <pre>
 * 
 * <serviceStateTable>
 *   <stateVariable sendEvents="yes">
 *     <name>variableName</name>
 *     <dataType>variable data type</dataType>
 *     <defaultValue>default value</defaultValue>
 *     <allowedValueList>
 *       <allowedValue>enumerated value</allowedValue>
 *       Other allowed values defined by UPnP Forum working committee
 *       (if any) go here
 *     </allowedValueList>
 *   </stateVariable>
 *   <stateVariable sendEvents="yes">
 *     <name>variableName</name>
 *     <dataType>variable data type</dataType>
 *     <defaultValue>default value</defaultValue>
 *     <allowedValueRange>
 *       <minimum>minimum value</minimum>
 *       <maximum>maximum value</maximum>
 *       <step>increment value</step>
 *     </allowedValueRange>
 *   </stateVariable>
 *   Declarations for other state variables defined by UPnP Forum working
 *   committee (if any) go here
 *   Declarations for other state variables added by UPnP vendor (if any)
 *   go here
 * </serviceStateTable>
 * 
 * </pre>
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
 * -----------------------------------------------------------------------
 * 
 * @author Dave Longley
 */
public class UPnPServiceStateTable extends AbstractXmlSerializer
{
   /**
    * Creates a new UPnPServiceStateTable.
    */
   public UPnPServiceStateTable()
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
    * Gets the logger for this UPnPServiceStateTable.
    * 
    * @return the logger for this UPnPServiceStateTable.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
}
