/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.crypto;

import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PublicKey;
import java.security.spec.DSAPublicKeySpec;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.X509EncodedKeySpec;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A class for storing an X.509 encoded public key.
 * 
 * The algorithms currently supported for returning PublicKey objects
 * from this class are the Digital Signature Algorithm (DSA) and RSA.
 * 
 * An X.509 encoded key is stored in ASN.1 format. The SubjectPublicKeyInfo
 * ASN.1 structure is composed of an algorithm of type AlgorithmIdentifier
 * and a subjectPublicKey of type bit string.
 * 
 * The AlgorithmIdentifier contains an Object Identifier (OID) and 
 * parameters for the algorithm, if any.
 * 
 * DSA requires parameters P, Q, and G. These are used in the Digital Signature
 * Algorithm to create and verify signatures. These parameters must be provided
 * along with the public key in order to verify a signature.
 * 
 * SubjectPublicKeyInfo ::= SEQUENCE
 * {
 *    algorithm AlgorithmIdentifier,
 *    subjectPublicKey BIT STRING
 * }
 * 
 * AlgorithmIdentifer ::= SEQUENCE
 * {
 *    algorithm OBJECT IDENTIFIER,
 *    parameters ANY DEFINED BY algorithm OPTIONAL
 * }
 * 
 * For a DSA public key --
 * 
 * The parameters for the AlgorithmIdentifier are such:
 * 
 * Dss-Parms ::= SEQUENCE
 * {
 *    P INTEGER,
 *    Q INTEGER,
 *    G INTEGER
 * }
 * 
 * The subjectPublicKey bit string contains the DSA public key as
 * an integer:
 * 
 * DSAPublicKey ::= INTEGER
 * 
 * @author Dave Longley
 */
public class X509PublicKey
{
   /**
    * The ASN.1 DER encoded structure for this X.509 public key.
    */
   protected Asn1Der mAsn1Der;
   
   /**
    * The Object Identifier (OID) for a DSA signature key.
    */
   public static final String DSA_SIGNATURE_KEY_OID = "1.2.840.10040.4.1";
   
   /**
    * The Object Identifier (OID) for an RSA signature key.
    */
   public static final String RSA_SIGNATURE_KEY_OID = "1.2.840.113549.1.1.1";

   /**
    * Creates a new X509PublicKey from a byte array.
    * 
    * @param bytes the byte array containing the ASN.1 DER encoded public key.
    */
   public X509PublicKey(byte[] bytes)
   {
      // ensure bytes aren't null, have a length > 0, and the ASN.1 tag
      // is a ASN.1 Type of Sequence
      if(bytes != null && bytes.length > 0 &&
         (bytes[0] & Asn1Der.ASN1_TYPE_SEQUENCE) ==
         Asn1Der.ASN1_TYPE_SEQUENCE)
      {
         mAsn1Der = new Asn1Der(bytes);
      }
   }
   
   /**
    * Creates a new X509PublicKey from a DSAPublicKeySpec object.
    * 
    * @param keySpec the DSAPublicKeySpec spec to convert from.
    */
   public X509PublicKey(DSAPublicKeySpec keySpec)
   {
      convertFrom(keySpec);
   }

