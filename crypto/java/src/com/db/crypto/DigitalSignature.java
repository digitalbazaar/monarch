/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.crypto;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.Signature;

/**
 * A class for generating and verifying digital signatures.
 * 
 * TEMPCODE: All of this code will be replaced by C++ libraries, it has just
 * been refactored slightly to accommodate changes to related code for the
 * time being.
 * 
 * @author Dave Longley
 */
public class DigitalSignature
{
   /**
    * Signs the passed byte array with the passed private key and
    * returns the signature.
    *
    * @param data the data to sign.
    * @param privateKey the privateKey to sign with.
    * 
    * @return the signature if successfully signed, null if not. 
    */
   public static byte[] sign(byte[] data, PrivateKey privateKey)
   {
      byte[] sig = null;
      
      // make sure there is a key to sign with
      if(privateKey != null)
      {
         try
         {
            // get the key algorithm
            String algorithm = privateKey.getAlgorithm();
            
            // get the signature algorithm
            if(algorithm.equals("DSA"))
            {
               algorithm = "SHAwithDSA";
            }
            else if(algorithm.equals("RSA"))
            {
               algorithm = "SHA1withRSA";
            }
            
            // sign the signature string with the private key
            Signature signature = Signature.getInstance(algorithm);
            signature.initSign(privateKey);
            signature.update(data);
            
            // sign the data
            sig = signature.sign();
         }
         catch(Exception e)
         {
            getLogger().debug(DigitalSignature.class, Logger.getStackTrace(e));
         }
      }

      return sig;
   }

   /**
    * Signs the passed string with the passed private key and
    * returns the signature. The signature will be Base64-encoded.
    *
    * @param text the string to sign.
    * @param privateKey the privateKey to sign with.
    * 
    * @return the signature if successfully signed, null if not. 
    */
   public static byte[] sign(String text, PrivateKey privateKey)
   {
      byte[] data = null;

      try
      {
         if(text != null)
         {
            data = text.getBytes("UTF-8");
         }
         else
         {
            data = new byte[0];
         }
      }
      catch(Exception e)
      {
         getLogger().debug(DigitalSignature.class, Logger.getStackTrace(e));
      }

      return sign(data, privateKey);
   }
   
   /**
    * Signs the passed byte array with the passed private key and
    * returns the signature.
    *
    * @param data the data to sign.
    * @param key the Base64-PKCS8 privateKey to sign with.
    * 
    * @return the signature if successfully signed, null if not. 
    */
   public static byte[] sign(byte[] data, String key)
   {
      PrivateKey privateKey = KeyManager.decodePrivateKey(key);
      return sign(data, privateKey);
   }

   /**
    * Signs the passed string with the passed private key and
    * returns the signature. The signature will be Base64-encoded.
    *
    * @param text the string to sign.
    * @param key the Base64-PKCS8 privateKey to sign with.
    * 
    * @return the signature if successfully signed, null if not. 
    */
   public static byte[] sign(String text, String key)
   {
      PrivateKey privateKey = KeyManager.decodePrivateKey(key);
      return sign(text, privateKey);
   }

   /**
    * Attempts to verify the signature for the passed byte array
    * using the passed public key.
    *
    * @param sig the signature to verify.
    * @param data the data the signature is for.
    * @param key the public key to verify the signature.
    * 
    * @return true if verified, false if not.
    */
   public static boolean verify(byte[] sig, byte[] data, PublicKey key)
   {
      boolean rval = false;

      if(key != null)
      {
         try
         {
            // get the key algorithm
            String algorithm = key.getAlgorithm();
            
            // get the signature algorithm
            if(algorithm.equals("DSA"))
            {
               algorithm = "SHAwithDSA";
            }
            else if(algorithm.equals("RSA"))
            {
               algorithm = "SHA1withRSA";
            }
            
            // verify the digital signature
            Signature signature = Signature.getInstance(algorithm);
            signature.initVerify(key);
            signature.update(data);
         
            rval = signature.verify(sig);
         }
         catch(Exception e)
         {
            getLogger().debug(DigitalSignature.class, Logger.getStackTrace(e));
         }
      }
      
      return rval;
   }

   /**
    * Attempts to verify the signature for the passed byte array
    * using the passed Base64-X509-encoded public key.
    *
    * @param sig the signature to verify.
    * @param data the data the signature is for.
    * @param key the public key to verify the signature.
    * 
    * @return true if verified, false if not.
    */
   public static boolean verify(byte[] sig, byte[] data, String key)
   {
      // obtain the decoded public key
      PublicKey publicKey = KeyManager.decodePublicKey(key);
      return verify(sig, data, publicKey);
   }

   /**
    * Attempts to verify the signature for the passed string
    * using the passed public key.
    *
    * @param sig the signature to verify.
    * @param text the text the signature is for.
    * @param key the public key to verify the signature.
    * 
    * @return true if verified, false if not.
    */
   public static boolean verify(byte[] sig, String text, PublicKey key)
   {
      byte[] data = null;

      try
      {
         data = text.getBytes("UTF-8");
      }
      catch(Exception e)
      {
         getLogger().debug(DigitalSignature.class, Logger.getStackTrace(e));
      }
      
      return verify(sig, data, key);
   }

   /**
    * Attempts to verify the signature for the passed string
    * using the passed Base64-X509-encoded public key.
    *
    * @param sig the signature to verify.
    * @param text the text the signature is for.
    * @param key the public key to verify the signature.
    * 
    * @return true if verified, false if not.
    */
   public static boolean verify(byte[] sig, String text, String key)
   {
      // obtain the decoded public key
      PublicKey publicKey = KeyManager.decodePublicKey(key);
      return verify(sig, text, publicKey);
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public static Logger getLogger()
   {
      return LoggerManager.getLogger("dbcrypto");
   }
}
