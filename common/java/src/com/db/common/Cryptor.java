/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import com.db.common.logging.Logger;
import com.db.common.logging.LoggerManager;

import java.io.File;
import java.io.FileInputStream;
import java.io.UnsupportedEncodingException;
import java.security.Key;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.Signature;
import java.security.spec.AlgorithmParameterSpec;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.PBEParameterSpec;

/**
 * A class for encrypting and decrypting data.
 * 
 * @author Dave Longley
 */
public class Cryptor
{
   /**
    * The key used to decrypt the encrypted data.
    */
   protected Key mKey = null;

   /**
    * A stock salt to be used in an encryption process.
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
   }
   
   /**
    * Gets a checksum for the passed text.
    *
    * @param text the text to run the algorithm on.
    * @param algorithm the algorithm to run.
    * @return an array of bytes that represents the checksum or null.
    * @throws NoSuchAlgorithmException
    * @throws UnsupportedEncodingException
    */
   public static byte[] getChecksumBytes(String text, String algorithm)
      throws NoSuchAlgorithmException, UnsupportedEncodingException
   {
      byte[] checksum = null;
      
      if(text != null)
      {
         // obtain the checksum using the passed algorithm
         MessageDigest md = MessageDigest.getInstance(algorithm);
         md.update(text.getBytes("UTF-8"));
         checksum = md.digest();
      }
      
      return checksum;
   }   

   /**
    * Gets the checksum for the passed file.
    *
    * @param file the file to run the algorithm on.
    * @param algorithm the algorithm to run.
    * @return an array of bytes that represents the checksum or null.
    * @throws NoSuchAlgorithmException
    * @throws UnsupportedEncodingException
    */
   public static byte[] getChecksumBytes(File file, String algorithm)
      throws NoSuchAlgorithmException
   {
      byte[] checksum = null;
      
      // obtain the checksum using the passed algorithm
      MessageDigest md = MessageDigest.getInstance(algorithm);

      if(file != null && file.isFile())
      {
         FileInputStream fis = null;
         
         try
         {
            // open file
            fis = new FileInputStream(file);
            
            byte[] buffer = new byte[1024];
            int numBytes = -1;
            while((numBytes = fis.read(buffer)) != -1)
            {
               // update digest
               md.update(buffer, 0, numBytes);
            }
            
            // close file
            fis.close();
            
            checksum = md.digest();
         }
         catch(Throwable t)
         {
            getLogger().error(
                  "could not get file checksum,algorithm=" + algorithm);
         }
         
         if(fis != null)
         {
            try
            {
               fis.close();
            }
            catch(Throwable t)
            {
            }
         }
      }
      
      return checksum;
   }
   
   /**
    * Gets the checksum for the passed text.
    *
    * @param text the text to run the algorithm on.
    * @param algorithm the algorithm to run.
    * @return the checksum in a hexidecimal formatted string or null.
    * @throws NoSuchAlgorithmException
    * @throws UnsupportedEncodingException
    */
   public static String getChecksumString(String text, String algorithm)
      throws NoSuchAlgorithmException, UnsupportedEncodingException
   {
      // obtain the checksum using the passed algorithm and convert to hex
      return convertBytesToHex(getChecksumBytes(text, algorithm));
   }
   
   /**
    * Gets the checksum for the passed file.
    *
    * @param file the file to run the algorithm on.
    * @param algorithm the algorithm to run.
    * @return the checksum in a hexidecimal formatted string or null.
    * @throws NoSuchAlgorithmException
    */
   public static String getChecksumString(File file, String algorithm)
      throws NoSuchAlgorithmException
   {
      // obtain the checksum using the passed algorithm and convert to hex
      return convertBytesToHex(getChecksumBytes(file, algorithm));
   }
   
   /**
    * Gets the MD5 checksum for the passed text.
    *
    * @param text the text to run the MD5 algorithm on.
    * @return an array of bytes that represents the MD5 checksum or null.
    * @throws NoSuchAlgorithmException
    * @throws UnsupportedEncodingException
    */
   public static byte[] getMD5ChecksumBytes(String text)
      throws NoSuchAlgorithmException, UnsupportedEncodingException
   {
      return getChecksumBytes(text, "MD5");
   }
   
   /**
    * Gets the MD5 checksum for the passed file.
    *
    * @param file the file to run the MD5 algorithm on.
    * @return an array of bytes that represents the MD5 checksum or null.
    * @throws NoSuchAlgorithmException
    * @throws UnsupportedEncodingException
    */
   public static byte[] getMD5ChecksumBytes(File file)
      throws NoSuchAlgorithmException
   {
      return getChecksumBytes(file, "MD5");
   }
   