   /**
    * Converts this X.509 public key from a DSAPublicKeySpec object.
    * 
    * @param keySpec the DSA public key spec to convert from.
    */
   protected void convertFrom(DSAPublicKeySpec keySpec)   
   {
      // get the DSA signature key OBJECT IDENTIFIER in ASN.1 format
      Asn1Der oidASN1 = Asn1Der.oidToAsn1Der(DSA_SIGNATURE_KEY_OID);
      
      // get the DSS-Params
      Asn1Der parameters = new Asn1Der(Asn1Der.ASN1_TYPE_SEQUENCE);
      parameters.addAsn1Der(Asn1Der.unsignedBigIntToAsn1Der(keySpec.getP()));
      parameters.addAsn1Der(Asn1Der.unsignedBigIntToAsn1Der(keySpec.getQ()));
      parameters.addAsn1Der(Asn1Der.unsignedBigIntToAsn1Der(keySpec.getG()));
      
      // get the algorithm identifier
      Asn1Der algId = new Asn1Der(Asn1Der.ASN1_TYPE_SEQUENCE);
      algId.addAsn1Der(oidASN1);
      algId.addAsn1Der(parameters);
      
      // get the DSA public key
      Asn1Der dsaPublicKey = Asn1Der.unsignedBigIntToAsn1Der(keySpec.getY());
      
      // create a BIT STRING
      // the first bit in the BIT STRING is the number of unused
      // bits in the first byte -- which is assumed to be zero because
      // the keys are stored as bytes
      byte[] val = dsaPublicKey.getBytes();
      byte[] bitString = new byte[val.length + 1];
      bitString[0] = 0;
      System.arraycopy(val, 0, bitString, 1, val.length);
      
      // get the subject public key
      Asn1Der subjectPublicKey =
         new Asn1Der(Asn1Der.ASN1_TYPE_BITSTRING, bitString);
      
      // get the subject public key info
      Asn1Der subjectPublicKeyInfo = new Asn1Der(Asn1Der.ASN1_TYPE_SEQUENCE);
      subjectPublicKeyInfo.addAsn1Der(algId);
      subjectPublicKeyInfo.addAsn1Der(subjectPublicKey);
      
      // set the internal ASN.1 structure
      mAsn1Der = subjectPublicKeyInfo;         
   }
   
   /**
    * Converts this X.509 public key into a PublicKey. Only DSA and RSA are
    * currently supported.
    * 
    * @return the public key or null if there was an error.
    */
   protected PublicKey convertToPublicKey()
   {
      PublicKey rval = null;
   
      if(mAsn1Der != null)
      {
         // get the first ASN.1 (algorithm identifier)
         Asn1Der algId = mAsn1Der.getAsn1Der(0);
         
         // get the OID ASN.1 from the algorithm
         Asn1Der oidAsn1Der = algId.getAsn1Der(0);
         
         // get the OID string
         String oidStr = Asn1Der.asn1DerToOid(oidAsn1Der);
         
         try
         {
            // get the X.509 encoded key spec
            X509EncodedKeySpec keySpec = new X509EncodedKeySpec(getBytes());

            // determine if a DSA or an RSA key should be loaded
            if(oidStr.equals(DSA_SIGNATURE_KEY_OID))
            {
               KeyFactory keyFactory = KeyFactory.getInstance("DSA");
               rval = keyFactory.generatePublic(keySpec);
            }
            else if(oidStr.equals(RSA_SIGNATURE_KEY_OID))
            {
               KeyFactory keyFactory = KeyFactory.getInstance("RSA");
               rval = keyFactory.generatePublic(keySpec);
            }            
         }
         catch(NoSuchAlgorithmException nsae)
         {
            LoggerManager.getLogger("dbcrypto").error(getClass(), 
               "algorithm is not supported.");
            LoggerManager.getLogger("dbcrypto").debug(getClass(), 
               Logger.getStackTrace(nsae));
         }
         catch(InvalidKeySpecException ikse)
         {
            LoggerManager.getLogger("dbcrypto").error(getClass(),
               "KeySpec is invalid.");
            LoggerManager.getLogger("dbcrypto").debug(getClass(), 
               Logger.getStackTrace(ikse));
         }         
      }
      
      return rval;
   }
   
   /**
    * Gets the PublicKey interface for this public key.
    * 
    * @return the PublicKey interface for this public key.
    */
   public PublicKey getPublicKey()
   {
      return convertToPublicKey();
   }
   
   /**
    * Gets this X.509 encoded public key in binary form.
    * 
    * @return the DER encoded public key bytes or null.
    */
   public byte[] getBytes()
   {
      byte[] bytes = null;
      
      if(mAsn1Der != null)
      {
         bytes = mAsn1Der.getBytes();
      }
      
      return bytes;
   }
   
   /**
    * Gets this X.509 encoded public key in ASN.1 format. Returns
    * the ASN.1 structure "SubjectPublicKeyInfo".
    * 
    * @return the ASN.1 DER encoded public key structure.
    */
   public Asn1Der getAsn1Der()
   {
      return mAsn1Der;
   }
}
