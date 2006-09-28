/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import com.db.crypto.KeyManager;

import javax.crypto.SecretKey;

/**
 * A class that encrypts and decrypts streaming data
 * using the TripleDES "DESede" algorithm.
 * 
 * @author Dave Longley
 */
public class TripleDesStreamCryptor extends StreamCryptor
{
   /**
    * Constructs a TripleDES stream cryptor. The encrypt chunk size must be
    * specified. The encrypt chunk size is the size that the internal chunk
    * buffer must reach before it is encrypted. If the encrypt chunk size
    * passed is not a multiple of 8, an illegal argument exception will be
    * thrown and any further use of this object will result in undefined
    * behavior.
    * 
    * @param encryptChunkSize the encrypt chunk size.
    * 
    * @throws IllegalArgumentException
    */
   public TripleDesStreamCryptor(int encryptChunkSize)
      throws IllegalArgumentException
   {
      this(encryptChunkSize, (SecretKey)null);
   }
   
   /**
    * Constructs a TripleDES stream cryptor. The encrypt chunk size must be
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
   public TripleDesStreamCryptor(int encryptChunkSize, String key)
      throws IllegalArgumentException
   {
      this(encryptChunkSize, decodeSecretKey(key));
   }
   
   /**
    * Constructs a TripleDES stream cryptor. The encrypt chunk size must be
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
   public TripleDesStreamCryptor(int encryptChunkSize, SecretKey key)
      throws IllegalArgumentException
   {
      // use 8 for padding
      super(encryptChunkSize, encryptChunkSize + 8, 8);
      
      // encrypt chunk size must be a multiple of padding
      if(encryptChunkSize % 8 != 0)
      {
         throw new IllegalArgumentException();
      }
      
      if(key == null)
      {
         // generate a DESede key for the encryption and decryption
         getCryptor().generateInternalKey("DESede");
      }
      else
      {
         // set the key
         getCryptor().setKey(key);
      }
   }
   
   /**
    * Decodes a TripleDES secret key from its encoded byte form.
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
         secretKey = KeyManager.decodeSymmetricalKey(encodedKey, "DESede");
      }

      return secretKey;
   }
   
   /**
    * Decodes a DES secret key from its Base64-encoded string form.
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
