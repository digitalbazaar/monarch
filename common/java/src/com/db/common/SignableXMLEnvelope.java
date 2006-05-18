/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import com.db.common.logging.Logger;
import com.db.common.logging.LoggerManager;

import java.io.StringReader;
import java.security.PrivateKey;
import java.util.Iterator;
import java.util.Vector;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

import org.xml.sax.InputSource;
import org.xml.sax.SAXParseException;

/**
 * A SignableXMLEnvelope is a transferrable container that allows
 * any object that implements the IXMLSerializer interface to be
 * transported in a secure, signed vessel.
 * 
 * This envelope may be used in either "signed" or "unsigned" form to
 * transport information. Once sign() is called on this envelope, however,
 * its text contents will become locked until sign() is called again.
 * 
 * This ensures the integrity of the signature generated when sign() was
 * called such that the text contents of this envelope will match that
 * signature regardless of any changes to the xml serializer contents of
 * this envelope. If you want to see the changes reflected, then you must
 * re-sign the envelope.
 * 
 * If an envelope is never signed, then it will remain in an "unsigned" state
 * and any changes to the xml serializer contents of that envelope will be
 * instantly reflected.
 * 
 * So:
 * 
 * By default, an envelope is in an unsigned state. Calling sign() will
 * permanently change the envelopes state to signed. 
 * 
 * When an envelope is in an unsigned state any changes made to its xml
 * serializer contents will be immediately reflected when converting the
 * envelope to xml.
 * 
 * When an envelope is in a signed state any changes made to its xml
 * serializer contents will only be reflected upon calling sign().
 * 
 * @author Dave Longley
 */
public class SignableXMLEnvelope implements IXMLSerializer
{
   /**
    * The xml text that this envelope was last converted from. This
    * is used to parse out the contents of the envelope for
    * digital signing.
    */
   protected String mXMLEnvelope;
   
   /**
    * The xml serializer interface that produces the
    * xml that will be signed.
    */
   protected IXMLSerializer mIXMLSerializer;
   
   /**
    * The text that was signed.
    */
   protected String mSignedText;

   /**
    * The version of this envelope.
    */
   protected String mVersion;

   /**
    * The identity of the signer.
    */
   protected String mSigner;

   /**
    * The status of this signable xml entity. The status
    * could be "valid" or "invalid".
    */
   protected String mStatus;
   
   /**
    * The algorithm used to produce the signature.
    */
   protected String mAlgorithm;
   
   /**
    * The signature produced when signing.
    */
   protected String mSignature;

   /**
    * Constructs a signable xml envelope with no xml serializer contents.
    */
   public SignableXMLEnvelope()
   {
      this((IXMLSerializer)null);
   }

   /**
    * Constructs a signable xml envelope that converts from
    * the passed xml text.
    *
    * @param xmlText the xml text to convert from.
    */
   public SignableXMLEnvelope(String xmlText)
   {
      this();

      convertFromXML(xmlText);
   }   

   /**
    * Constructs a signable xml envelope that envelopes the
    * xml text produced by the passed IXMLSerializer and
    * converts from the passed xml text.
    *
    * @param xmlSerializer the xml serializer interface to envelope.
    * @param xmlText the xml text to convert from.
    */
   public SignableXMLEnvelope(IXMLSerializer xmlSerializer, String xmlText)
   {
      this(xmlSerializer);

      convertFromXML(xmlText);
   }

   /**
    * Constructs a signable xml envelope that envelopes the
    * xml text produced by the passed IXMLSerializer.
    *
    * @param xmlSerializer the xml serializer interface to envelope.
    */
   public SignableXMLEnvelope(IXMLSerializer xmlSerializer)
   {
      mIXMLSerializer = xmlSerializer;
      mXMLEnvelope = "";
      mSignedText = null;
      mVersion = "1.0";

      mSigner = "0";
      mStatus = "valid";
      mAlgorithm = "";
      mSignature = "";
   }

