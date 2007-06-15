/*
 * Copyright (c) 2003-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.crypto;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.Base64Coder;
import com.db.util.ByteBuffer;

import java.security.Key;
import java.security.SecureRandom;
import java.security.spec.AlgorithmParameterSpec;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.Random;

import javax.crypto.Cipher;
import javax.crypto.EncryptedPrivateKeyInfo;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.PBEParameterSpec;

/**
 * A class for encrypting and decrypting data.
 * 
 * TEMPCODE: This class and is various short-comings (and weirdness) will be
 * fixed (replaced) with the new C++ library.
 * 
 * @author Dave Longley
 */
public class Cryptor
{
   /**
    * The Cipher used to encrypt/decrypt data.
    */
   protected Cipher mCipher;
   
   /**
    * The key used to decrypt the encrypted data.
    */
   protected Key mKey;
   
   /**
    * A stock salt to be used in an encryption process.
    * 
    * FIXME: This makes salting anything nearly useless.
    */
   protected static byte[] smSalt = {(byte)0xBD, (byte)0x11,
                                     (byte)0x79, (byte)0x31,
                                     (byte)0x07, (byte)0x2A,
                                     (byte)0x41, (byte)0x0B};
   
   /**
    * The iteration count used in the encryption process.
    */
   protected static int smIterationCount = 82;
   
   /**
    * Creates a Cryptor. This object is used to encrypt and
    * decrypt data. If it is used to encrypt data that is
    * stored in memory, then when the data is not being used
    * it should be encrypted, and when it needs to be
    * used it should be decrypted until it is not used again.
    */
   public Cryptor()
   {
      // no cipher, key
      mCipher = null;
      mKey = null;
   }
   
