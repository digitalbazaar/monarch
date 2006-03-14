/*
 * Copyright (c) 2003 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import com.db.common.logging.LoggerManager;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.security.Key;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;

import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;

/**
 * A class that provides management code for private, public,
 * and secret keys.
 * 
 * @author Dave Longley
 */
public class KeyManager
{
   /**
    * The last generated private key.
    */
   private PrivateKey mPrivateKey;
   
   /**
    * The last generated public key.
    */
   private PublicKey mPublicKey;
   
   /**
    * The last error that occurred.
    */
   private String mError;
   
   /**
    * Creates a KeyManager. For every new set of keys, a new
    * KeyManager should be instantiated.
    */
   public KeyManager()
   {
      mPrivateKey = null;
      mPublicKey = null;
      
      mError = "";
   }

   /**
    * Encodes the passed key that is in raw encoded byte form and
    * and encodes it with Base64 encoding. A string is the result.
    *
    * @param bytes the key in raw encoded byte form.
    * @return the Base64-encoded string.
    */
   public static String encodeKey(byte[] bytes)
   {
      Base64Coder encoder = new Base64Coder();
      return encoder.encode(bytes);
   }

   /**
    * Encodes the passed key by retrieving its raw encoded bytes
    * and encoding them with Base64 encoding. A string is the result.
    *
    * @param key the key to encode.
    * @return the Base64-encoded string.
    */
   public static String encodeKey(Key key)
   {
      return encodeKey(key.getEncoded());
   }
   
   /**
    * Decodes the passed key from a Base64-encoded string into its
    * encoded byte form.
    * 
    * @param encodedKey the encoded key in Base64-encoded string form.
    * @return the encodedKey in byte form.
    */
   public static byte[] decodeKey(String encodedKey)
   {
      byte[] key = null;
      
      if(encodedKey != null)
      {
         try
         {
            Base64Coder decoder = new Base64Coder(); 
            key = decoder.decode(encodedKey);
         }
         catch(Exception e)
         {
            LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(e));
         }
      }
      
