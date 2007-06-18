/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.upnp.service;

import java.text.DateFormat;
import java.util.LinkedList;
import java.util.List;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.XmlElement;
import com.db.xml.XmlException;

/**
 * A UPnPServiceStateVariable is an XML serializable state variable for
 * an UPnPService.
 * 
 * The following is taken from:
 * 
 * http://www.upnp.org/download/UPnPDA10_20000613.htm
 * 
 * -----------------------------------------------------------------------
 * <pre>
 * 
 * <stateVariable sendEvents="yes">
 *   <name>variableName</name>
 *   <dataType>variable data type</dataType>
 *   <defaultValue>default value</defaultValue>
 *   <allowedValueList>
 *     <allowedValue>enumerated value</allowedValue>
 *     Other allowed values defined by UPnP Forum working committee
 *     (if any) go here
 *   </allowedValueList>
 * </stateVariable>
 * 
 * --XOR--
 * 
 * <stateVariable sendEvents="yes">
 *   <name>variableName</name>
 *   <dataType>variable data type</dataType>
 *   <defaultValue>default value</defaultValue>
 *   <allowedValueRange>
 *     <minimum>minimum value</minimum>
 *     <maximum>maximum value</maximum>
 *     <step>increment value</step>
 *   </allowedValueRange>
 * </stateVariable>
 * 
 * </pre>
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
public class UPnPServiceStateVariable extends AbstractXmlSerializer
{
   /**
    * The name for this state variable.
    */
   protected String mName;
   
   /**
    * Set to "yes" if events are sent when this variable changes, set to
    * "no" if not.
    */
   protected String mSendEvents; 
   
   /**
    * The data type for this state variable.
    */
   protected String mDataType;
   
   /**
    * The default value for this state variable. Null if there is no default
    * value.
    */
   protected String mDefaultValue;
   
   /**
    * The list of allowed values for this state variable.
    */
   protected List<String> mAllowedValues;
   
   /**
    * The minimum value for this state variable. Null if there is no minimum.
    */
   protected String mMinimumValue;
   
   /**
    * The maximum value for this state variable. Null if there is no maximum.
    */
   protected String mMaximumValue;
   
   /**
    * The step value for this state variable. Null if there is no step.
    */
   protected String mStep;
   
   /**
    * Creates a new UPnPServiceStateVariable.
    */
   public UPnPServiceStateVariable()
   {
      // set defaults
      setName("");
      setSendEvents(true);
      setDataType("string");
      setDefaultValue(null);
      setMinimumValue(null);
      setMaximumValue(null);
      setStep(null);
      
      // create allowed values list
      mAllowedValues = new LinkedList<String>();
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   @Override
   public String getRootTag()   
   {
      return "stateVariable";
   }
   
   /**
    * Creates an XmlElement from this object.
    *
    * @param parent the parent XmlElement for the XmlElement being created
    *               (can be null). 
    * 
    * @return the XmlElement that represents this object.
    */
   @Override
   public XmlElement convertToXmlElement(XmlElement parent)   
   {
      // create the state variable element
      XmlElement stateVariableElement = new XmlElement(getRootTag());
      stateVariableElement.setParent(parent);
      
      // add send events attribute
      stateVariableElement.addAttribute("sendEvents", getSendEvents());
      
      // add name element
      XmlElement nameElement = new XmlElement("name");
      nameElement.setValue(getName());
      stateVariableElement.addChild(nameElement);
      
      // add data type element
      XmlElement dataTypeElement = new XmlElement("dataType");
      dataTypeElement.setValue(getDataType());
      stateVariableElement.addChild(dataTypeElement);
      
      // add default value element, if applicable
      if(getDefaultValue() != null)
      {
         XmlElement defaultValueElement = new XmlElement("defaultValue");
         defaultValueElement.setValue(getDefaultValue());
         stateVariableElement.addChild(defaultValueElement);
      }
      
      // add allowed value list, if applicable,
      // otherwise add allowed value range, if applicable
      if(hasAllowedValueList())
      {
         XmlElement allowedValueListElement =
            new XmlElement("allowedValueList");
         stateVariableElement.addChild(allowedValueListElement);
         
         // add each allowed value
         for(String value: getAllowedValues())
         {
            XmlElement allowedValueElement = new XmlElement("allowedValue");
            allowedValueElement.setValue(value);
            allowedValueListElement.addChild(allowedValueElement);
         }
      }
      else if(hasAllowedValueRange())
      {
         // add allowed value range
         XmlElement allowedValueRangeElement =
            new XmlElement("allowedValueRange");
         stateVariableElement.addChild(allowedValueRangeElement);
         
         // add minimum value element
         XmlElement minimumValueElement = new XmlElement("minimumValue");
         minimumValueElement.setValue(getMinimumValue());
         allowedValueRangeElement.addChild(minimumValueElement);
         
         // add maximum value element
         XmlElement maximumValueElement = new XmlElement("maximumValue");
         maximumValueElement.setValue(getMaximumValue());
         allowedValueRangeElement.addChild(maximumValueElement);
         
         // add step element, if applicable
         if(getStep() != null)
         {
            XmlElement stepElement = new XmlElement("step");
            stepElement.setValue(getStep());
            allowedValueRangeElement.addChild(stepElement);
         }
      }
      
      // return root element
      return stateVariableElement;
   }
   
   /**
    * Converts this object from an XmlElement.
    *
    * @param element the XmlElement to convert from.
    * 
    * @exception XmlException thrown if this object could not be converted from
    *                         xml.
    */
   @Override
   public void convertFromXmlElement(XmlElement element) throws XmlException
   {
      super.convertFromXmlElement(element);
      
      // get the send events attribute
      String sendEvents = element.getAttributeValue("sendEvents");
      setSendEvents(sendEvents.equals("yes"));
      
      // get the name element
      setName(element.getFirstChildValue("name"));
      
      // get the data type element
      setDataType(element.getFirstChildValue("dataType"));
      
      // get default value element, if any
      XmlElement defaultValueElement = element.getFirstChild("defaultValue");
      if(defaultValueElement != null)
      {
         setDefaultValue(defaultValueElement.getValue());
      }
      else
      {
         // no default value
         setDefaultValue(null);
      }
      
      // clear allowed values
      getAllowedValues().clear();
      
      // clear allowed value range
      setMinimumValue(null);
      setMaximumValue(null);
      setStep(null);
      
      // add allowed value list, if any
      if(getDataType().equals("string"))
      {
         XmlElement allowedValueListElement =
            element.getFirstChild("allowedValueList");
         if(allowedValueListElement != null)
         {
            // add each allowed value
            for(XmlElement allowedValueElement:
                allowedValueListElement.getChildren("allowedValue"))
            {
               addAllowedValue(allowedValueElement.getValue());
            }
         }
      }
      else
      {
         // add allowed value range, if any
         XmlElement allowedValueRangeElement =
            element.getFirstChild("allowedValueRange");
         if(allowedValueRangeElement != null)
         {
            // get minimum value element
            setMinimumValue(
               allowedValueRangeElement.getFirstChildValue("minimumValue"));
            
            // add maximum value element
            setMaximumValue(
               allowedValueRangeElement.getFirstChildValue("maximumValue"));
            
            // add step element, if applicable
            if(getStep() != null)
            {
               setStep(allowedValueRangeElement.getFirstChildValue("step"));
            }
         }
      }
   }
   
   /**
    * Sets the name for this state variable. The name will be truncated at 31
    * characters.
    * 
    * @param name the name for this state variable.
    */
   public void setName(String name)
   {
      if(name == null)
      {
         name = "";
      }
      else if(name.length() > 31)
      {
         name = name.substring(0, 32);
      }
      
      mName = name;
   }
   
   /**
    * Gets the name for this state variable.
    * 
    * @return the name for this state variable.
    */
   public String getName()
   {
      return mName;
   }
   
   /**
    * Sets whether or not events are sent when this variable changes.
    * 
    * @param yes true if events are sent when this variable changes, false
    *            if not.
    */
   public void setSendEvents(boolean yes)
   {
      if(yes)
      {
         mSendEvents = "yes";
      }
      else
      {
         mSendEvents = "no";
      }
   }
   
   /**
    * Gets whether or not events are sent when this variable changes.
    * 
    * @return "yes" if events are sent when this variable changes, "no" if
    *         not.
    */
   public String getSendEvents()
   {
      return mSendEvents;
   }
   
   /**
    * Sets the data type for this state variable.
    * 
    * @param dataType the data type for this state variable.
    */
   public void setDataType(String dataType)
   {
      mDataType = dataType;
   }
   
   /**
    * Gets the data type for this state variable.
    * 
    * @return the data type for this state variable.
    */
   public String getDataType()
   {
      return mDataType;
   }
   
   /**
    * Sets the default value for this state variable. Null if there is no
    * default value.
    * 
    * @param defaultValue the default value for this state variable.
    */
   public void setDefaultValue(String defaultValue)
   {
      mDefaultValue = defaultValue;
   }
   
   /**
    * Gets the default value for this state variable. Null if there is no
    * default value.
    * 
    * @return the default value for this state variable.
    */
   public String getDefaultValue()
   {
      return mDefaultValue;
   }
   
   /**
    * Adds an allowed value for this state variable.
    * 
    * @param value the allowed value to add.
    */
   public void addAllowedValue(String value)
   {
      getAllowedValues().add(value);
   }
   
   /**
    * Removes an allowed value from this state variable.
    * 
    * @param value the allowed value to remove.
    */
   public void removeAllowedValue(String value)
   {
      getAllowedValues().remove(value);
   }
   
   /**
    * Gets the list of allowed values for this state variable.
    * 
    * @return the list of allowed values for this state variable.
    */
   public List<String> getAllowedValues()
   {
      return mAllowedValues;
   }
   
   /**
    * Clears the list of allowed values for this state variable.
    */
   public void clearAllowedValues()
   {
      getAllowedValues().clear();
   }
   
   /**
    * Returns true if this state variable has a list of allowed values,
    * false if not.
    * 
    * @return true if this state variable has a list of allowed values,
    *         false if not.
    */
   public boolean hasAllowedValueList()
   {
      return getDataType().equals("string") && getAllowedValues().size() > 0;
   }
   
   /**
    * Sets the minimum value for this state variable. Null if there is no
    * minimum.
    * 
    * @param min the minimum value for this state variable.
    */
   public void setMinimumValue(String min)
   {
      mMinimumValue = min;
   }

   /**
    * Gets the minimum value for this state variable. Null if there is no
    * minimum.
    * 
    * @return the minimum value for this state variable.
    */
   public String getMinimumValue()
   {
      return mMinimumValue;
   }
   
   /**
    * Sets the maximum value for this state variable. Null if there is no
    * maximum.
    * 
    * @param max the maximum value for this state variable.
    */
   public void setMaximumValue(String max)
   {
      mMaximumValue = max;
   }   
   
   /**
    * Gets the maximum value for this state variable. Null if there is no
    * maximum.
    * 
    * @return the maximum value for this state variable.
    */
   public String getMaximumValue()
   {
      return mMaximumValue;
   }
   
   /**
    * Sets the step value for this state variable. Null if there is no step.
    * 
    * @param step the step value for this state variable.
    */
   public void setStep(String step)
   {
      mStep = step;
   }

   /**
    * Gets the step value for this state variable. Null if there is no step.
    * 
    * @return the step value for this state variable.
    */
   public String getStep()
   {
      return mStep;
   }
   
   /**
    * Returns true if this state variable has a specified allowed value range,
    * false if not.
    * 
    * @return true if this state variable has a specified allowed value range,
    *         false if not.
    */
   public boolean hasAllowedValueRange()
   {
      // minimum value must be set if there is an allowed value range set
      return mMinimumValue != null;
   }
   
   /**
    * Gets the logger for this UPnPServiceStateVariable.
    * 
    * @return the logger for this UPnPServiceStateVariable.
    */
   @Override
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbupnp");
   }
   
   /**
    * Converts a String value into the appropriate java type.
    * 
    * @param value the String value to convert to the appropriate java type.
    * @param dataType the data type to convert to.
    *  
    * @return the appropriate java type.
    */
   public static Object convertType(String value, String dataType)
   {
      Object rval = null;
      
      try
      {
         if(dataType.equals("ui1"))
         {
            rval = new Byte(value);
         }
         else if(dataType.equals("ui2"))
         {
            rval = new Short(value);
         }
         else if(dataType.equals("ui4"))
         {
            rval = new Integer(value);
         }
         else if(dataType.equals("i1"))
         {
            rval = new Byte(value);
         }
         else if(dataType.equals("i2"))
         {
            rval = new Short(value);
         }
         else if(dataType.equals("i4"))
         {
            rval = new Integer(value);
         }
         else if(dataType.equals("int"))
         {
            rval = new Integer(value);
         }
         else if(dataType.equals("r4"))
         {
            rval = new Float(value);
         }
         else if(dataType.equals("r8"))
         {
            rval = new Double(value);
         }
         else if(dataType.equals("number"))
         {
            rval = new Double(value);
         }
         else if(dataType.equals("fixed.14.4"))
         {
            rval = new Double(value);
         }
         else if(dataType.equals("float"))
         {
            rval = new Float(value);
         }
         else if(dataType.equals("char"))
         {
            rval = new Character(value.charAt(0));
         }
         else if(dataType.equals("string"))
         {
            rval = value;
         }
         else if(dataType.equals("date"))
         {
            DateFormat df = DateFormat.getDateInstance();
            rval = df.parse(value);
         }
         else if(dataType.equals("dateTime"))
         {
            DateFormat df = DateFormat.getDateTimeInstance();
            rval = df.parse(value);
         }
         else if(dataType.equals("dateTime.tz"))
         {
            DateFormat df = DateFormat.getDateTimeInstance();
            rval = df.parse(value);
         }
         else if(dataType.equals("time"))
         {
            DateFormat df = DateFormat.getTimeInstance();
            rval = df.parse(value);
         }
         else if(dataType.equals("time.tz"))
         {
            DateFormat df = DateFormat.getTimeInstance();
            rval = df.parse(value);
         }
         else if(dataType.equals("boolean"))
         {
            if(value.equals("0"))
            {
               rval = new Boolean(false);
            }
            else if(value.equals("1"))
            {
               rval = new Boolean(true);
            }
            else
            {
               rval = new Boolean(value);
            }
         }
         else if(dataType.equals("bin.base64"))
         {
            rval = value;
         }
         else if(dataType.equals("bin.hex"))
         {
            rval = value;
         }
         else if(dataType.equals("uri"))
         {
            rval = value;
         }
      }
      catch(Throwable ignore)
      {
         // just return null object
      }
      
      return rval;
   }
}