   /**
    * Initializes the internal cipher and key as necessary.
    * 
    * @param encryption true to initialize for encryption, false to initialize
    *                   for decryption.
    * 
    * @return true if initialized, false if error.
    */
   public boolean initializeCipher(boolean encryption)
   {
      boolean rval = false;
      
      try
      {
         if(mCipher == null)
         {
            if(mKey == null)
            {
               // generate internal key as necessary
               generateInternalKey("AES");
            }
            
            // initialize cipher
            mCipher = Cipher.getInstance(mKey.getAlgorithm(), "SunJCE");
            
            if(encryption)
            {
               mCipher.init(Cipher.ENCRYPT_MODE, mKey);
            }
            else
            {
               mCipher.init(Cipher.DECRYPT_MODE, mKey);
            }
            
            rval = true;
         }
         else
         {
            // cipher already initialized
            rval = true;
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "Could not initialize cipher!,exception= " + t);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Updates an encryption using the internal key.
    * 
    * @param b the array of bytes to encrypt.
    * @param offset the offset to start at.
    * @param length the number of bytes to encrypt.
    * @param out the ByteBuffer to write the encrypted data to.
    * 
    * @return true if the encryption was successful, false if there was an
    *         error.
    */
   public boolean updateEncryption(
      byte[] b, int offset, int length, ByteBuffer out)
   {
      boolean rval = false;
      
      try
      {
         if(initializeCipher(true))
         {
            // allocate space for output
            out.allocateSpace(mCipher.getOutputSize(length), true);
            
            // update encryption
            int count = mCipher.update(
               b, offset, length, out.getBytes(),
               out.getOffset() + out.getUsedSpace());
            
            // update bytes
            out.setBytes(
               out.getBytes(), out.getOffset(), out.getUsedSpace() + count);
            
            rval = true;
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "Could not update encryption!,exception= " + t);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Finishes an encryption using the internal key.
    * 
    * @param b the array of bytes to encrypt.
    * @param offset the offset to start at.
    * @param length the number of bytes to encrypt.
    * @param out the ByteBuffer to write the encrypted data to.
    * 
    * @return true if the encryption was successful, false if there was an
    *         error.
    */
   public boolean finishEncryption(
      byte[] b, int offset, int length, ByteBuffer out)
   {
      boolean rval = false;
      
      try
      {
         if(initializeCipher(true))
         {
            // allocate space for output
            out.allocateSpace(mCipher.getOutputSize(length), true);
            
            // finish encryption
            int count = mCipher.doFinal(
               b, offset, length, out.getBytes(),
               out.getOffset() + out.getUsedSpace());
            
            // update bytes
            out.setBytes(
               out.getBytes(), out.getOffset(), out.getUsedSpace() + count);
            
            // reset cipher
            resetCipher();
            
            rval = true;
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "Could not finish encryption!,exception= " + t);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Updates a decryption using the internal key.
    * 
    * @param b the array of bytes to decrypt.
    * @param offset the offset to start at.
    * @param length the number of bytes to decrypt.
    * @param out the ByteBuffer to write the decrypted data to.
    * 
    * @return true if the decryption was successful, false if there was an
    *         error.
    */
   public boolean updateDecryption(
      byte[] b, int offset, int length, ByteBuffer out)
   {
      boolean rval = false;
      
      try
      {
         if(initializeCipher(false))
         {
            // allocate space for output
            out.allocateSpace(mCipher.getOutputSize(length), true);
            
            // update decryption
            int count = mCipher.update(
               b, offset, length, out.getBytes(),
               out.getOffset() + out.getUsedSpace());
            
            // update bytes
            out.setBytes(
               out.getBytes(), out.getOffset(), out.getUsedSpace() + count);
            
            rval = true;
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "Could not update decryption!,exception= " + t);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Finishes a decryption using the internal key.
    * 
    * @param b the array of bytes to decrypt.
    * @param offset the offset to start at.
    * @param length the number of bytes to decrypt.
    * @param out the ByteBuffer to write the decrypted data to.
    * 
    * @return true if the decryption was successful, false if there was an
    *         error.
    */
   public boolean finishDecryption(
      byte[] b, int offset, int length, ByteBuffer out)
   {
      boolean rval = false;
      
      try
      {
         if(initializeCipher(false))
         {
            // allocate space for output
            out.allocateSpace(mCipher.getOutputSize(length), true);
            
            // finish decryption
            int count = mCipher.doFinal(
               b, offset, length, out.getBytes(),
               out.getOffset() + out.getUsedSpace());
            
            // update bytes
            out.setBytes(
               out.getBytes(), out.getOffset(), out.getUsedSpace() + count);
            
            // reset cipher
            resetCipher();
            
            rval = true;
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "Could not finish decryption!,exception= " + t);
         getLogger().debug(getClass(), Logger.getStackTrace(t));
      }
      
      return rval;
   }
   
   /**
    * Encrypts a string of text using the internal key.
    *
    * @param text the string to encrypt.
    * 
    * @return the encrypted string or null.
    */
   public String encrypt(String text)
   {
      String encrypted = null;
      
      if(text != null)
      {
         // create a key if one does not exist
         if(mKey == null)
         {
            mKey = generateKey("AES");
         }
         
         if(mKey != null)
         {
            try
            {
               getLogger().detail(Cryptor.class, "encrypting text");
               
               // do one-off encryption
               Cipher cipher = Cipher.getInstance(
                  mKey.getAlgorithm(), "SunJCE");
               cipher.init(Cipher.ENCRYPT_MODE, mKey);
               byte[] out = cipher.doFinal(text.getBytes("UTF-8"));
               
               if(out != null)
               {
                  // base64-encode data
                  Base64Coder encoder = new Base64Coder();
                  encrypted = encoder.encode(out);
               }
               
               getLogger().detail(Cryptor.class, "text encrypted");
            }
            catch(Exception e)
            {
               getLogger().debug(Cryptor.class, Logger.getStackTrace(e));
            }
         }
      }
      
      return encrypted;
   }
   
   /**
    * Decrypts an encrypted base64 string of text using the internal key.
    *
    * @param data the string of base64 data to decrypt.
    * 
    * @return the decrypted text or null.
    */
   public String decrypt(String data)
   {
      String decrypted = null;
      
      if(mKey != null && data != null)
      {
         try
         {
            // for base64-decoding string
            Base64Coder decoder = new Base64Coder();
            
            // do one-off decryption
            Cipher cipher = Cipher.getInstance(mKey.getAlgorithm(), "SunJCE");
            cipher.init(Cipher.DECRYPT_MODE, mKey);
            byte[] out = cipher.doFinal(decoder.decode(data));
            
            if(out != null)
            {
               // UTF-8-encode and return the new string
               decrypted = new String(out, "UTF-8");
            }
         }
         catch(Exception e)
         {
            getLogger().debug(Cryptor.class, Logger.getStackTrace(e));
         }
      }
      
      return decrypted;
   }
   
   /**
    * Generates an internal secret key using the provided algorithm.
    *
    * @param algorithm the algorithm to use to generate the key.
    * 
    * @return true if successful, false if not.
    */
   public boolean generateInternalKey(String algorithm)
   {
      mKey = generateKey(algorithm);
      return mKey != null;
   }
   
   /**
    * Sets the internal key.
    * 
    * @param key the key to set as the internal key.
    */
   public void setKey(Key key)
   {
      mKey = key;
   }
   
   /**
    * Gets the internal key.
    * 
    * @return this cryptor's internal key.
    */
   public Key getKey()
   {
      return mKey;
   }
   
   /**
    * Resets the cipher for this Cryptor.
    */
   public void resetCipher()
   {
      mCipher = null;
   }
   
   /**
    * Generates a secret key using the provided algorithm.
    *
    * @param algorithm the algorithm to use to generate the key.
    * 
    * @return the generated secret key or null if failure.
    */
   public static SecretKey generateKey(String algorithm)
   {
      getLogger().debug(Cryptor.class, "generating key...");
      
      SecretKey key = null;

      try
      {
         // create the key generator, generate the key
         KeyGenerator kg = KeyGenerator.getInstance(algorithm, "SunJCE");
         SecureRandom sr = SecureRandom.getInstance("SHA1PRNG");
         kg.init(sr);
         key = kg.generateKey();
         
         getLogger().debug(Cryptor.class, "key generation complete.");         
      }
      catch(Exception e)
      {
         getLogger().debug(Cryptor.class, Logger.getStackTrace(e));
      }

      return key;
   }
   
   /**
    * Generates a secret key from a password.
    *
    * @param password the password to use to generate the key.
    * 
    * @return the generated secret key.
    */
   public static SecretKey generatePasswordKey(String password)
   {
      SecretKey secretKey = null;
      
      try
      {
         // create the key factory
         SecretKeyFactory skf =
            SecretKeyFactory.getInstance("PBEWithMD5AndDES", "SunJCE");
         
         // generate the key from the password
         PBEKeySpec desSpec = new PBEKeySpec(password.toCharArray());
         secretKey = skf.generateSecret(desSpec);
      }
      catch(Exception e)
      {
         getLogger().debug(Cryptor.class, Logger.getStackTrace(e));
      }

      return secretKey;
   }
   
   /**
    * Encrypts an array of bytes using a password.
    *
    * @param data the array of bytes to encrypt.
    * @param password the password to generate the key.
    * 
    * @return the encrypted data.
    */
   public static byte[] encrypt(byte[] data, String password)
   {
      return encrypt(data, smSalt, password);
   }
   
   /**
    * Encrypts an array of bytes using a password.
    *
    * @param data the array of bytes to encrypt.
    * @param salt the salt to use.
    * @param password the password to generate the key.
    * 
    * @return the encrypted data.
    */
   public static byte[] encrypt(byte[] data, byte[] salt, String password)
   {
      byte[] encrypted = null;
      
      if(data != null)
      {
         if(password == null)
         {
            password = "";
         }
      
         try
         {
            // generate a key from the password
            SecretKey key = generatePasswordKey(password);

            AlgorithmParameterSpec apSpec =
               new PBEParameterSpec(salt, smIterationCount);

            // use a cipher to encrypt
            Cipher cipher = Cipher.getInstance(key.getAlgorithm(), "SunJCE");
            cipher.init(Cipher.ENCRYPT_MODE, key, apSpec);
            encrypted = cipher.doFinal(data);
         }
         catch(Exception e)
         {
            getLogger().debug(Cryptor.class, Logger.getStackTrace(e));
         }
      }
      
      return encrypted;
   }
   
   /**
    * Encrypts a PKCS#8 private key.
    * 
    * @param bytes the DER bytes for the PKCS#8 private key.
    * @param password the password for the key.
    * 
    * @return a EncryptedPrivateKeyInfo ASN.1 structure in DER format with
    *         the encrypted private key and its encryption information -- or
    *         null if there is an error.
    */
   public static byte[] encryptPrivateKey(byte[] bytes, String password)
   {
      byte[] encodedBytes = null;
      
      try
      {
         // generate a key from the password
         SecretKey key = generatePasswordKey(password);
         
         // generate a new random salt
         Random random = new Random();
         byte[] salt = new byte[8];
         random.nextBytes(salt);

         // create the parameter spec
         AlgorithmParameterSpec apSpec =
            new PBEParameterSpec(salt, smIterationCount);

         // use a cipher to encrypt
         Cipher cipher = Cipher.getInstance(key.getAlgorithm(), "SunJCE");
         cipher.init(Cipher.ENCRYPT_MODE, key, apSpec);
         byte[] encryptedKeyBytes = cipher.doFinal(bytes);
         
         // create encrypted private key info
         EncryptedPrivateKeyInfo epki = new EncryptedPrivateKeyInfo(
            cipher.getParameters(), encryptedKeyBytes);
         
         // get DER encoded bytes
         encodedBytes = epki.getEncoded();
      }
      catch(Throwable t)
      {
         getLogger().debug(Cryptor.class, Logger.getStackTrace(t));
      }
      
      return encodedBytes;
   }
   
   /**
    * Decrypts an encrypted array of bytes using a password.
    *
    * @param data the array of bytes to decrypt.
    * @param password the password to generate the key.
    * 
    * @return the decrypted data.
    */
   public static byte[] decrypt(byte[] data, String password)
   {
      return decrypt(data, smSalt, password);
   }

   /**
    * Decrypts an encrypted array of bytes using a password.
    *
    * @param data the array of bytes to decrypt.
    * @param salt the salt to use.
    * @param password the password to generate the key.
    * 
    * @return the decrypted data.
    */
   public static byte[] decrypt(byte[] data, byte[] salt, String password)
   {
      byte[] decrypted = null;
      
      if(data != null)
      {
         if(password == null)
         {
            password = "";
         }
      
         try
         {
            // generate a key from the password
            SecretKey key = generatePasswordKey(password);
            
            AlgorithmParameterSpec apSpec =
               new PBEParameterSpec(salt, smIterationCount);
         
            // use a cipher to decrypt
            Cipher cipher = Cipher.getInstance(key.getAlgorithm());
            cipher.init(Cipher.DECRYPT_MODE, key, apSpec);
            decrypted = cipher.doFinal(data);
         }
         catch(Exception e)
         {
            getLogger().debug(Cryptor.class, Logger.getStackTrace(e));
         }
      }

      return decrypted;
   }
   
   /**
    * Decrypts an encrypted PKCS#8 private key. The passed bytes should be
    * an EncryptedPrivateKeyInfo ASN.1 structure in DER format.
    * 
    * @param bytes the encrypted PKCS#8 private key bytes.
    * @param password the password for the key.
    * 
    * @return the decrypted bytes (PKCS#8 DER encoded) or null if the
    *         password failed.
    */
   public static byte[] decryptPrivateKey(byte[] bytes, String password)
   {
      byte[] decryptedBytes = null;
      
      try
      {
         // get encrypted private key information
         EncryptedPrivateKeyInfo epki = new EncryptedPrivateKeyInfo(bytes);
         
         // create the decryption key based on the password
         SecretKeyFactory sf = SecretKeyFactory.getInstance(epki.getAlgName());
         PBEKeySpec keySpec = new PBEKeySpec(password.toCharArray());
         Key decryptionKey = sf.generateSecret(keySpec);
         
         // create a cipher to decrypt
         Cipher cipher = Cipher.getInstance(decryptionKey.getAlgorithm());
         cipher.init(
            Cipher.DECRYPT_MODE, decryptionKey, epki.getAlgParameters());
         
         // get the PKCS8 encoded key spec and the encoded bytes
         PKCS8EncodedKeySpec pkcs8 = epki.getKeySpec(cipher);
         decryptedBytes = pkcs8.getEncoded();

         // clear out the password for security
         keySpec.clearPassword();
      }
      catch(Throwable t)
      {
         getLogger().debug(Cryptor.class, Logger.getStackTrace(t));
      }
      
      return decryptedBytes;
   }
   
   /**
    * Encrypts a string of text using a password.
    *
    * @param text the string to encrypt.
    * @param password the password to generate the key.
    * 
    * @return the encrypted string or null.
    */
   public static String encrypt(String text, String password)
   {
      String encrypted = null;
      
      if(text != null)
      {
         try
         {
            // UTF-8-encode and encrypt the text
            byte[] data = encrypt(text.getBytes("UTF-8"), password);

            if(data != null)
            {
               // transform it back into a string using Base64-encoding
               Base64Coder encoder = new Base64Coder();
               encrypted = encoder.encode(data);
            }
         }
         catch(Exception e)
         {
            getLogger().debug(Cryptor.class, Logger.getStackTrace(e));
         }
      }

      return encrypted;
   }
   
   /**
    * Decrypts an encrypted string of text using a password.
    *
    * @param data the string of data to decrypt.
    * @param password the password to generate the key.
    * 
    * @return the decrypted text or null.
    */
   public static String decrypt(String data, String password)
   {
      String decrypted = null;
      
      if(data != null)
      {
         try
         {
            // transform the string into an array of bytes, and decrypt
            Base64Coder decoder = new Base64Coder();
            byte[] bytes = decrypt(decoder.decode(data), password);

            if(bytes != null)
            {
               // UTF-8-encode and return the new string
               decrypted = new String(bytes, "UTF-8");
            }
         }
         catch(Exception e)
         {
            getLogger().debug(Cryptor.class, Logger.getStackTrace(e));
         }
      }

      return decrypted;      
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