   /**
    * Parses out content of the envelope so it can be used to
    * verify the digital signature of the envelope.
    * 
    * @return the content of the envelope in xml form.
    */
   protected String parseContents()
   {
      String contents = "";
      
      String sTag1 = "<signature";
      String sTag2 = "</signature>";
      String eTag = "</" + getRootTag() + ">";
      
      // look for beginning of signature tag
      int start = mXMLEnvelope.indexOf(sTag1);
      if(start != -1)
      {
         // look for end of signature tag
         int end = mXMLEnvelope.indexOf(sTag2, start + sTag1.length());
         if(end == -1)
         {
            end = mXMLEnvelope.indexOf("/>", start + sTag1.length());
            if(end != -1)
            {
               end += 2;
            }
         }
         else
         {
            end += sTag2.length();
         }
         
         if(end != -1)
         {
            // look for closing envelope tag
            int close = mXMLEnvelope.lastIndexOf(eTag);
            if(close != -1)
            {
               contents = mXMLEnvelope.substring(end, close).trim();
            }
         }
      }
      
      return contents;
   }
   
   /**
    * Updates the text to sign.
    */
   protected synchronized void updateSignedText()
   {
      mSignedText = null;
      if(mIXMLSerializer != null)
      {
         mSignedText = mIXMLSerializer.convertToXML(1).trim();
      }
   }

   /**
    * Contructs a signature with the passed signer and
    * the private key.
    * 
    * When this method is called, the text contents of the envelope
    * are locked until this method is called again. Therefore, any changes
    * made to the xml serializer contents of this envelope will not be
    * reflected until this method is called again. This ensures the
    * integrity of the signature.
    *  
    * When converting this envelope to xml, the signed text will be used
    * as the contents of this envelope, the xml serializers convert method
    * will not be called again until sign() is called again.
    *
    * @param signer the signer of the signature.
    * @param privateKey the privateKey to sign with.
    * @return true if successfully signed, false if not. 
    */
   public boolean sign(long signer, PrivateKey privateKey)
   {
      return sign("" + signer, privateKey);
   }
   
   
   /**
    * Contructs a signature with the passed signer and
    * the private key.
    * 
    * When this method is called, the text contents of the envelope
    * are locked until this method is called again. Therefore, any changes
    * made to the xml serializer contents of this envelope will not be
    * reflected until this method is called again. This ensures the
    * integrity of the signature.
    *  
    * When converting this envelope to xml, the signed text will be used
    * as the contents of this envelope, the xml serializers convert method
    * will not be called again until sign() is called again.
    *
    * @param signer the signer of the signature.
    * @param key the Base64-PKCS8 privateKey to sign with.
    * @return true if successfully signed, false if not. 
    */
   public boolean sign(long signer, String key)   
   {
      return sign("" + signer, key);
   }
   
   /**
    * Contructs a signature with the passed signer and
    * the private key.
    * 
    * When this method is called, the text contents of the envelope
    * are locked until this method is called again. Therefore, any changes
    * made to the xml serializer contents of this envelope will not be
    * reflected until this method is called again. This ensures the
    * integrity of the signature.
    *  
    * When converting this envelope to xml, the signed text will be used
    * as the contents of this envelope, the xml serializers convert method
    * will not be called again until sign() is called again.
    *
    * @param signer the signer of the signature.
    * @param key the Base64-PKCS8 privateKey to sign with.
    * @return true if successfully signed, false if not. 
    */
   public boolean sign(String signer, String key)
   {
      PrivateKey privateKey = KeyManager.decodePrivateKey(key);
      return sign(signer, privateKey);
   }   

   /**
    * Contructs a signature with the passed signer and
    * the private key.
    * 
    * When this method is called, the text contents of the envelope
    * are locked until this method is called again. Therefore, any changes
    * made to the xml serializer contents of this envelope will not be
    * reflected until this method is called again. This ensures the
    * integrity of the signature.
    *  
    * When converting this envelope to xml, the signed text will be used
    * as the contents of this envelope, the xml serializers convert method
    * will not be called again until sign() is called again.
    *
    * @param signer the signer of the signature.
    * @param privateKey the privateKey to sign with.
    * @return true if successfully signed, false if not. 
    */
   public synchronized boolean sign(String signer, PrivateKey privateKey)
   {
      boolean rval = false;

      // make sure there is a key to sign it with
      if(privateKey != null)
      {
         try
         {
            // update the text to sign
            updateSignedText();
            
            // use blank string if signed text is null
            if(mSignedText == null)
            {
               mSignedText = "";
            }
            
            // set the signer
            mSigner = signer;
            
            // set algorithm to SHA1:key's algorithm
            mAlgorithm = "SHA1/" + privateKey.getAlgorithm();
         
            // sign the text
            byte[] sig = Cryptor.sign(mSignedText, privateKey);
            
            // base64 encode the signature for xml transport
            Base64Coder encoder = new Base64Coder();
            mSignature = encoder.encode(sig);
            
            LoggerManager.debug("dbcommon",
                  "BEGIN SIGN TEXT:" + mSignedText + ":END SIGN TEXT\n" +
                  "SIGNATURE: '" + mSignature + "'");

            rval = (mSignature != null);

            if(mSignature == null)
            {
               mSignature = "";
            }
         }
         catch(Exception e)
         {
            LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(e));
         }
      }

