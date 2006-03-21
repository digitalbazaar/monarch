/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import com.db.common.logging.Logger;
import com.db.common.logging.LoggerManager;

import java.security.PrivateKey;

/**
 * A convenient private key cryptor class. Used to generate and store
 * encrypted private keys on disk and in memory.
 * 
 * @author Dave Longley
 */
public class PrivateKeyCryptor
{
   /**
    * The cryptor for the private key.
    */
   protected Cryptor mKeyCryptor;

   /**
    * The encrypted private key.
    */
   protected byte[] mPrivateKey = null;
   
   /**
    * The plain-text public key.
    */
   protected String mPublicKey;

   /**
    * The cryptor for the private key password.
    */
   protected Cryptor mPasswordCryptor;

   /**
    * The encrypted private key password.
    */
   protected String mEncryptedPassword;
   
   /**
    * The name of the private key file.
    */
   protected String mKeyFilename;
   
   /**
    * The error associated with loading a private key, etc, if any.
    */
   protected String mError;
   
   /**
    * Creates a new private key cryptor. The private key filename and
    * password for the private key must be set before trying to load
    * a private key.
    */
   public PrivateKeyCryptor()
   {
      mKeyCryptor = null;
      mPrivateKey = null;
      mKeyFilename = null;
      mEncryptedPassword = null;
      mError = "";
   }   
   
   /**
    * Creates a new private key cryptor.
    * 
    * @param keyFilename the name of the file to store the private key in.
    * @param password the password to store the file with.
    */
   public PrivateKeyCryptor(String keyFilename, String password)
   {
      mKeyCryptor = null;
      mPrivateKey = null;
      mKeyFilename = keyFilename;
      mError = "";
      
      storePasswordInMemory(password);
   }
   
