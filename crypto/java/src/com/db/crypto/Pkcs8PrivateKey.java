/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.crypto;

import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.spec.DSAPrivateKeySpec;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * A class for storing a PKCS#8 encoded private key.
 * 
 * The algorithms currently supported for returning PrivateKey objects
 * from this class are the Digital Signature Algorithm (DSA) and RSA.
 * 
 * A PKCS#8 encoded key is stored in ASN.1 DER format. The PrivateKeyInfo
 * ASN.1 structure is composed of a version of type Integer, a 
 * privateKeyAlgorithm of type AlgorithmIdentifier, a privateKey of
 * type octet string, and attributes of type set.
 * 
 * The AlgorithmIdentifier contains an Object Identifier (OID) and 
 * parameters for the algorithm, if any.
 * 
 * DSA requires parameters P, Q, and G. These are used in the Digital Signature
 * Algorithm to create and verify signatures. These parameters must be provided
 * along with the private key in order to create a signature.
 * 
 * PrivateKeyInfo ::= SEQUENCE
 * {
 *    version INTEGER,
 *    privateKeyAlgorithm AlgorithmIdentifier,
 *    privateKey OCTET STRING
 *    attributes [0] IMPLICIT SET OPTIONAL
 * }
 * 
 * AlgorithmIdentifer ::= SEQUENCE
 * {
 *    algorithm OBJECT IDENTIFIER,
 *    parameters ANY DEFINED BY algorithm OPTIONAL
 * }
 * 
 * For a DSA private key --
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
 * The privateKey octet string contains the DSA private key.
 * 
 * An OCTET STRING has the following structure:
 * 
 * OCTECT STRING
 * {
 *    number of octets
 *    data
 * }
 * 
 * @author Dave Longley
 */
public class Pkcs8PrivateKey
{
   /**
    * The ASN.1 DER structure for this PKCS#8 private key.
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
    * Creates a new PKCS8PrivateKey from a byte array.
    * 
    * @param bytes the DER byte array containing the PKCS#8 private key.
    */
   public Pkcs8PrivateKey(byte[] bytes)
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
    * Creates a new PKCS8PrivateKey from a DSAPrivateKeySpec object.
    * 
    * @param keySpec the DSA private key spec to convert from.
    */
   public Pkcs8PrivateKey(DSAPrivateKeySpec keySpec)
   {
      convertFrom(keySpec);
   }
   
   /**
    * Converts this PKCS#8 private key from a DSAPrivateKeySpec object.
    * 
    * @param keySpec the DSA private key spec to convert from.
    */
   protected void convertFrom(DSAPrivateKeySpec keySpec)
   {
      // get the version
      Asn1Der version = Asn1Der.unsignedBigIntToAsn1Der(new byte[1]);
      
      // get the DSA signature key OBJECT IDENTIFIER in ASN.1 format
      Asn1Der oidASN1 = Asn1Der.oidToAsn1Der(DSA_SIGNATURE_KEY_OID);
      
      // get the DSS-Params
      Asn1Der parameters = new Asn1Der();
      parameters.addAsn1Der(Asn1Der.unsignedBigIntToAsn1Der(keySpec.getP()));
      parameters.addAsn1Der(Asn1Der.unsignedBigIntToAsn1Der(keySpec.getQ()));
      parameters.addAsn1Der(Asn1Der.unsignedBigIntToAsn1Der(keySpec.getG()));
      
      // get the algorithm identifier
      Asn1Der algId = new Asn1Der(Asn1Der.ASN1_TYPE_SEQUENCE);
      algId.addAsn1Der(oidASN1);
      algId.addAsn1Der(parameters);
      
      // get the DSA private key
      Asn1Der privateKey =
         Asn1Der.unsignedBigIntToAsn1Der(
            Asn1Der.ASN1_TYPE_OCTETSTRING, keySpec.getX());
      
      // get the attributes set
      Asn1Der attributes = new Asn1Der(Asn1Der.ASN1_TYPE_SET, new byte[0]);
      
      // get the private key info
      Asn1Der privateKeyInfo = new Asn1Der(Asn1Der.ASN1_TYPE_SEQUENCE);
      privateKeyInfo.addAsn1Der(version);
      privateKeyInfo.addAsn1Der(algId);
      privateKeyInfo.addAsn1Der(privateKey);
      privateKeyInfo.addAsn1Der(attributes);
      
      // set the internal ASN.1 structure
      mAsn1Der = privateKeyInfo;  
   }
   
   /**
    * Converts this PKCS#8 encoded private key into a PrivateKey. Only
    * DSA and RSA are currently supported.
    * 
    * @return the private key or null if there was an error.
    */
   protected PrivateKey convertToPrivateKey()
   {
      PrivateKey rval = null;
      
      if(mAsn1Der != null)
      {
         // get the second ASN.1 (algorithm identifier)
         Asn1Der algId = mAsn1Der.getAsn1Der(1);
         
         // get the OID ASN.1 from the algorithm
         Asn1Der oidAsn1Der = algId.getAsn1Der(0);
         
         // get the OID string
         String oidStr = Asn1Der.asn1DerToOid(oidAsn1Der);
         
         try
         {
            // get the PKCS8 encoded key spec
            PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(getBytes());

            // determine if a DSA or an RSA key should be loaded
            if(oidStr.equals(DSA_SIGNATURE_KEY_OID))
            {
               KeyFactory keyFactory = KeyFactory.getInstance("DSA");
               rval = keyFactory.generatePrivate(keySpec);
            }
            else if(oidStr.equals(RSA_SIGNATURE_KEY_OID))
            {
               KeyFactory keyFactory = KeyFactory.getInstance("RSA");
               rval = keyFactory.generatePrivate(keySpec);
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
    * Gets the PrivateKey interface for this private key.
    * 
    * @return the PrivateKey interface for this private key.
    */
   public PrivateKey getPrivateKey()
   {
      return convertToPrivateKey();
   }
   
   /**
    * Gets this PKCS#8 encoded private key in binary form.
    * 
    * @return the DER encoded private key bytes or null.
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
    * Gets this PKCS#8 encoded private key in ASN.1 format. Returns
    * the ASN.1 structure "PrivateKeyInfo".
    * 
    * @return the ASN.1 DER encoded private key structure.
    */
   public Asn1Der getAsn1Der()
   {
      return mAsn1Der;
   }
}
