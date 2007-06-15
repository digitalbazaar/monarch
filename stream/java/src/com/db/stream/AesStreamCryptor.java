/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import com.db.crypto.KeyManager;

import javax.crypto.SecretKey;

/**
 * A class that encrypts and decrypts streaming data using the "AES" algorithm
 * (Advanced Encryption Algorithm).
 * 
 * @author Dave Longley
 */
public class AesStreamCryptor extends StreamCryptor
{
   /**
    * Constructs a AES stream cryptor with an auto-generated key.
    */
   public AesStreamCryptor()
   {
      this((SecretKey)null);
   }
   
   /**
    * Constructs a AES stream cryptor with the specified key.
    * 
    * @param key the key to use to encrypt and decrypt.
    */
   public AesStreamCryptor(String key)
   {
      this(decodeSecretKey(key));
   }
   
   /**
    * Constructs a AES stream cryptor with the specified key.
    * 
    * @param key the key to use to encrypt and decrypt. 
    */
   public AesStreamCryptor(SecretKey key)
   {
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
         secretKey = KeyManager.decodeSymmetricalKey(encodedKey, "AES");
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
