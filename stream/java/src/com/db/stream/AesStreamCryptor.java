/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import com.db.crypto.KeyManager;
import com.db.logging.LoggerManager;

import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;

/**
 * A class that encrypts and decrypts streaming data using the "AES" algorithm
 * (Advanced Encryption Algorithm).
 * 
 * @author Dave Longley
 */
public class AesStreamCryptor extends StreamCryptor
{
   /**
    * Constructs a AES stream cryptor. The encrypt chunk size must be
    * specified. The encrypt chunk size is the size that the internal
    * chunk buffer must reach before it is encrypted. If the encrypt
    * chunk size passed is not a multiple of 16, an illegal argument
    * exception will be thrown and any further use of this object
    * will result in undefined behavior.
    * 
    * @param encryptChunkSize the encrypt chunk size.
    * 
    * @throws IllegalArgumentException
    */
   public AesStreamCryptor(int encryptChunkSize)
      throws IllegalArgumentException
   {
      this(encryptChunkSize, (SecretKey)null);
   }
   
   /**
    * Constructs a AES stream cryptor. The encrypt chunk size must be
    * specified. The encrypt chunk size is the size that the internal
    * chunk buffer must reach before it is encrypted. If the encrypt
    * chunk size passed is not a multiple of padding, an illegal argument
    * exception will be thrown and any further use of this object
    * will result in undefined behavior.
    * 
    * @param encryptChunkSize the encrypt chunk size.
    * @param key the key to use to encrypt and decrypt.
    * 
    * @throws IllegalArgumentException
    */
   public AesStreamCryptor(int encryptChunkSize, String key)
      throws IllegalArgumentException
   {
      this(encryptChunkSize, decodeSecretKey(key));
   }   
   
   /**
    * Constructs a AES stream cryptor. The encrypt chunk size must be
    * specified. The encrypt chunk size is the size that the internal
    * chunk buffer must reach before it is encrypted. If the encrypt
    * chunk size passed is not a multiple of padding, an illegal argument
    * exception will be thrown and any further use of this object
    * will result in undefined behavior.
    * 
    * @param encryptChunkSize the encrypt chunk size.
    * @param key the key to use to encrypt and decrypt. 
    * 
    * @throws IllegalArgumentException
    */
   public AesStreamCryptor(int encryptChunkSize, SecretKey key)
      throws IllegalArgumentException
   {
      // use 16 for padding
      super(encryptChunkSize, encryptChunkSize + 16, 16);
      
      // encrypt chunk size must be a multiple of padding
      if(encryptChunkSize % 16 != 0)
      {
         throw new IllegalArgumentException();
      }
      
      if(key == null)
      {
         // generate a AES key for the encryption and decryption
         getCryptor().generateInternalKey("AES");
      }
      else
      {
         // set the key
         getCryptor().setKey(key);
      }
   }
   
   /**
    * Decodes a AES secret key from its encoded byte form.
    *
    * @param encodedKey the encoded byte array of key material.
    * 
    * @return the decoded secret key object.
    */
   public static SecretKey decodeSecretKey(byte[] encodedKey)
   {
      SecretKey secretKey = null;

      if(encodedKey != null)
      {
         try
         {
            secretKey = new SecretKeySpec(encodedKey, "AES");
         }
         catch(Exception e)
         {
            LoggerManager.getLogger("dbstream").debug(AesStreamCryptor.class, 
               LoggerManager.getStackTrace(e));
         }
      }

      return secretKey;
   }
   
   /**
    * Decodes a AES secret key from its Base64-encoded string form.
    *
    * @param encodedKey the encoded byte array of key material.
    * 
    * @return the decoded private key object.
    */
   public static SecretKey decodeSecretKey(String encodedKey)
   {
      return decodeSecretKey(KeyManager.base64DecodeKey(encodedKey));
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
      return (key == null) ? null : KeyManager.base64EncodeKey(key);
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
