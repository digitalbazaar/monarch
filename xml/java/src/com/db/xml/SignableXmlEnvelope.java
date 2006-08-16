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
import java.util.Iterator;
import java.util.Vector;

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
 * @author Dave Longley
 */
public class SignableXmlEnvelope extends VersionedXmlSerializer
{
   /**
    * The xml text that this envelope was last converted from. This
    * is used to parse out the contents of the envelope for digital
    * signature verification.
    */
   protected String mXmlEnvelope;
   
   /**
    * The xml serializer interface that produces the xml that will
    * be signed. This is the content for this envelope.
    */
   protected IXmlSerializer mContent;
   
   /**
    * The text that was signed.
    */
   protected String mSignedText;

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
    * Constructs a signable xml envelope with no xml serializer contents.
    */
   public SignableXmlEnvelope()
   {
      this(null);
   }

   /**
    * Constructs a signable xml envelope that envelopes the
    * xml text produced by the passed IXMLSerializer and
    * converts from the passed xml text.
    *
    * @param xmlSerializer the xml serializer interface to envelope.
    * @param xmlText the xml text to convert from.
    */
   public SignableXmlEnvelope(IXmlSerializer xmlSerializer, String xmlText)
   {
      this(xmlSerializer);

      convertFromXml(xmlText);
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
      mXmlEnvelope = "";
      mSignedText = null;

      mSigner = "0";
      mStatus = "unsigned";
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
      int start = mXmlEnvelope.indexOf(sTag1);
      if(start != -1)
      {
         // look for end of signature tag
         int end = mXmlEnvelope.indexOf(sTag2, start + sTag1.length());
         if(end == -1)
         {
            end = mXmlEnvelope.indexOf("/>", start + sTag1.length());
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
            int close = mXmlEnvelope.lastIndexOf(eTag);
            if(close != -1)
            {
               contents = mXmlEnvelope.substring(end, close).trim();
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
      if(getContent() != null)
      {
         mSignedText = getContent().convertToXml(1).trim();
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
            byte[] sig = Cryptor.sign(mSignedText, privateKey);
            
            // base64 encode the signature for xml transport
            Base64Coder encoder = new Base64Coder();
            mSignature = encoder.encode(sig);
            
            getLogger().detail(getClass(),
               "BEGIN SIGN TEXT:" + mSignedText + ":END SIGN TEXT\n" +
               "SIGNATURE: '" + mSignature + "'");

            if(mSignature != null)
            {
               rval = true;
               
               // set status to signed
               setStatus("signed");
            }

            if(mSignature == null)
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
    * @param publicKey the public key to verify the signature.
    * 
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
                     "BEGIN VERIFY TEXT:" + contents + ":END VERIFY TEXT\n" +
                     "SIGNATURE: '" + mSignature + "'");
         
                  // verify the signature
                  rval = Cryptor.verify(sig, contents, publicKey);
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
    * Sets the content of this envelope, that is, the object that implements
    * an xml serializer interface that produces the xml text to envelope.
    *
    * @param xmlSerializer the xml serializer interface.
    */
   public synchronized void setContent(IXmlSerializer xmlSerializer)
   {
      mContent = xmlSerializer;
      mSignedText = null;
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
      
      xml.append(indent);
      xml.append(" <signature algorithm=\"");
      xml.append(XmlCoder.encode(getAlgorithm()));
      xml.append("\">");
      xml.append(XmlCoder.encode(mSignature));
      xml.append("</signature>");

      // use signed text, if available
      if(mSignedText != null)
      {
         xml.append(mSignedText);
      }
      else if(getContent() != null)
      {
         xml.append(getContent().convertToXml(indentLevel + 1));
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
    * 
    * @return true if successful, false otherwise.    
    */
   public boolean convertFromXml(String xmlText)
   {
      boolean rval = false;
      
      mXmlEnvelope = xmlText;
      
      rval = super.convertFromXml(xmlText);
      
      return rval;
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
         
         ElementReader ser = er.getFirstElementReader("signature");
         mAlgorithm = XmlCoder.decode(ser.getStringAttribute("algorithm"));
         mSignature = XmlCoder.decode(ser.getStringValue());
         
         rval = true;
         
         Vector ers = er.getElementReaders();
         Iterator i = ers.iterator();
         while(i.hasNext())
         {
            er = (ElementReader)i.next();

            if(!er.getTagName().equals("signature") && getContent() != null)
            {
               // if there is an embedded envelope load parsed contents
               if(er.getTagName().equals(getRootTag()))
               {
                  rval &= getContent().convertFromXml(parseContents());
               }
               else
               {
                  rval &= getContent().convertFromXml(er.getElement());
               }
               
               break;
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