      return rval;
   }

   /**
    * Attempts to verify that this object was digitally
    * signed. The passed public key should be a Base64-encoded
    * string that represents the X.509 encoded public key.
    *
    * @param publicKey the public key to verify the signature.
    * @return true if verified, false if not.
    */
   public synchronized boolean verify(String publicKey)
   {
      boolean rval = false;
      
      // make sure the status is valid
      if(isValid())
      {
         // make sure there is a signature
         if(!mSignature.equals(""))
         {
            // make sure there is a public key
            if(publicKey != null)
            {
               try
               {
                  // get the text to verify
                  String contents = parseContents();

                  if(!mAlgorithm.startsWith("SHA1"))
                  {
                     LoggerManager.debug("dbcommon",
                                         "unknown signature algorithm!," +
                                         "algorithm=" + mAlgorithm);
                  }
                  
                  // base64 decode the signature
                  Base64Coder decoder = new Base64Coder();
                  byte[] sig = decoder.decode(mSignature);
                  
                  LoggerManager.debug("dbcommon",
                        "BEGIN VERIFY TEXT:" + contents + ":END VERIFY TEXT\n" +
                        "SIGNATURE: '" + mSignature + "'");
         
                  // verify the signature
                  rval = Cryptor.verify(sig, contents, publicKey);
               }
               catch(Exception e)
               {
                  LoggerManager.debug("dbcommon",
                        LoggerManager.getStackTrace(e));
               }
            }
         }
      }

      if(!rval)
      {
         setStatus("invalid");
      }

      return rval;
   }

   /**
    * Gets this envelope's version.
    * 
    * @return this envelope's version.
    */
   public String getVersion()
   {
      return mVersion;
   }

   /**
    * Gets the last signer of this envelope as a long.
    * 
    * @return the signer's id as a long.
    */
   public long getSignerLong()
   {
      long rval = 0;
      
      try
      {
         rval = Long.parseLong(getSigner());
      }
      catch(Throwable t)
      {
         LoggerManager.debug("dbcommon", Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Gets the last signer of this envelope.
    * 
    * @return the signer's id.
    */
   public String getSigner()
   {
      return mSigner;
   }

   /**
    * Gets the status of this envelope.
    * 
    * @return the status of this envelope.
    */
   public String getStatus()
   {
      return mStatus;
   }

   /**
    * Sets the status of this envelope.
    * 
    * @param status the status to set the envelope to.
    */
   public void setStatus(String status)
   {
      mStatus = status;
   }
   
   /**
    * Gets the algorithm used to create the signature for the envelope.
    * 
    * @return the algorithm used to create the signature for the envelope,
    *         or a blank string if the envelope is not signed.
    */
   public String getAlgorithm()
   {
      return mAlgorithm;
   }

   /**
    * Returns true if this envelope's status is valid, false if not.
    * Whenever verify() fails on an envelope its status is set to
    * invalid.
    * 
    * @return returns true if this envelope's status is valid, false if not.
    */
   public boolean isValid()
   {
      return mStatus.equals("valid");
   }
   
   /**
    * Sets the interface that produces the xml text to
    * envelope and sign.
    *
    * @param xmlSerializer the xml serializer interface.
    */
   public synchronized void setContents(IXMLSerializer xmlSerializer)
   {
      mIXMLSerializer = xmlSerializer;
      mSignedText = null;
   }

   /**
    * Gets the interface that produces the xml text to envelope
    * and sign.
    *
    * @return the xml serializer interface.
    */
   public IXMLSerializer getContents()
   {
      return mIXMLSerializer;
   }
   
   /**
    * This method takes options that are used to configure
    * how to convert to and from xml.
    *
    * @param options the configuration options.
    * @return true if options successfully set, false if not.    
    */
   public boolean setSerializerOptions(int options)
   {
      return false;
   }

   /**
    * This method gets the options that are used to configure
    * how to convert to and from xml.
    *
    * @return the configuration options.
    */
   public int getSerializerOptions()
   {
      return 0;
   }
   
   /**
    * Returns the root tag name for this serializer.
    * 
    * @return the root tag name for this serializer.
    */
   public String getRootTag()   
   {
      return "envelope";
   }   

   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    * 
    * @return the xml-based representation of this object.
    */
   public String convertToXML()
   {
      return convertToXML(0);
   }
   
   /**
    * This method takes the object representation and creates an
    * XML-based representation of the object.
    *
    * @param indentLevel the number of spaces to place before the text
    *                    after each new line.
    * @return the xml-based representation of the object.
    */
   public String convertToXML(int indentLevel)
   {
      StringBuffer xml = new StringBuffer();
      StringBuffer indent = new StringBuffer("\n");
      for(int i = 0; i < indentLevel; i++)
      {
         indent.append(' ');
      }

      if(indentLevel == 0)
      {
         xml.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
         //xml.append("<!DOCTYPE transaction SYSTEM \"bitmunk.dtd\">\n");
      }

      xml.append(indent);
      xml.append('<');
      xml.append(getRootTag());
      xml.append(" version=\"");
      xml.append(XMLCoder.encode(getVersion()));
      xml.append("\" signer=\"");
      xml.append(XMLCoder.encode(getSigner()));
      xml.append("\" status=\"");
      xml.append(XMLCoder.encode(getStatus()));
      xml.append("\">");
      
      xml.append(indent);
      xml.append(" <signature algorithm=\"");
      xml.append(XMLCoder.encode(getAlgorithm()));
      xml.append("\">");
      xml.append(XMLCoder.encode(mSignature));
      xml.append("</signature>");

      // use signed text, if available
      if(mSignedText != null)
      {
         xml.append(mSignedText);
      }
      else if(mIXMLSerializer != null)
      {
         xml.append(mIXMLSerializer.convertToXML(indentLevel + 1));
      }

      xml.append(indent);
      xml.append("</");
      xml.append(getRootTag());
      xml.append('>');

      return xml.toString();
   }
   
   /**
    * This method takes XML text (in full document form) and converts
    * it to it's internal representation.
    *
    * @param xmlText the xml text document that represents the object.
    * @return true if successful, false otherwise.    
    */
   public boolean convertFromXML(String xmlText)
   {
      boolean rval = false;
      
      mXMLEnvelope = xmlText;
      
      try
      {
         DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
         DocumentBuilder builder = factory.newDocumentBuilder();
         
         InputSource is = new InputSource(new StringReader(xmlText));
         Document doc = builder.parse(is);
         
         // normalize text representation
         doc.getDocumentElement().normalize();
         
         rval = convertFromXML(doc.getDocumentElement());
      }
      catch(SAXParseException spe)
      {
         LoggerManager.debug("dbcommon",
                             "SignableXMLEnvelope parsing error" +
                             ", line " + spe.getLineNumber() +
                             ", uri " + spe.getSystemId());
         LoggerManager.debug("dbcommon", "   " + spe.getMessage());
         
         LoggerManager.debug("dbcommon", "SXE string:\n" + xmlText);
         
         LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(spe));
      }
      catch(Throwable t)
      {
         LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects information.
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXML(Element element)
   {
      boolean rval = false;
      
      ElementReader er = new ElementReader(element);
      
      if(er != null)
      {
         // get version, signer, status
         mVersion = XMLCoder.decode(er.getStringAttribute("version")); 
         mSigner = XMLCoder.decode(er.getStringAttribute("signer"));
         mStatus = XMLCoder.decode(er.getStringAttribute("status"));
         
         ElementReader ser = er.getFirstElementReader("signature");
         mAlgorithm = XMLCoder.decode(ser.getStringAttribute("algorithm"));
         mSignature = XMLCoder.decode(ser.getStringValue());
         
         rval = true;
         
         Vector ers = er.getElementReaders();
         Iterator i = ers.iterator();
         while(i.hasNext())
         {
            er = (ElementReader)i.next();

            if(!er.getTagName().equals("signature") && mIXMLSerializer != null)
            {
               // if there is an embedded envelope load parsed contents
               if(er.getTagName().equals(getRootTag()))
               {
                  rval &= mIXMLSerializer.convertFromXML(parseContents());
               }
               else
               {
                  rval &= mIXMLSerializer.convertFromXML(er.getElement());
               }
               
               break;
            }
         }
      }
      
      return rval;
   }
}
