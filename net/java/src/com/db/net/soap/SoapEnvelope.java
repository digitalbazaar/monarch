/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.net.soap;

import java.util.LinkedList;
import java.util.List;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.xml.AbstractXmlSerializer;
import com.db.xml.IXmlSerializer;
import com.db.xml.XmlElement;
import com.db.xml.XmlException;

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
   protected List<IXmlSerializer> mHeaderBlockSerializers;

   /**
    * The list of IXmlSerializers used to convert the body content sub-elements
    * for the body of the SOAP envelope to or from XML.
    */
   protected List<IXmlSerializer> mBodyContentSerializers;
   
   /**
    * The encoding style URI for this envelope.
    */
   protected String mEncodingStyleUri;
   
   /**
    * These URIs point to the XML schema that defines the structure of a
    * SOAP envelope.
    */
   public static final String[] SOAP_ENVELOPE_URIS =
   {
      "http://schemas.xmlsoap.org/soap/envelope/",
      "http://www.w3.org/2003/05/soap-envelope"
   };
   
   /**
    * These URIs point to the XML schema that defines how to encode information
    * in XML for SOAP transportation.
    */
   public static final String[] SOAP_ENCODING_URIS =
   {
      "http://schemas.xmlsoap.org/soap/encoding/",
      "http://www.w3.org/2003/05/soap-encoding"
   };
   
   /**
    * Creates a new SoapEnvelope.
    */
   public SoapEnvelope()
   {
      // create the header block serializers list
      mHeaderBlockSerializers = new LinkedList<IXmlSerializer>();
      
      // create the body content serializers list
      mBodyContentSerializers = new LinkedList<IXmlSerializer>();
      
      // defaults to no special encoding style
      setEncodingStyle(null);
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   @Override
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
   @Override
   public XmlElement convertToXmlElement(XmlElement parent)
   {
      // create the root xml element with the SOAP envelope namespace
      XmlElement envelopeElement = new XmlElement(
         getRootTag(), SOAP_ENVELOPE_URIS[0]);
      envelopeElement.setParent(parent);
      
      // use a namespace prefix of "soapenv" for the soap envelope elements
      String prefix = "soapenv";
      
      // add an attribute defining the namespace prefix
      envelopeElement.addAttribute("xmlns:" + prefix, SOAP_ENVELOPE_URIS[0]);
      
      // add an encoding style attribute (in the SOAP envelope namespace),
      // if applicable
      if(getEncodingStyle() != null && getEncodingStyle().length() > 0)
      {
         envelopeElement.addAttribute(
            "encodingStyle", getEncodingStyle(),
            SoapEnvelope.SOAP_ENVELOPE_URIS[0]);
      }
      
      // if there are header serializers present, then add a header element
      // and serialize the header blocks
      if(mHeaderBlockSerializers.size() > 0)
      {
         // add the header element
         XmlElement headerElement = new XmlElement("Header");
         envelopeElement.addChild(headerElement);
         
         // add each header block
         for(IXmlSerializer serializer: mHeaderBlockSerializers)
         {
            headerElement.addChild(
               serializer.convertToXmlElement(headerElement));
         }
      }
      
      // if there are body content serializers present, then add a body
      // element and its contents
      if(mBodyContentSerializers.size() > 0)
      {
         // add the body element
         XmlElement bodyElement = new XmlElement("Body");
         envelopeElement.addChild(bodyElement);
         
         // add each body sub-element
         for(IXmlSerializer serializer: mBodyContentSerializers)
         {
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
    * @exception XmlException thrown if this object could not be converted from
    *                         xml.
    */
   @Override
   public void convertFromXmlElement(XmlElement element) throws XmlException
   {
      super.convertFromXmlElement(element);
      
      // clear header block serializers
      mHeaderBlockSerializers.clear();
      
      // clear body content serializers
      mBodyContentSerializers.clear();
      
      // get the encoding style, if any
      String encodingStyle = "";
      for(String uri: SoapEnvelope.SOAP_ENCODING_URIS)
      {
         encodingStyle = element.getAttributeValue("encodingStyle", uri);
         
         if(!encodingStyle.equals(""))
         {
            break;
         }
      }
      
      // convert the header of the envelope, if any
      XmlElement headerElement = null;
      for(String uri: SoapEnvelope.SOAP_ENVELOPE_URIS)
      {
         headerElement = element.getFirstChild("Header", uri);
         if(headerElement != null)
         {
            break;
         }
      }
      
      if(headerElement != null)
      {
         getLogger().detail(getClass(), "found soap envelope header...");
         
         // add all children as header block serializers
         for(XmlElement headerBlock: headerElement.getChildren())
         {
            addHeaderBlockSerializer(headerBlock);
         }
      }
      
      // convert the body of the envelope, if any
      XmlElement bodyElement = null;
      for(String uri: SoapEnvelope.SOAP_ENVELOPE_URIS)
      {
         bodyElement = element.getFirstChild("Body", uri);
         if(bodyElement != null)
         {
            break;
         }
      }
      
      if(bodyElement != null)
      {
         getLogger().detail(getClass(), "found soap envelope body...");
         
         // add all children as body content serializers
         for(XmlElement bodyContent: bodyElement.getChildren())
         {
            addBodyContentSerializer(bodyContent);
         }
      }
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
    * Gets the header blocks (as IXmlSerializer) from this SOAP envelope's
    * header.
    * 
    * @return a vector of IXmlSerializer that make up the header.
    */
   public List<IXmlSerializer> getHeaderBlocks()
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
   public List<IXmlSerializer> getBodyContents()
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
    * Sets the encoding style URI for this envelope.
    * 
    * @param encodingStyleUri the URI for the encoding style for this envelope.
    */
   public void setEncodingStyle(String encodingStyleUri)
   {
      mEncodingStyleUri = encodingStyleUri;
   }
   
   /**
    * Gets the encoding style URI for this envelope.
    * 
    * @return the URI for the encoding style for this envelope.
    */
   public String getEncodingStyle()
   {
      return mEncodingStyleUri;
   }   
   
   /**
    * Gets the logger for this SoapEnvelope.
    * 
    * @return the logger for this SoapEnvelope.
    */
   @Override
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbnet");
   }
}
