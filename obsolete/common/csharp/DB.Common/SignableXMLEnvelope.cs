/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.IO;
using System.Text;
using System.Xml;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// A SignableXMLEnvelope is a transferrable container that allows
   /// any object that implements the IXMLSerializer interface to be
   /// transported in a secure, signed vessel.
   /// </summary>
   /// 
   /// <author>Dave Longley</author>
   public class SignableXMLEnvelope : IXMLSerializer
   {
      /// <summary>
      /// The xml text that this envelope was last converted from. This
      /// is used to parse out the contents of the envelope for
      /// digital signing.
      /// </summary>
      protected string mXMLEnvelope;
      
      /// <summary>
      /// The xml serializer interface that produces the
      /// xml that will be signed.
      /// </summary>
      protected IXMLSerializer mIXMLSerializer;
      
      /// <summary>
      /// The version of this envelope.
      /// </summary>
      protected string mVersion;
      
      /// <summary>
      /// The identity of the signer.
      /// </summary>
      protected string mSigner;
      
      /// <summary>
      /// The status of this signable xml entity. The status
      /// could be "valid" or "invalid".
      /// </summary>
      protected string mStatus;
      
      /// <summary>
      /// The algorithm used to produce the signature.
      /// </summary>
      protected string mAlgorithm;
      
      /// <summary>
      /// The signature produced when signing.
      /// </summary>
      protected string mSignature;
      
      /// <summary>
      /// Constructs an empty signable xml envelope.
      /// </summary>
      public SignableXMLEnvelope() :
         this((IXMLSerializer)null)
      {
      }
      
      /// <summary>
      /// Constructs a signable xml envelope that envelopes the
      /// xml text produced by the passed IXMLSerializer.
      /// </summary>
      ///
      /// <param name="xmlSerializer">the xml serializer interface to
      /// envelope.</param>
      public SignableXMLEnvelope(IXMLSerializer xmlSerializer)
      {
         mXMLEnvelope = "";
         mIXMLSerializer = xmlSerializer;
         mVersion = "1.0";
         
         mSigner = "0";
         mStatus = "valid";
         mAlgorithm = "";
         mSignature = "";
      }
      
      /// <summary>
      /// Constructs a signable xml envelope that converts from
      /// the passed xml text.
      /// </summary>
      ///
      /// <param name="xmlText">the xml text to convert from.</param>
      public SignableXMLEnvelope(string xmlText) :
         this()
      {
         ConvertFromXML(xmlText);
      }
      
      /// <summary>
      /// Constructs a signable xml envelope that envelopes the
      /// xml text produced by the passed IXMLSerializer and
      /// converts from the passed xml text.
      /// </summary>
      ///
      /// <param name="xmlSerializer">the xml serializer interface to
      /// envelope.</param>
      /// <param name="xmlText">the xml text to convert from.</param>
      public SignableXMLEnvelope(IXMLSerializer xmlSerializer, string xmlText) :
         this(xmlSerializer)
      {
         ConvertFromXML(xmlText);
      }
      
      /// <summary>
      /// Trims the beginning and trailing whitespace from xmlText.
      /// </summary>
      ///
      /// <param name="xmlText">the xmlText to trim the whitespace out
      /// of.</param>
      /// <returns>the trimmed contents.</returns>
      protected virtual string TrimWhitespace(string xmlText)
      {
         StringBuilder trimmed = new StringBuilder();
         
         try
         {
            StringReader sr = new StringReader(xmlText);
            
            string line = "";
            while((line = sr.ReadLine()) != null)
            {
               trimmed.Append(line.Trim());
            }
         }
         catch(Exception e)
         {
            LoggerManager.Debug("dbcommon", e.ToString());
         }
         
         return trimmed.ToString();
      }
      
      /// <summary>
      /// Parses out content of the envelope so it can be used to
      /// verify the digital signature of the envelope.
      /// </summary>
      ///
      /// <returns>the content of the envelope in xml form.</returns>
      protected virtual string ParseContents()
      {
         string contents = "";
      
         string sTag1 = "<signature";
         string sTag2 = "</signature>";
         string eTag = "</" + RootTag + ">";
      
         // look for beginning of signature tag
         int start = mXMLEnvelope.IndexOf(sTag1);
         if(start != -1)
         {
            // look for end of signature tag
            int end = mXMLEnvelope.IndexOf(sTag2, start + sTag1.Length);
            if(end == -1)
            {
               end = mXMLEnvelope.IndexOf("/>", start + sTag1.Length);
               if(end != -1)
               {
                  end += 2;
               }
            }
            else
            {
               end += sTag2.Length;
            }
            
            if(end != -1)
            {
               // look for closing envelope tag
               int close = mXMLEnvelope.IndexOf(eTag, end);
               if(close != -1)
               {
                  contents = mXMLEnvelope.Substring(end, close - 1 - end);
               }
            }
         }
      
         return contents;
      }
      
      /// <summary>
      /// Contructs a signature with the passed signer and
      /// the private key.
      /// </summary>
      ///
      /// <param name="signer">the signer of the signature.</param>
      /// <param name="privateKey">the privateKey to sign with.</param>
      /// <returns>true if successfully signed, false if not.</returns>
      public virtual bool Sign(long signer, PKCS8PrivateKey privateKey)
      {
         return Sign("" + signer, privateKey);
      }
      
      /// <summary>
      /// Contructs a signature with the passed signer and
      /// the private key.
      /// </summary>
      ///
      /// <param name="signer">the signer of the signature.</param>
      /// <param name="privateKey">the privateKey to sign with.</param>
      /// <returns>true if successfully signed, false if not.</returns>
      public virtual bool Sign(string signer, PKCS8PrivateKey privateKey)
      {
         bool rval = false;

         // make sure there is a key to sign it with
         if(privateKey != null)
         {
            try
            {
               // get the text to sign
               string text = "";
               if(Contents != null)
               {
                  text = TrimWhitespace(Contents.ConvertToXML(false));
               }
               
               // set the signer
               mSigner = signer;
               
               LoggerManager.DebugData("dbcommon",
                                       "BEGIN SIGN TEXT:" + text +
                                       ":END SIGN TEXT");
               
               // set algorithm to SHA1:key's algorithm
               mAlgorithm = "SHA1/" + privateKey.Algorithm;
               
               // sign the text
               byte[] sig = Cryptor.Sign(text, privateKey);
               
               // base64 encode the signature for xml transport
               mSignature = Convert.ToBase64String(sig);
               
               LoggerManager.DebugData("dbcommon", "SIGNATURE: " + mSignature);
               
               rval = (mSignature != null);
               
               if(mSignature == null)
               {
                  mSignature = "";
               }
            }
            catch(Exception e)
            {
               LoggerManager.Debug("dbcommon", e.ToString());
            }
         }
         
         return rval;
      }
      
      /// <summary>
      /// Contructs a signature with the passed signer and
      /// the private key.
      /// </summary>
      ///
      /// <param name="signer">the signer of the signature.</param>
      /// <param name="key">the Base64-PKCS8 privateKey to sign with.</param>
      /// <returns>true if successfully signed, false if not.</returns>
      public virtual bool Sign(long signer, string key)
      {
         return Sign("" + signer, key);
      }
      
      /// <summary>
      /// Contructs a signature with the passed signer and
      /// the private key.
      /// </summary>
      ///
      /// <param name="signer">the signer of the signature.</param>
      /// <param name="key">the Base64-PKCS8 privateKey to sign with.</param>
      /// <returns>true if successfully signed, false if not.</returns>
      public virtual bool Sign(string signer, string key)
      {
         PKCS8PrivateKey privateKey =
            new PKCS8PrivateKey(Convert.FromBase64String(key));
         return Sign(signer, privateKey);
      }
      
      /// <summary>
      /// Attempts to verify that this object was digitally
      /// signed. The passed public key should be a Base64-encoded
      /// string that represents the X.509 encoded public key.
      /// </summary>
      ///
      /// <param name="publicKey">the public key to verify the
      /// signature.</param>
      /// <returns>true if verified, false if not.</returns>
      public virtual bool Verify(X509PublicKey publicKey)
      {
         bool rval = false;
         
         // make sure the status is valid
         if(IsValid)
         {
            // make sure there is a signature
            if(mSignature != "")
            {
               // make sure there is a public key
               if(publicKey != null)
               {
                  try
                  {
                     // get the text to verify
                     string contents = ParseContents();
                     contents = TrimWhitespace(contents);
                     
                     LoggerManager.DebugData("dbcommon",
                                             "BEGIN VERIFY TEXT:" + contents +
                                             ":END VERIFY TEXT");
                                         
                     // base64 decode the signature
                     byte[] sig = Convert.FromBase64String(mSignature);
                     
                     LoggerManager.DebugData("dbcommon",
                                             "SIGNATURE: " + mSignature);
                     
                     // verify the signature
                     rval = Cryptor.Verify(sig, contents, publicKey);
                  }
                  catch(Exception e)
                  {
                     LoggerManager.Error("dbcommon", "could not verify " +
                                         "envelope signature,an exception " +
                                         "occurred!");
                     LoggerManager.Debug("dbcommon", e.ToString());
                  }
               }
            }
         }
         
         if(!rval)
         {
            Status = "invalid";
         }
         
         return rval;      
      }

      /// <summary>
      /// Attempts to verify that this object was digitally
      /// signed. The passed public key should be a Base64-encoded
      /// string that represents the X.509 encoded public key.
      /// </summary>
      ///
      /// <param name="publicKey">the public key to verify the
      /// signature.</param>
      /// <returns>true if verified, false if not.</returns>
      public virtual bool Verify(string publicKey)
      {
         X509PublicKey key =
            new X509PublicKey(Convert.FromBase64String(publicKey));
         return Verify(key);
      }
      
      /// <summary>
      /// Gets this envelope's version.
      /// </summary>
      ///
      /// <returns>this envelope's version.</returns>
      public virtual string Version
      {
         get
         {
            return mVersion;
         }
      }
      
      /// <summary>
      /// Gets the last signer of this envelope.
      /// </summary>
      ///
      /// <returns>the signer's id.</returns>
      public virtual string Signer
      {
         get
         {
            return mSigner;
         }
      }      
      
      /// <summary>
      /// Gets the last signer of this envelope as a long.
      /// </summary>
      ///
      /// <returns>the signer's id as a long.</returns>
      public virtual long SignerAsLong
      {
         get
         {
            long rval = 0;
            
            try
            {
               rval = Convert.ToInt64(Signer);
            }
            catch(Exception e)
            {
               LoggerManager.Debug("dbcommon", e.ToString());
            }
            
            return rval;
         }
      }
      
      /// <summary>
      /// Gets/Sets the status of this envelope.
      /// </summary>
      public virtual string Status
      {
         get
         {
            return mStatus;
         }
         
         set
         {
            mStatus = value;
         }
      }
      
      /// <summary>
      /// Gets the algorithm used to create the signature for the envelope.
      /// </summary>
      ///
      /// <returns>the algorithm used to create the signature for the envelope,
      /// or a blank string if the envelope is not signed.</returns>
      public virtual string Algorithm
      {
         get
         {
            return mAlgorithm;
         }
      }
      
      /// <summary>
      /// Returns true if this envelope's status is valid, false if not.
      /// Whenever verify() fails on an envelope its status is set to
      /// invalid.
      /// </summary>
      ///
      /// <returns>returns true if this envelope's status is valid,
      /// false if not.</returns>
      public virtual bool IsValid
      {
         get
         {
            return Status == "valid";
         }
      }
      
      /// <summary>
      /// Gets/Sets the interface that produces the xml text to
      /// envelope and sign.
      /// </summary>
      public virtual IXMLSerializer Contents
      {
         get
         {
            return mIXMLSerializer;
         }
         
         set
         {
            mIXMLSerializer = value;
         }
      }
      
      /// <summary>
      /// Gets/Sets the serializer options used to configure how to convert
      /// to and convert from xml.
      /// </summary>
      public virtual int SerializerOptions
      {
         get
         {
            return 0;
         }
         
         set
         {
            // does nothing
         }
      }
      
      /// <summary>
      /// Returns the root tag name for this serializer.
      /// </summary>
      ///
      /// <returns>the root tag name for this serializer.</returns>
      public virtual string RootTag
      {
         get
         {
            return "envelope";
         }
      }
      
      /// <summary>
      /// This method takes the object representation and creates an
      /// XML-based representation of the object.
      /// </summary>
      ///
      /// <param name="doc">true to create a new document, false not to.</param>
      /// <returns>the XML-based representation of the object.</returns>
      public virtual string ConvertToXML(bool doc)
      {
         // create a string writer, using a string buffer if the document
         // is set because otherwise utf-16 is forcibly used
         // do NOT use writer.WriteStartDocument/writer.WriteEndDocument
         StringWriter sw = null;
         
         if(doc)
         {
            StringBuilder sb = new StringBuilder(
               "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            sw = new StringWriter(sb);
         }
         else
         {
            sw = new StringWriter();
         }
      
         // create a new xml text writer, indentation 1 for each level
         XmlTextWriter writer = new XmlTextWriter(sw);
         writer.Formatting = Formatting.Indented;
         writer.Indentation = 1;
         
         // convert to XML
         ConvertToXML(writer);
         
         // return the XML string created
         return sw.ToString();
      }
      
      /// <summary>
      /// This method takes the object representation and writes out the
      /// XML-based representation of the object.
      /// </summary>
      ///
      /// <param name="writer">the xml writer used to write to xml.</param>
      public virtual void ConvertToXML(XmlWriter writer)
      {
         // start root tag
         writer.WriteStartElement(RootTag);
         writer.WriteAttributeString("version", Version);
         writer.WriteAttributeString("signer", Signer);
         writer.WriteAttributeString("status", Status);
         
         // write signature element
         writer.WriteStartElement("signature");
         writer.WriteAttributeString("algorithm", Algorithm);
         writer.WriteString(mSignature);
         writer.WriteEndElement();
         
         // write contents
         if(Contents != null)
         {
            Contents.ConvertToXML(writer);
         }
         
         // end root tag
         writer.WriteEndElement();
      }
      
      /// <summary>
      /// This method takes XML text (in full document form) and converts
      /// it to it's internal representation.
      /// </summary>
      ///
      /// <param name="xmlText">the xml text document that represents the
      /// object.</param>
      /// <returns>true if successful, false otherwise.</returns>
      public virtual bool ConvertFromXML(string xmlText)
      {
         bool rval = false;
         
         mXMLEnvelope = xmlText;
         
         try
         {
            StringReader sr = new StringReader(xmlText);
            XmlTextReader reader = new XmlTextReader(sr);
            
            // move to the root tag, convert
            ElementReader er = new ElementReader(reader);
            if(er.Read(RootTag, RootTag))
            {
               rval = ConvertFromXML(reader);
            }
         }
         catch(Exception e)
         {
            LoggerManager.Error("dbcommon",
                                "SignableXMLEnvelope parsing error");
            LoggerManager.Debug("dbcommon", e.ToString());
         }
         
         return rval;
      }
      
      /// <summary>
      /// This method takes an XML reader and converts data read from it into
      /// thi object's representation.
      /// </summary>
      ///
      /// <param name="reader">the xml reader that is reading this object's
      /// information.</param>
      /// <returns>true if successful, false otherwise.</returns>
      public virtual bool ConvertFromXML(XmlReader reader)
      {
         bool rval = false;
         
         try
         {
            // ensure the reader is at the root tag
            if(reader.Name == RootTag)
            {
               // get version, signer, and status
               mVersion = reader.GetAttribute("version");
               mSigner = reader.GetAttribute("signer");
               mStatus = reader.GetAttribute("status");
               
               // move to signature element
               ElementReader er = new ElementReader(reader);
               if(er.Read("signature", RootTag))
               {
                  // get algorithm and signature
                  mAlgorithm = reader.GetAttribute("algorithm");
                  mSignature = reader.ReadString();
                  
                  rval = true;
               }
               
               if(Contents != null)
               {
                  // move to the XML serializer element
                  er.ReadNext(RootTag);
                  
                  // check for an embedded envelope
                  if(reader.NodeType != XmlNodeType.EndElement &&
                     reader.Name == RootTag)
                  {
                     rval &= Contents.ConvertFromXML(ParseContents());
                  }
                  else
                  {
                     rval &= Contents.ConvertFromXML(reader);
                  }
               }
            }
         }
         catch(Exception e)
         {
            LoggerManager.Error("dbcommon",
                                "SignableXMLEnvelope parsing error");
            LoggerManager.Debug("dbcommon", e.ToString());
         }
         
         return rval;
      }
   }
}
