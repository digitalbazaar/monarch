/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.util.Iterator;
import java.util.Vector;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.IXmlSerializer;
import com.db.xml.XmlElement;

/**
 * A SoapEnvelope is an XML container for SOAP (Simple Object Access Protocol)
 * messages.
 * 
 * To add header blocks or body content to this SoapEnvelope, add an
 * IXmlSerializer by calling the appropriate method:
 * 
 * addHeaderBlockSerializer() or addBodyContentSerializer()
 * 
 * To get header blocks or body content from this SoapEnvelope AFTER
 * converting from XML, call:
 * 
 * getHeaderBlocks() or getBodyContents()
 * 
 * Both of which will return a vector with IXmlSerializers. The IXmlSerializers
 * will be XmlElements after a call to convertFromXml().
 * 
 * FUTURE CODE: When we move over to SOAP 1.2 or (even before that if we
 * add complex object support) we want a clean redesign of the soap classes
 * and interfaces to make it more streamlined and easy to use. This may
 * include created a new interface for serializing complex objects to xml.  
 * 
 * @author Dave Longley
 */
public class SoapEnvelope extends AbstractXmlSerializer
{
   /**
    * The list of IXmlSerializers used to convert the header blocks for the
    * header of the SOAP envelope to or from XML.
    */
   protected Vector mHeaderBlockSerializers;

   /**
    * The list of IXmlSerializers used to convert the body content sub-elements
    * for the body of the SOAP envelope to or from XML.
    */
   protected Vector mBodyContentSerializers;
   
   /**
    * This URI points to the XML schema that defines the structure of a
    * SOAP envelope.
    */
   public static final String SOAP_ENVELOPE_URI =
      "http://schemas.xmlsoap.org/soap/envelope/";
   
   /**
    * This URI points to the XML schema that defines how to encode information
    * in XML for SOAP transportation.
    */
   public static final String SOAP_ENCODING_URI =
      "http://schemas.xmlsoap.org/soap/encoding/";
   
   /**
    * Creates a new SoapEnvelope.
    */
   public SoapEnvelope()
   {
      // create the header block serializers list
      mHeaderBlockSerializers = new Vector();
      
      // create the body content serializers list
      mBodyContentSerializers = new Vector();
   }
   
   /**
    * Adds a header block serializer for converting SOAP header blocks to
    * XML.
    * 
    * @param serializer the IXmlSerializer to use to convert a header block.
    */
   public void addHeaderBlockSerializer(IXmlSerializer serializer)
   {
      mHeaderBlockSerializers.add(serializer);
   }
   
   /**
    * Gets the header blocks (as XmlElements) from this SOAP envelope's
    * header.
    * 
    * @return a vector of XmlElements that make up the header.
    */
   public Vector getHeaderBlocks()
   {
      return mHeaderBlockSerializers;
   }
   
   /**
    * Clears the header blocks for this SOAP envelope.
    */
   public void clearHeaderBlocks()
   {
      mHeaderBlockSerializers.clear();
   }
   
   /**
    * Adds a body content serializer for converting SOAP body content to XML.
    * 
    * @param serializer the IXmlSerializer to use to convert body content.
    */
   public void addBodyContentSerializer(IXmlSerializer serializer)
   {
      mBodyContentSerializers.add(serializer);
   }
   
   /**
    * Gets the body content sub-elements (as XmlElements) from this SOAP
    * envelope's body.
    * 
    * @return a vector of XmlElements that make up the body content.
    */
   public Vector getBodyContents()
   {
      return mBodyContentSerializers;
   }
   
   /**
    * Clears the body contents for this SOAP envelope.
    */
   public void clearBodyContents()
   {
      mBodyContentSerializers.clear();
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      // this is according to the soap envelope schema
      return "Envelope";
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
      // use a namespace prefix of "soapenv" for the soap envelope elements
      String prefix = "soapenv";

      // create the root xml element, using the namespace prefix and the
      // SOAP envelope URI that points to the SOAP envelope XML schema
      XmlElement envelopeElement = new XmlElement(
         getRootTag(), prefix, SOAP_ENVELOPE_URI);
      envelopeElement.setParent(parent);
      
      // add an attribute defining the "soapenv" namespace prefix
      envelopeElement.addAttribute(prefix, SOAP_ENVELOPE_URI, "xmlns", null);
      
      // if there are header serializers present, then add a header element
      // and serialize the header blocks
      if(mHeaderBlockSerializers.size() > 0)
      {
         // add the header element
         XmlElement headerElement =
            new XmlElement("Header", prefix, SOAP_ENVELOPE_URI);
         envelopeElement.addChild(headerElement);
         
         // add each header block
         for(Iterator i = mHeaderBlockSerializers.iterator(); i.hasNext();)
         {
            IXmlSerializer serializer = (IXmlSerializer)i.next();
            headerElement.addChild(
               serializer.convertToXmlElement(headerElement));
         }
      }
      
      // if there are body content serializers present, then add a body
      // element and its contents
      if(mBodyContentSerializers.size() > 0)
      {
         // add the body element
         XmlElement bodyElement =
            new XmlElement("Body", prefix, SOAP_ENVELOPE_URI);
         envelopeElement.addChild(bodyElement);
         
         // add each body sub-element
         for(Iterator i = mBodyContentSerializers.iterator(); i.hasNext();)
         {
            IXmlSerializer serializer = (IXmlSerializer)i.next();
            bodyElement.addChild(serializer.convertToXmlElement(bodyElement));
         }
      }
      
      // return envelope element
      return envelopeElement;
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
      
      // clear header block serializers
      mHeaderBlockSerializers.clear();
      
      // clear body content serializers
      mBodyContentSerializers.clear();
      
      // convert the header of the envelope, if any
      XmlElement headerElement = element.getFirstChild(
         "Header", SOAP_ENVELOPE_URI);
      if(headerElement != null)
      {
         getLogger().detail(getClass(), "found soap envelope header...");
         
         // add all children as header block serializers
         for(Iterator i = headerElement.getChildren().iterator(); i.hasNext();)
         {
            XmlElement headerBlock = (XmlElement)i.next();
            addHeaderBlockSerializer(headerBlock);
         }
      }
      
      // convert the body of the envelope, if any
      XmlElement bodyElement = element.getFirstChild("Body", SOAP_ENVELOPE_URI);
      if(bodyElement != null)
      {
         getLogger().detail(getClass(), "found soap envelope body...");
         
         // add all children as body content serializers
         for(Iterator i = bodyElement.getChildren().iterator(); i.hasNext();)
         {
            XmlElement bodyContent = (XmlElement)i.next();
            addBodyContentSerializer(bodyContent);
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the logger for this SoapEnvelope.
    * 
    * @return the logger for this SoapEnvelope.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
