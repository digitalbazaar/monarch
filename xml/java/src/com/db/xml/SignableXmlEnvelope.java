/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.xml;

import com.db.crypto.Cryptor;
import com.db.crypto.KeyManager;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.Base64Coder;

import java.security.PrivateKey;
import java.security.PublicKey;

import org.w3c.dom.Element;

/**
 * A SignableXMLEnvelope is a transferrable container that allows
 * any object that implements the IXmlSerializer interface to be
 * transported in a secure, signed vessel.
 * 
 * This envelope may be used in either "signed" or "unsigned" form to
 * transport information.
 * 
 * Whenever this envelope is signed, its content becomes locked. If
 * the content of this envelope is changed after it has been signed, then
 * it will fail verification. The envelope must be signed again if the
 * contents are changed in order to pass verification. 
 *
 * In other words, once sign() is called on this envelope, if you want
 * to modify its contents and pass verification, you must call sign()
 * again after the modification.
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
 * Xml Format:
 * 
 * The current version of SignableXmlEnvelope (2.0) uses the following
 * xml format:
 * 
 * <pre>
 * 
 * <envelope version="2.0" signer="100" status="signed">
 * 
 *  <signature algorithm="SHAwithDSA">a-base64-encoded-signature</signature>
 *  <content>my xml-encoded if necessary content</content>
 * 
 * </envelope>
 * 
 * </pre>
 * 
 * The text in the content tag (the value of the content element) is the
 * xml-encoded sign text. To verify the text, this envelope retrieves the
 * content element's value, xml-decodes it, and checks it against the
 * signature using the signature's algorithm and the public key associated
 * with the signer of the envelope (in this case, signer "100"). The signer
 * is actually stored as a string, so it doesn't necessarily have to be
 * a number.
 * 
 * The content, for this implementation, is another xml structure that is
 * exported to xml using any kind of whitespace formatting that the structure
 * desires. Once the content value is read in, the whitespace at the beginning
 * and end of the value is trimmed and this envelope's xml serializer content
 * object parses it.
 * 
 * @author Dave Longley
 */
public class SignableXmlEnvelope extends VersionedXmlSerializer
{
   /**
    * The xml serializer interface that produces the xml that will
    * be signed. This is the content for this envelope.
    */
   protected IXmlSerializer mContent;
   
   /**
    * The text to sign/that was signed.
    */
   protected String mSignText;

   /**
    * The identity of the signer.
    */
   protected String mSigner;

   /**
    * The status of this signable xml entity. The status can be
    * "unsigned", "signed", or "invalid".
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
    * Constructs a SignableXmlEnvelope with no yet set xml serializer content.
    */
   public SignableXmlEnvelope()
   {
      this(null);
   }

   /**
    * Constructs a SignableXmlEnvelope that envelopes the passed xml
    * serializer. 
    *
    * @param xmlSerializer the xml serializer to envelope.
    * @param xml the xml to convert from.
    */
   public SignableXmlEnvelope(IXmlSerializer xmlSerializer, String xml)
   {
      this(xmlSerializer);

      // convert from xml
      convertFromXml(xml);
   }

   /**
    * Constructs a signable xml envelope that envelopes the
    * xml text produced by the passed IXMLSerializer.
    *
    * @param xmlSerializer the xml serializer interface to envelope.
    */
   public SignableXmlEnvelope(IXmlSerializer xmlSerializer)
   {
      super("2.0");
      
      mContent = xmlSerializer;
      mSignText = "";

      mSigner = "0";
      mStatus = "unsigned";
      mAlgorithm = "";
      mSignature = "";
   }
   
   /**
    * Updates the text to sign.
    */
   protected synchronized void updateSignText()
   {
      mSignText = "";
      if(getContent() != null)
      {
         mSignText = getContent().convertToXml(1);
      }
   }