   /**
    * Convenience method. Encrypts and stores the private key password
    * in memory.
    * 
    * @param password the plain-text password to store.
    * @return true if successfully stored, false if not.
    */
   protected boolean storePasswordInMemory(String password)
   {
      boolean rval = false;
      
      // get a new password cryptor
      mPasswordCryptor = new Cryptor();
      
      // store the password
      mEncryptedPassword = mPasswordCryptor.encrypt(password);
      
      if(mEncryptedPassword != null)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Convenience method. Retrieves the encrypted private key password from
    * memory and decrypts it into plain-text.
    * 
    * @return the decrypted plain-text password or null if there was
    *         an error.
    */
   protected String getDecryptedPassword()
   {
      String password = null;
      
      if(mPasswordCryptor != null)
      {
         password = mPasswordCryptor.decrypt(mEncryptedPassword);
      }
      
      return password;
   }
   
   /**
    * Convenience method. Takes a key manager and stores its private key
    * in an encrypted string.
    * 
    * @param km the key manager to get the private key from.
    * @return true if successfully stored, false if not.
    */
   protected boolean storePrivateKeyInMemory(KeyManager km) 
   {
      boolean rval = false;
      
      // store the encrypted private key in memory
      if(km.getPrivateKey() != null)
      {
         // get a fresh cryptor
         mKeyCryptor = new Cryptor();
         
         // store the encrypted private key
         byte[] encodedBytes = km.getPrivateKey().getEncoded();
         mPrivateKey = mKeyCryptor.encrypt(encodedBytes);
         
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Clears the private key from memory.
    */
   protected void clearPrivateKeyFromMemory()
   {
      mPrivateKey = null;
   }
   
   /**
    * Convenience method. Gets the private key as an encoded array of bytes.
    *
    * @return the encoded private key bytes or null.
    */
   protected byte[] getPrivateKeyEncodedBytes()
   {
      byte[] encodedKey = null;
      
      if(mPrivateKey == null)
      {
         String password = getDecryptedPassword();
         if(password != null)
         {
            // load key from disk
            KeyManager km = new KeyManager();
            if(km.loadPrivateKey(mKeyFilename, password))
            {
               // store the private key in memory
               storePrivateKeyInMemory(km);
            }
            else
            {
               setError(km.getError());
               
               getLogger().debug(
                  "PrivateKeyCryptor: ERROR - invalid password!");
            }
         }
         else
         {
            getLogger().debug(
               "PrivateKeyCryptor: ERROR - cannot use null password!");
         }
      }

      // make sure private key has been stored
      if(mPrivateKey != null)
      {
         // decrypt the private key
         encodedKey = mKeyCryptor.decrypt(mPrivateKey);
      }
      
      return encodedKey;
   }
   
   /**
    * Sets the error that occurred.
    * 
    * @param error the error to set.
    */
   public void setError(String error)
   {
      mError = error;
   }
   
   /**
    * Generates a new set of public and private keys using the
    * password and keyfile in memory. Will overwrite the old keys stored
    * in memory and write the private key to disk.
    * 
    * @return true if successful, false if not.
    */
   public boolean generateKeys()
   {
      return generateKeys(getPrivateKeyFilename(), getDecryptedPassword());
   }
   
   /**
    * Generates a new set of public and private keys using the private key
    * filename in memory. Will overwrite the old keys stored in memory and
    * write the private key to disk.
    * 
    * @param password the password to use.
    * @return true if successful, false if not.
    */
   public boolean generateKeys(String password)
   {
      return generateKeys(getPrivateKeyFilename(), password);
   }
   
   /**
    * Generates a new set of public and private keys. Will overwrite
    * the old keys stored in memory and save the private key on disk.
    * 
    * @param keyFilename the private key filename to use.
    * @param password the password to use.
    * @return true if successful, false if not.
    */
   public boolean generateKeys(String keyFilename, String password)
   {
      boolean rval = false;
      
      if(password != null)
      {
         // update private key file name
         mKeyFilename = keyFilename;
         
         // create a key manager and generate a pair of public/private keys
         KeyManager km = new KeyManager();
         if(km.generateKeyPair())
         {
            // store the new password
            if(storePasswordInMemory(password))
            {
               // get the private key password
               // we decrypt the password here to verify that it
               // was stored in memory
               password = getDecryptedPassword();
               if(password != null)
               {
                  // store private key on disk
                  if(km.storePrivateKey(mKeyFilename, password))
                  {
                     // store the keys in memory
                     if(storePrivateKeyInMemory(km))
                     {
                        // store the public key in memory
                        mPublicKey = km.getPublicKeyString();
                     
                        rval = true;
                     }
                     else
                     {
                        getLogger().debug(
                           "PrivateKeyCryptor: ERROR - could not store keys!");
                     }
                  }
                  else
                  {
                     getLogger().debug( 
                        "PrivateKeyCryptor: ERROR - could not store key file!");
                  }
               }
               else
               {
                  getLogger().debug(
                     "PrivateKeyCryptor: ERROR - could not decrypt password!");
               }
            }
            else
            {
               getLogger().debug(
                  "PrivateKeyCryptor: ERROR - could not store " +
                  "encrypted password!");
            }
         }
         else
         {
            getLogger().debug(
               "PrivateKeyCryptor: ERROR - could not generate keys!");
         }
      }
      else
      {
         getLogger().debug(
            "PrivateKeyCryptor: ERROR - cannot generate keys with " +
            "null password!");
      }

      return rval;
   }
   
   /**
    * Generates a new set of public and private keys. Will overwrite
    * the old keys stored in memory. The private key will not be written to
    * disk.
    * 
    * @param password the password to use.
    * @return true if successful, false if not.
    */
   public boolean generateKeysInMemory(String password)
   {
      boolean rval = false;
      
      if(password != null)
      {
         // create a key manager and generate a pair of public/private keys
         KeyManager km = new KeyManager();
         if(km.generateKeyPair())
         {
            // store the new password
            if(storePasswordInMemory(password))
            {
               // get the private key password
               // we decrypt the password here to verify that it
               // was stored in memory
               password = getDecryptedPassword();
               if(password != null)
               {
                  // store the public key in memory
                  mPublicKey = km.getPublicKeyString();
                  rval = true;
               }
               else
               {
                  getLogger().debug(
                     "PrivateKeyCryptor: ERROR - could not decrypt password!");
               }
            }
            else
            {
               getLogger().debug(
                  "PrivateKeyCryptor: ERROR - could not store " +
                  "encrypted password!");
            }
         }
         else
         {
            getLogger().debug(
               "PrivateKeyCryptor: ERROR - could not generate keys!");
         }
      }
      else
      {
         getLogger().debug(
            "PrivateKeyCryptor: ERROR - cannot generate keys with " +
            "null password!");
      }

      return rval;
   }
   
   /**
    * Retrieves the encrypted private key password from memory.
    * 
    * @return the encryped private key password or null.
    */
   public String getEncryptedPassword()
   {
      return mEncryptedPassword;
   }
   
   /**
    * Decrypts the encrypted in-memory private key password and returns
    * it in plain-text.
    * 
    * @return the plain-text password.
    */
   public String getPlainTextPassword()
   {
      return getDecryptedPassword();
   }
   
   /**
    * Sets the private key.
    * 
    * @param pkey the private key.
    * @return true if the private key was set, false if not.
    */
   public boolean setPrivateKey(PrivateKey pkey)
   {
      boolean rval = false;
      
      // store the encrypted private key in memory
      if(pkey != null)
      {
         // get a fresh cryptor
         mKeyCryptor = new Cryptor();
         
         // store the encrypted private key
         byte[] encodedBytes = pkey.getEncoded();
         mPrivateKey = mKeyCryptor.encrypt(encodedBytes);
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Sets the private key from the passed encrypted key (encoded in base64)
    * that is locked with the passed password.
    *
    * @param encryptedKey the encrypted key in a base64-encoded string.
    * @param password the password to unlock the file.
    * @return true if successful, false if not.
    */
   public boolean setEncryptedPrivateKey(String encryptedKey, String password)
   {
      boolean rval = false;
      
      Base64Coder base64 = new Base64Coder();
      byte[] bytes = base64.decode(encryptedKey);
      rval = setEncryptedPrivateKey(bytes, password);
      
      return rval;
   }
   
   /**
    * Sets the private key from the passed encrypted key bytes
    * that are locked with the passed password.
    *
    * @param encryptedKey the encrypted key in a byte array.
    * @param password the password to unlock the file.
    * @return true if successful, false if not.
    */
   public boolean setEncryptedPrivateKey(byte[] encryptedKey, String password)
   {
      boolean rval = false;
      
      try
      {
         // decrypt the key with the passed password
         byte[] decryptedKey = Cryptor.decrypt(encryptedKey, password);

         // if decryptedKey is not null, decode the key
         if(decryptedKey != null)
         {
            rval = setPrivateKey(KeyManager.decodePrivateKey(decryptedKey));
         }
      }
      catch(Throwable t)
      {
         getLogger().error(
            "PrivateKeyCryptor: ERROR - Unable to load encrypted private key.");
         getLogger().debug(Logger.getStackTrace(t));
      }

      return rval;      
   }

   /**
    * Gets the private key as java.security.PrivateKey object.
    *
    * @return the private key or null.
    */
   public PrivateKey getPrivateKey()
   {
      PrivateKey pkey = null;
      
      byte[] encodedKey = getPrivateKeyEncodedBytes();
      if(encodedKey != null)
      {
         // decode the private key
         pkey = KeyManager.decodePrivateKey(encodedKey);
      }
      else
      {
         getLogger().debug(
            "PrivateKeyCryptor: ERROR - could not get encoded key!");
      }
      
      return pkey;
   }
   
   /**
    * Gets the private key as a PKCS8-Base64 string.
    *
    * @return the private key or null.
    */
   public String getPrivateKeyString()
   {
      String pkey = null;
      
      byte[] encodedKey = getPrivateKeyEncodedBytes();
      if(encodedKey != null)
      {
         // encode the private key as a string
         pkey = KeyManager.encodeKey(encodedKey);
      }
      else
      {
         getLogger().debug(
            "PrivateKeyCryptor: ERROR - could not get encoded key!");
      }
      
      return pkey;
   }
   
   /**
    * Gets the private key in encrypted form.
    *
    * @return the encrypted private key bytes or null.
    */
   public byte[] getEncryptedPrivateKeyBytes()
   {
      if(mPrivateKey == null)
      {
         String password = getDecryptedPassword();
         if(password != null)
         {
            // load key from disk
            KeyManager km = new KeyManager();
            if(km.loadPrivateKey(mKeyFilename, password))
            {
               // store the private key in memory
               storePrivateKeyInMemory(km);
            }
            else
            {
               setError(km.getError());
               
               getLogger().debug(
                  "PrivateKeyCryptor: ERROR - invalid password!");
            }
         }
         else
         {
            getLogger().debug(
               "PrivateKeyCryptor: ERROR - cannot use null password!");
         }
      }

      return mPrivateKey;
   }
   
   /**
    * Gets the private key in encrypted form as a base64-encoded string.
    *
    * @return the encrypted private key bytes base64-encoded or null.
    */
   public String getEncryptedPrivateKeyString()
   {
      String rval = null;
      
      byte[] encryptedBytes = getEncryptedPrivateKeyBytes();
      if(encryptedBytes != null)
      {
         Base64Coder base64 = new Base64Coder();
         rval = base64.encode(encryptedBytes);
      }
      
      return rval;
   }
   
   /**
    * Gets the public key as a X.509-Base64 string.
    *
    * @return the public key or null.
    */
   public String getPublicKeyString()
   {
      return mPublicKey;
   }
   
   /**
    * Tries to verify that the password stored in memory unlocks the
    * private key stored in the private key file.
    * 
    * @return true if verified, false if not.
    */
   public boolean verify()
   {
      return verify(getDecryptedPassword());
   }
   
   /**
    * Sets the password and then tries to verify that the password unlocks the
    * private key stored in the private key file.
    * 
    * @param password the password to set and verify.
    * @return true if verified, false if not. 
    */
   public boolean verify(String password)
   {
      return verify(getPrivateKeyFilename(), password);
   }
   
   /**
    * Sets the private key filename and the password and then tries to
    * verify that the password unlocks the private key stored in the
    * private key file.
    * 
    * @param keyFilename the private key filename.
    * @param password the password to set and verify.
    * @return true if verified, false if not. 
    */
   public boolean verify(String keyFilename, String password)
   {
      boolean rval = false;
      
      if(setPassword(password))
      {
         setPrivateKeyFilename(keyFilename);
         
         if(getPrivateKey() != null)
         {
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Sets the private key password that is stored in memory. This does
    * not update the private key file.
    * 
    * @param password the plain-text password.
    * @return true if successful, false if not.
    */
   public boolean setPassword(String password)
   {
      clearPrivateKeyFromMemory();
      
      return storePasswordInMemory(password);
   }
   
   /**
    * Sets the private key filename.
    * 
    * @param keyFilename the name of the private key file.
    */
   public void setPrivateKeyFilename(String keyFilename)
   {
      clearPrivateKeyFromMemory();
      
      mKeyFilename = keyFilename;
   }
   
   /**
    * Gets the previously set private key filename.
    * 
    * @return the private key filename.
    */
   public String getPrivateKeyFilename()
   {
      return mKeyFilename;
   }
   
   /**
    * Clears the private key password and private key from memory.
    */
   public void clear()
   {
      clearPrivateKeyFromMemory();
      mEncryptedPassword = null;
   }
   
   /**
    * Gets the error that was associated with loading a private key, etc,
    * if there was any.
    * 
    * @return the error associated with loading a private key, etc, if any.
    */
   public String getError()
   {
      return mError;
   }
   
   /**
    * Gets the logger for this private key cryptor.
    * 
    * @return the logger for this private key cryptor.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbcommon");
   }
}
