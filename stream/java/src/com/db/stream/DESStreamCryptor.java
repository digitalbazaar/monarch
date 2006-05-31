/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import com.db.crypto.KeyManager;
import com.db.logging.LoggerManager;

import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;

/**
 * A class that encrypts and decrypts streaming data
 * using the "DES" algorithm.
 * 
 * @author Dave Longley
 */
public class DESStreamCryptor extends StreamCryptor
{
   /**
    * Constructs a DES stream cryptor. The encrypt chunk size must be
    * specified. The encrypt chunk size is the size that the internal
    * chunk buffer must reach before it is encrypted. If the encrypt
    * chunk size passed is not a multiple of 8, an illegal argument
    * exception will be thrown and any further use of this object
    * will result in undefined behavior.
    * 
    * @param encryptChunkSize the encrypt chunk size.
    * @throws IllegalArgumentException
    */
   public DESStreamCryptor(int encryptChunkSize)
      throws IllegalArgumentException
   {
      // use 8 for padding
      super(encryptChunkSize, encryptChunkSize + 8, 8);
      
      // encrypt chunk size must be a multiple of padding (8)
      if(encryptChunkSize % 8 != 0)
         throw new IllegalArgumentException();
      
      // generate a DES key for the encryption and decryption
      getCryptor().generateInternalKey("DES");
   }
   
   /**
    * Constructs a DES stream cryptor. The encrypt chunk size must be
    * specified. The encrypt chunk size is the size that the internal
    * chunk buffer must reach before it is encrypted. If the encrypt
    * chunk size passed is not a multiple of padding, an illegal argument
    * exception will be thrown and any further use of this object
    * will result in undefined behavior.
    * 
    * @param encryptChunkSize the encrypt chunk size.
    * @param key the key to use to encrypt and decrypt. 
    * @throws IllegalArgumentException
    */
   public DESStreamCryptor(int encryptChunkSize, SecretKey key)
      throws IllegalArgumentException
   {
      // use 8 for padding
      super(encryptChunkSize, encryptChunkSize + 8, 8);
      
      // encrypt chunk size must be a multiple of padding
      if(encryptChunkSize % 8 != 0)
         throw new IllegalArgumentException();
      
      // generate a DES key for the encryption and decryption
      getCryptor().setKey(key);
   }
   
   /**
    * Constructs a DES stream cryptor. The encrypt chunk size must be
    * specified. The encrypt chunk size is the size that the internal
    * chunk buffer must reach before it is encrypted. If the encrypt
    * chunk size passed is not a multiple of padding, an illegal argument
    * exception will be thrown and any further use of this object
    * will result in undefined behavior.
    * 
    * @param encryptChunkSize the encrypt chunk size.
    * @param key the key to use to encrypt and decrypt. 
    * @throws IllegalArgumentException
    */
   public DESStreamCryptor(int encryptChunkSize, String key)
      throws IllegalArgumentException
   {
      super(encryptChunkSize, encryptChunkSize + 8, 8);
      
      // encrypt chunk size must be a multiple of 8
      if(encryptChunkSize % 8 != 0)
         throw new IllegalArgumentException();
      
      // generate a DES key for the encryption and decryption
      setKey(key);
   }   
   
   /**
    * Decodes a DES secret key from its encoded byte form.
    *
    * @param encodedKey the encoded byte array of key material.
    * @return the decoded secret key object.
    */
   public static SecretKey decodeSecretKey(byte[] encodedKey)
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
    * Decodes a DES secret key from its Base64-encoded string form.
    *
    * @param encodedKey the encoded byte array of key material.
    * @return the decoded private key object.
    */
   public static SecretKey decodeSecretKey(String encodedKey)
   {
      return decodeSecretKey(KeyManager.decodeKey(encodedKey));
   }

   /**
    * Gets the key used to encrypt and decrypt data.
    * 
    * @return the key used to encrypt and decrypt data in Base64
    *         byte encoded string form.
    */
   public String getKey()
   {
      SecretKey key = (SecretKey)getCryptor().getKey();
      return (key == null) ? null : KeyManager.encodeKey(key);
   }
   
   /**
    * Sets the internal key after decoding the passed Base64-encoded key.
    * 
    * @param encodedKey the Base64-encoded key to set as the internal key.
    */
   public void setKey(String encodedKey)
   {
      getCryptor().setKey(decodeSecretKey(encodedKey));
   }   
}