   /**
    * Gets the MD5 checksum for the passed text.
    *
    * @param text the text to run the MD5 algorithm on.
    * @return the md5 checksum in a hexidecimal formatted string or null.
    * @throws NoSuchAlgorithmException
    * @throws UnsupportedEncodingException
    */
   public static String getMD5ChecksumString(String text)
      throws NoSuchAlgorithmException, UnsupportedEncodingException
   {
      // obtain the checksum using the MD5 algorithm and convert to hex
      return getChecksumString(text, "MD5");
   }
   
   /**
    * Gets the MD5 checksum for the passed file.
    *
    * @param file the file to run the MD5 algorithm on.
    * @return the md5 checksum in a hexidecimal formatted string or null.
    * @throws NoSuchAlgorithmException
    */
   public static String getMD5ChecksumString(File file)
      throws NoSuchAlgorithmException
   {
      return getChecksumString(file, "MD5");
   }

   /**
    * Gets the SHA1 checksum for the passed text.
    *
    * @param text the text to run the SHA1 algorithm on.
    * @return an array of bytes that represents the SHA1 checksum or null.
    * @throws NoSuchAlgorithmException
    * @throws UnsupportedEncodingException
    */
   public static byte[] getSHA1ChecksumBytes(String text)
      throws NoSuchAlgorithmException, UnsupportedEncodingException
   {
      return getChecksumBytes(text, "SHA1");
   }   
   
   /**
    * Gets the SHA1 checksum for the passed file.
    *
    * @param file the file to run the SHA1 algorithm on.
    * @return an array of bytes that represents the SHA1 checksum or null.
    * @throws NoSuchAlgorithmException
    * @throws UnsupportedEncodingException
    */
   public static byte[] getSHA1ChecksumBytes(File file)
      throws NoSuchAlgorithmException
   {
      return getChecksumBytes(file, "SHA1");
   }
   
   /**
    * Gets the SHA1 checksum for the passed text.
    *
    * @param text the text to run the SHA1 algorithm on.
    * @return the SHA1 checksum in a hexidecimal formatted string or null.
    * @throws NoSuchAlgorithmException
    * @throws UnsupportedEncodingException
    */
   public static String getSHA1ChecksumString(String text)
      throws NoSuchAlgorithmException, UnsupportedEncodingException
   {
      // obtain the checksum using the SHA1 algorithm and convert to hex
      return getChecksumString(text, "SHA1");
   }
   
   /**
    * Gets the SHA1 checksum for the passed file.
    *
    * @param file the file to run the SHA1 algorithm on.
    * @return the SHA1 checksum in a hexidecimal formatted string or null.
    * @throws NoSuchAlgorithmException
    */
   public static String getSHA1ChecksumString(File file)
      throws NoSuchAlgorithmException
   {
      return getChecksumString(file, "SHA1");
   }
   
   /**
    * Converts an array of bytes into a hexidecimal string.
    * 
    * @param bytes the array of bytes to convert.
    * @return the hexidecimal string.
    */
   public static String convertBytesToHex(byte[] bytes)
   {
      String s = "";
      
      if(bytes != null)
      {
         // use a string buffer to build the hexidecimal string
         // make sure the buffer is large enough
         StringBuffer sb = new StringBuffer(bytes.length * 2);
         for(int i = 0; i < bytes.length; i++)
         {
            // interpret byte as an int chopped down to byte size
            int hex = bytes[i] & 0xff;
         
            // if its less than 10 hex, append a zero
            if(hex < 0x10)
            {
               sb.append("0");
            }
         
            // add the value in base 16
            sb.append(Integer.toString(hex, 16));
         }
         
         s = sb.toString();
      }
      
      return s;   
   }

   /**
    * Signs the passed byte array with the passed private key and
    * returns the signature.
    *
    * @param data the data to sign.
    * @param privateKey the privateKey to sign with.
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
            // get the key's algorithm
            String algorithm = privateKey.getAlgorithm();
         
            // sign the signature string with the private key
            Signature signature = Signature.getInstance(algorithm);
            signature.initSign(privateKey);
            signature.update(data);
            
            // sign the data
            sig = signature.sign();
         }
         catch(Exception e)
         {
            getLogger().debug(Logger.getStackTrace(e));
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
    * @return the signature if successfully signed, null if not. 
    */
   public static byte[] sign(String text, PrivateKey privateKey)
   {
      byte[] data = null;

      try
      {
         data = text.getBytes("UTF-8");
      }
      catch(Exception e)
      {
         getLogger().debug(Logger.getStackTrace(e));
      }

      return sign(data, privateKey);
   }
   