      return key;
   }
   
   /**
    * Generates a pair of keys, one public, one private and stores
    * them internally. The keys are generated using DSA/SHA
    * algorithms with 1024 bit encryption. They can be obtained by
    * calling the appropriate get() method.
    *
    * @return true if successful, false if not.
    */
   public boolean generateKeyPair()
   {
      boolean rval = false;
      
      try
      {
         // uses digital signature algorithm (DSA)
         KeyPairGenerator kpg = KeyPairGenerator.getInstance("DSA");
         
         // set up a secure random number generator for key generation
         // using SHA1 pseudo-random-number-generator provided by Sun
         SecureRandom srng = SecureRandom.getInstance("SHA1PRNG", "SUN");
         
         // initialize the key generator with 1024 bit encryption
         kpg.initialize(1024, srng);
         
         // generate the private and public keys
         KeyPair keyPair = kpg.generateKeyPair();
         mPrivateKey = keyPair.getPrivate();
         mPublicKey = keyPair.getPublic();

         rval = true;
      }
      catch(NoSuchAlgorithmException nsae)
      {
         LoggerManager.error("dbcommon", "DSA algorithm not supported.");
         LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(nsae));
      }
      catch(NoSuchProviderException nspe)
      {
         LoggerManager.error("dbcommon", "SUN is not a supported provider.");
         LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(nspe));
      }

      return rval;
   }
   
   /**
    * Decodes any private key that is PKCS8-encoded.
    *
    * @param encodedKey the PKCS8-encoded byte array.
    * @return the decoded private key object.
    */
   public static PrivateKey decodePrivateKey(byte[] encodedKey)
   {
      PrivateKey privateKey = null;

      if(encodedKey != null)
      {
         try
         {
            PKCS8EncodedKeySpec keySpec = new PKCS8EncodedKeySpec(encodedKey);
            KeyFactory keyFactory = KeyFactory.getInstance("DSA");
            privateKey = keyFactory.generatePrivate(keySpec);
         }
         catch(NoSuchAlgorithmException nsae)
         {
            LoggerManager.error("dbcommon", "DSA algorithm is not supported.");
            LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(nsae));
         }
         catch(InvalidKeySpecException ikse)
         {
            LoggerManager.error("dbcommon", "KeySpec is invalid.");
            LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(ikse));
         }
      }
      
      return privateKey;
   }
   
   /**
    * Decodes any public key that is represented by a string
    * that is Base64-PKCS8-encoded. The passed string is
    * first Base64-decoded into a byte array, and then PKCS8-decoded
    * into a PrivateKey object.
    *
    * @param encodedKey the Base64-PKCS8-encoded string.
    * @return the decoded public key object.
    */
   public static PrivateKey decodePrivateKey(String encodedKey)
   {
      PrivateKey privateKey = null;
      
      if(encodedKey != null)
      {
         privateKey = decodePrivateKey(decodeKey(encodedKey));
      }

      return privateKey;
   }   

   /**
    * Decodes any public key that is X509-encoded.
    *
    * @param encodedKey the X509-encoded byte array.
    * @return the decoded public key object.
    */
   public static PublicKey decodePublicKey(byte[] encodedKey)
   {
      PublicKey publicKey = null;

      if(encodedKey != null)
      {
         try
         {
            X509EncodedKeySpec keySpec = new X509EncodedKeySpec(encodedKey);
            KeyFactory keyFactory = KeyFactory.getInstance("DSA");
            publicKey = keyFactory.generatePublic(keySpec);
         }
         catch(NoSuchAlgorithmException nsae)
         {
            LoggerManager.error("dbcommon", "DSA algorithm is not supported.");
            LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(nsae));
         }
         catch(InvalidKeySpecException ikse)
         {
            LoggerManager.error("dbcommon", "KeySpec is invalid.");
            LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(ikse));
         }
      }

      return publicKey;
   }

   /**
    * Decodes any public key that is represented by a string
    * that is Base64-X509-encoded. The passed string is
    * first Base64-decoded into a byte array, and then X509-decoded
    * into a PublicKey object.
    *
    * @param encodedKey the Base64-X509-encoded string.
    * @return the decoded public key object.
    */
   public static PublicKey decodePublicKey(String encodedKey)
   {
      PublicKey publicKey = null;
      
      if(encodedKey != null)
      {
         publicKey = decodePublicKey(decodeKey(encodedKey));
      }

      return publicKey;
   }

   /**
    * Decodes a DES secret key from its encoded byte form.
    *
    * @param encodedKey the encoded byte array of key material.
    * @return the decoded secret key object.
    */
   public static SecretKey decodeDESKey(byte[] encodedKey)
   {
      SecretKey secretKey = null;

      if(encodedKey != null)
      {
         try
         {
            DESKeySpec keySpec = new DESKeySpec(encodedKey);
            SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("DES");
            secretKey = keyFactory.generateSecret(keySpec);
         }
         catch(Exception e)
         {
            LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(e));
         }
      }

      return secretKey;
   }   

   /**
    * Stores the passed private key in a file with the passed filename
    * and password.
    *
    * @param key the private key to store.
    * @param filename the name of the file to store the key in.
    * @param password the password to lock the file with.
    * @return true if successful, false if not.
    */
   public static boolean storePrivateKey(PrivateKey key,
                                         String filename, String password)
   {
      boolean rval = false;
      
      if(key != null)
      {
         try
         {
            // encrypt the key with the passed password
            byte[] bytes = Cryptor.encrypt(key.getEncoded(), password);
         
            // create private key file, write encrypted-encoded bytes
            File file = new File(filename);
            FileOutputStream fos = new FileOutputStream(file);
            fos.write(bytes);
            fos.close();
            
            rval = true;
         }
         catch(Exception e)
         {
            LoggerManager.debug("dbcommon",
                  "Unable to write private key to file.");
            LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(e));
         }
      }

      return rval;
   }

   /**
    * Stores a private key in a file with the passed filename
    * and password.
    *
    * @param filename the name of the file to store the key in.
    * @param password the password to lock the file with.
    * @return true if successful, false if not.
    */
   public boolean storePrivateKey(String filename, String password)
   {
      return storePrivateKey(mPrivateKey, filename, password);
   }

   /**
    * Stores the passed public key in a file with the passed filename.
    *
    * @param key the public key to store.
    * @param filename the name of the file to store the key in.
    * @return true if successful, false if not.
    */
   public static boolean storePublicKey(PublicKey key, String filename)
   {
      boolean rval = false;
      
      if(key != null)
      {
         try
         {
            // create public key file, write encoded bytes
            File file = new File(filename);
            FileOutputStream fos = new FileOutputStream(file);
            fos.write(key.getEncoded());
            fos.close();
            
            rval = true;
         }
         catch(Exception e)
         {
            LoggerManager.error("dbcommon",
                  "Unable to write public key to file.");
            LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(e));
         }
      }
      
      return rval;
   }   

   /**
    * Stores a public key in a file with the passed filename.
    *
    * @param filename the name of the file to store the key in.
    * @return true if successful, false if not.
    */
   public boolean storePublicKey(String filename)
   {
      return storePublicKey(mPublicKey, filename);
   }

   /**
    * Loads a private key from the file with the passed filename,
    * that is locked with the passed password.
    *
    * @param filename the file that contains the private key.
    * @param password the password to unlock the file.
    * @return true if successful, false if not.
    */
   public boolean loadPrivateKey(String filename, String password)
   {
      boolean rval = false;
      
      try
      {
         // open encrypted-encoded private key file, get bytes
         File file = new File(filename);
         FileInputStream fis = new FileInputStream(file);
         byte[] bytes = new byte[(int)file.length()];
         fis.read(bytes);
         fis.close();

         // decrypt the key with the passed password
         bytes = Cryptor.decrypt(bytes, password);

         // if bytes are not null, decode the key
         if(bytes != null)
         {
            if((mPrivateKey = decodePrivateKey(bytes)) != null)
            {
               rval = true;
            }
            else
            {
               mError = "password-invalid";
            }
         }
      }
      catch(Exception e)
      {
         LoggerManager.error("dbcommon", "Unable to load private key.");
         mError = "key-file-not-found";
         LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(e));
      }

      return rval;
   }

   /**
    * Loads a public key from the file with the passed filename.
    *
    * @param filename the file that contains the public key.
    * @return true if successful, false if not.
    */
   public boolean loadPublicKey(String filename)
   {
      boolean rval = false;
      
      try
      {
         // open encoded public key file, get bytes
         File file = new File(filename);
         FileInputStream fis = new FileInputStream(file);
         byte[] bytes = new byte[(int)file.length()];
         fis.read(bytes);
         fis.close();

         rval = ((mPublicKey = decodePublicKey(bytes)) != null);
      }
      catch(Exception e)
      {
         LoggerManager.error("dbcommon", "Unable to load public key.");
         mError = "key-file-not-found";
         LoggerManager.debug("dbcommon", LoggerManager.getStackTrace(e));
      }

      return rval;
   }   

   /**
    * Returns the internal private key.
    * 
    * @return the internal private key.
    */
   public PrivateKey getPrivateKey()
   {
      return mPrivateKey;
   }
   
   /**
    * Returns the internal public key.
    * 
    * @return the internal public key.
    */
   public PublicKey getPublicKey()
   {
      return mPublicKey;
   }
   
   /**
    * Returns a string that represents a Base64-PKCS8-encoded
    * private key. This method PKCS8-encodes the previously generated
    * private key in a byte array and then Base64-encodes the byte
    * array to produce a string.
    *
    * @return the Base64-PKCS8-encoded string or null.
    */
   public String getPrivateKeyString()
   {
      return encodeKey(mPrivateKey);
   }

   /**
    * Returns a string that represents a Base64-X509-encoded
    * public key. This method X509-encodes the previously generated
    * public key in a byte array and then Base64-encodes the byte
    * array to produce a string.
    *
    * @return the Base64-X509-encoded string or null.
    */
   public String getPublicKeyString()
   {
      return encodeKey(mPublicKey);
   }
   
   /**
    * Clears the error flag.
    */
   public void clearError()
   {
      mError = "";
   }
   
   /**
    * Returns the last error that occurred.
    * 
    * @return the last error that occurred.
    */
   public String getError()
   {
      return mError;
   }
}