   /**
    * Sets the status of this envelope.
    * 
    * @param status the status to set the envelope to.
    */
   protected void setStatus(String status)
   {
      mStatus = status;
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
    * 
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
    * 
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
    * 
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
    * 
    * @return true if successfully signed, false if not. 
    */
   public synchronized boolean sign(String signer, PrivateKey privateKey)
   {
      boolean rval = false;
      
      // set status to signed, regardless of whether or not signature is
      // successful -- a sign() was attempted
      setStatus("signed");
      
      // set the signer
      mSigner = signer;

      // update the text to sign
      updateSignText();

      // make sure there is a key to sign it with
      if(privateKey != null)
      {
         try
         {
            // get the signature algorithm -- use SHA1
            if(privateKey.getAlgorithm().equals("DSA"))
            {
               mAlgorithm = "SHAwithDSA";
            }
            else if(privateKey.getAlgorithm().equals("RSA"))
            {
               mAlgorithm = "SHA1withRSA";
            }
            else
            {
               mAlgorithm = "";
            }
            
            // sign the text
            byte[] sig = Cryptor.sign(mSignText, privateKey);
            
            // base64 encode the signature for xml transport
            Base64Coder encoder = new Base64Coder();
            mSignature = encoder.encode(sig);
            
            getLogger().detail(getClass(),
               "BEGIN SIGN TEXT:" + mSignText + ":END SIGN TEXT\n" +
               "SIGNATURE: '" + mSignature + "'");

            if(mSignature != null)
            {
               rval = true;
            }
            else
            {
               mSignature = "";
            }
         }
         catch(Exception e)
         {
            getLogger().debug(getClass(), Logger.getStackTrace(e));
         }
      }

      return rval;
   }
   
   /**
    * Attempts to verify that this object was digitally
    * signed. The passed public key should be a Base64-encoded
    * string that represents the X.509 encoded public key.
    *
    * @param publicKeyString the public key to verify the signature.
    * 
    * @return true if verified, false if not.
    */
   public synchronized boolean verify(String publicKeyString)

   {
      // obtain the decoded public key and verify
      PublicKey publicKey = KeyManager.decodePublicKey(publicKeyString);
      return verify(publicKey);
   }
   
   /**
    * Attempts to verify that this object was digitally
    * signed. The passed public key should be a Base64-encoded
    * string that represents the X.509 encoded public key.
    *
    * @param publicKey the public key to verify the signature.
    * 
    * @return true if verified, false if not.
    */
   public synchronized boolean verify(PublicKey publicKey)   
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
                  if(!mAlgorithm.startsWith("SHA"))
                  {
                     getLogger().debug(getClass(),
                        "unknown signature algorithm!," +
                        "algorithm=" + mAlgorithm);
                  }
                  
                  // base64 decode the signature
                  Base64Coder decoder = new Base64Coder();
                  byte[] sig = decoder.decode(mSignature);
                  
                  getLogger().detail(getClass(),
                     "BEGIN VERIFY TEXT:" + mSignText + ":END VERIFY TEXT\n" +
                     "SIGNATURE: '" + mSignature + "'");
         
                  // verify the signature
                  rval = Cryptor.verify(sig, mSignText, publicKey);
               }
               catch(Exception e)
               {
                  getLogger().debug(getClass(), Logger.getStackTrace(e));
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
         getLogger().debug(getClass(), Logger.getStackTrace(t));
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
    * Returns true if this envelope's status is valid, false if not.
    * Whenever verify() fails on an envelope its status is set to
    * "invalid". Otherwise this envelope is valid whether its status is
    * "signed" or "unsigned."
    * 
    * @return returns true if this envelope's status is valid, false if not.
    */
   public boolean isValid()
   {
      return !getStatus().equals("invalid");
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
    * Sets the content of this envelope, that is, the object that implements
    * an xml serializer interface that produces the xml text to envelope.
    *
    * @param xmlSerializer the xml serializer interface.
    */
   public synchronized void setContent(IXmlSerializer xmlSerializer)
   {
      mContent = xmlSerializer;
      mSignText = "";
   }

   /**
    * Gets the content of this envelope, that is, the object that implements
    * an xml serializer interface that produces the text to envelope.
    *
    * @return the xml serializer interface.
    */
   public IXmlSerializer getContent()
   {
      return mContent;
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
    * @param indentLevel the number of spaces to place before the text
    *                    after each new line.
    *                    
    * @return the xml-based representation of the object.
    */
   public String convertToXml(int indentLevel)
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

      // start tag
      xml.append(indent);
      xml.append('<');
      xml.append(getRootTag());
      xml.append(" version=\"");
      xml.append(XmlCoder.encode(getVersion()));
      xml.append("\" signer=\"");
      xml.append(XmlCoder.encode(getSigner()));
      xml.append("\" status=\"");
      xml.append(XmlCoder.encode(getStatus()));
      xml.append("\">");
      
      // signature tag
      xml.append(indent);
      xml.append(" <signature algorithm=\"");
      xml.append(XmlCoder.encode(getAlgorithm()));
      xml.append("\">");
      xml.append(XmlCoder.encode(mSignature));
      xml.append("</signature>");
      
      // start content tag
      xml.append(indent);
      xml.append(" <content>");

      if(getStatus().equals("signed"))
      {
         // since the envelope is signed, use the sign text
         xml.append(XmlCoder.encode(mSignText));
      }
      else if(getContent() != null)
      {
         // the envelope is not signed, so just convert
         xml.append(XmlCoder.encode(getContent().convertToXml(1)));
      }
      
      // end content tag
      xml.append("</content>");

      // end tag
      xml.append(indent);
      xml.append("</");
      xml.append(getRootTag());
      xml.append('>');

      return xml.toString();
   }
   
   /**
    * This method takes a parsed DOM XML element and converts it
    * back into this object's representation.
    *
    * @param element the parsed element that contains this objects information.
    * 
    * @return true if successful, false otherwise.
    */
   public boolean convertFromXml(Element element)
   {
      boolean rval = false;
      
      ElementReader er = new ElementReader(element);
      
      if(er != null)
      {
         // get version, signer, status
         setVersion(XmlCoder.decode(er.getStringAttribute("version"))); 
         mSigner = XmlCoder.decode(er.getStringAttribute("signer"));
         mStatus = XmlCoder.decode(er.getStringAttribute("status"));
         
         // get signature information
         ElementReader sigReader = er.getFirstElementReader("signature");
         mAlgorithm = XmlCoder.decode(
            sigReader.getStringAttribute("algorithm"));
         mSignature = XmlCoder.decode(
            sigReader.getStringValue());
         
         rval = true;
         
         // if this envelope has content, get the content reader
         if(getContent() != null)
         {
            // get an element reader for the content
            ElementReader contentReader = er.getFirstElementReader("content");
            if(contentReader != null)
            {
               // store the content xml as the sign text
               mSignText = contentReader.getStringValue();
               
               // convert the xml content
               rval = getContent().convertFromXml(mSignText.trim());
            }
            else
            {
               // blank out sign text
               mSignText = "";
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the logger for this xml serializer.
    * 
    * @return the logger for this xml serializer.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbxml");
   }
}