   /**
    * Signs the passed byte array with the passed private key and
    * returns the signature.
    *
    * @param data the data to sign.
    * @param key the Base64-PKCS8 privateKey to sign with.
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
    * @return true if verified, false if not.
    */
   public static boolean verify(byte[] sig, byte[] data, PublicKey key)
   {
      boolean rval = false;

      if(key != null)
      {
         try
         {         
            // verify the digital signature
            Signature signature = Signature.getInstance(key.getAlgorithm());
            signature.initVerify(key);
            signature.update(data);
         
            rval = signature.verify(sig);
         }
         catch(Exception e)
         {
            getLogger().debug(Logger.getStackTrace(e));
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
         getLogger().debug(Logger.getStackTrace(e));
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
    * @return true if verified, false if not.
    */
   public static boolean verify(byte[] sig, String text, String key)
   {
      // obtain the decoded public key
      PublicKey publicKey = KeyManager.decodePublicKey(key);
      return verify(sig, text, publicKey);
   }

   /**
    * Generates an internal secret key using the provided algorithm.
    *
    * @param algorithm the algorithm to use to generate the key.
    * @return true if successful, false if not.
    */
   public boolean generateInternalKey(String algorithm)
   {
      mKey = generateKey(algorithm);
      return mKey != null;
   }
   
   /**
    * Generates a secret key using the provided algorithm.
    *
    * @param algorithm the algorithm to use to generate the key.
    * @return the generated secret key or null if failure.
    */
   public static SecretKey generateKey(String algorithm)
   {
      getLogger().debug("generating key...");
      
      SecretKey key = null;

      try
      {
         // create the key generator, generate the key
         KeyGenerator kg = KeyGenerator.getInstance(algorithm, "SunJCE");
         SecureRandom sr = SecureRandom.getInstance("SHA1PRNG");
         kg.init(sr);
         key = kg.generateKey();
         
         getLogger().debug("key generation complete.");         
      }
      catch(Exception e)
      {
         getLogger().debug(Logger.getStackTrace(e));
      }

      return key;
   }
   
   /**
    * Generates a secret key from a password.
    *
    * @param password the password to use to generate the key.
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
         getLogger().debug(Logger.getStackTrace(e));
      }

      return secretKey;
   }

   /**
    * Encrypts an array of bytes of length starting at offset using
    * the passed key.
    *
    * @param data the array of bytes to encrypt.
    * @param offset the offset to start at.
    * @param length the number of bytes to encrypt.
    * @param key the key to encrypt with.
    * @return the encrypted data.
    */
   public static byte[] encrypt(byte[] data, int offset, int length, Key key)
   {
      byte[] encrypted = null;
      
      if(key != null && data != null)
      {
         try
         {
            // use a cipher to encrypt
            Cipher cipher = Cipher.getInstance(key.getAlgorithm(), "SunJCE");
            cipher.init(Cipher.ENCRYPT_MODE, key);
            encrypted = cipher.doFinal(data, offset, length);
         }
         catch(Exception e)
         {
            getLogger().debug(Logger.getStackTrace(e));
         }
      }

      return encrypted;
   }
   
   /**
    * Encrypts an array of bytes with the passed key.
    *
    * @param data the array of bytes to encrypt.
    * @param key the key to encrypt with.
    * @return the encrypted data.
    */
   public static byte[] encrypt(byte[] data, Key key)
   {
      byte[] encrypted = null;
      
      if(data != null)
      {
         encrypted = encrypt(data, 0, data.length, key);
      }
      
      return encrypted;
   }

   /**
    * Encrypts an array of bytes of length starting at offset using
    * an internal key.
    *
    * @param data the array of bytes to encrypt.
    * @param offset the offset to start at.
    * @param length the number of bytes to encrypt.
    * @return the encrypted data.
    */
   public byte[] encrypt(byte[] data, int offset, int length)
   {
      if(mKey == null)
      {
         generateInternalKey("DES");
      }
      
      return encrypt(data, offset, length, mKey);
   }
   
   /**
    * Encrypts an array of bytes using an internal key.
    *
    * @param data the array of bytes to encrypt.
    * @return the encrypted data.
    */
   public byte[] encrypt(byte[] data)
   {
      return encrypt(data, 0, data.length);
   }
   
   /**
    * Encrypts an array of bytes using a password.
    *
    * @param data the array of bytes to encrypt.
    * @param password the password to generate the key.
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
            getLogger().debug(Logger.getStackTrace(e));
         }
      }
      
      return encrypted;
   }
   
   /**
    * Decrypts an array of bytes of length starting at offset using
    * the specified key.
    *
    * @param data the array of bytes to decrypt.
    * @param offset the offset to start at.
    * @param length the number of bytes to decrypt.
    * @param key the key to decrypt with.
    * @return the decrypted data.
    */   
   public static byte[] decrypt(byte[] data, int offset, int length, Key key)
   {
      byte[] decrypted = null;
      
      if(key != null && data != null)
      {
         try
         {
            // use a cipher to decrypt
            Cipher cipher = Cipher.getInstance(key.getAlgorithm());
            cipher.init(Cipher.DECRYPT_MODE, key);
            decrypted = cipher.doFinal(data, offset, length);
         }
         catch(Exception e)
         {
            getLogger().debug(Logger.getStackTrace(e));
         }
      }

      return decrypted;
   }
   
   /**
    * Decrypts an encrypted array of bytes using the specified key.
    *
    * @param data the array of bytes to decrypt.
    * @param key the key to decrypt with.
    * @return the decrypted data.
    */
   public static byte[] decrypt(byte[] data, Key key)
   {
      byte[] decrypted = null;
      
      if(data != null)
      {
         decrypted = decrypt(data, 0, data.length, key);
      }
      
      return decrypted;
   }

   /**
    * Decrypts an array of bytes of length starting at offset using
    * an internal key.
    *
    * @param data the array of bytes to decrypt.
    * @param offset the offset to start at.
    * @param length the number of bytes to decrypt.
    * @return the decrypted data.
    */   
   public byte[] decrypt(byte[] data, int offset, int length)
   {
      return decrypt(data, offset, length, mKey); 
   }

   /**
    * Decrypts an encrypted array of bytes using an internal key.
    *
    * @param data the array of bytes to decrypt.
    * @return the decrypted data.
    */
   public byte[] decrypt(byte[] data)
   {
      return decrypt(data, 0, data.length);
   }

   /**
    * Decrypts an encrypted array of bytes using a password.
    *
    * @param data the array of bytes to decrypt.
    * @param password the password to generate the key.
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
            getLogger().debug(Logger.getStackTrace(e));
         }
      }

      return decrypted;
   }

   /**
    * Encrypts a string of text.
    *
    * @param text the string to encrypt.
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
            mKey = generateKey("DES");
         }

         if(mKey != null)
         {
            try
            {
               getLogger().debug("encrypting text");
               
               // UTF-8-encode and encrypt the text
               byte[] data = encrypt(text.getBytes("UTF-8"));

               if(data != null)
               {
                  // transform it back into a string using Base64-encoding
                  Base64Coder encoder = new Base64Coder();
                  encrypted = encoder.encode(data);
               }
               
               getLogger().debug("text encrypted");
            }
            catch(Exception e)
            {
               getLogger().debug(Logger.getStackTrace(e));
            }
         }
      }

      return encrypted;
   }

   /**
    * Encrypts a string of text using a password.
    *
    * @param text the string to encrypt.
    * @param password the password to generate the key.
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
            getLogger().debug(Logger.getStackTrace(e));
         }
      }

      return encrypted;
   }

   /**
    * Decrypts an encrypted string of text.
    *
    * @param data the string of data to decrypt.
    * @return the decrypted text or null.
    */
   public String decrypt(String data)
   {
      String decrypted = null;
      
      if(mKey != null && data != null)
      {
         try
         {
            // transform the string into an array of bytes, and decrypt
            Base64Coder decoder = new Base64Coder();
            byte[] bytes = decrypt(decoder.decode(data));

            if(bytes != null)
            {
               // UTF-8-encode and return the new string
               decrypted = new String(bytes, "UTF-8");
            }
         }
         catch(Exception e)
         {
            getLogger().debug(Logger.getStackTrace(e));
         }
      }

      return decrypted;
   }

   /**
    * Decrypts an encrypted string of text using a password.
    *
    * @param data the string of data to decrypt.
    * @param password the password to generate the key.
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
            getLogger().debug(Logger.getStackTrace(e));
         }
      }

      return decrypted;      
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
    * Gets the logger.
    * 
    * @return the logger.
    */
   public static Logger getLogger()
   {
      return LoggerManager.getLogger("dbcommon");
   }
}
