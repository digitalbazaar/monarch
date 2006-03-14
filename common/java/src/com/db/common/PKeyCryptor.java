/*
 * Copyright (c) 2003 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.common;

import com.db.common.logging.LoggerManager;

import java.security.PrivateKey;

/**
 * A convenient private key cryptor class. Used to generate and store
 * encrypted private keys on disk and in memory.
 * 
 * @author Dave Longley
 */
public class PKeyCryptor
{
   /**
    * The cryptor for the private key.
    */
   protected Cryptor mPkCryptor;

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
   protected Cryptor mPwCryptor;

   /**
    * The encrypted private key password.
    */
   protected String mPkPassword;
   
   /**
    * The name of the private key file.
    */
   protected String mPkFilename;
   
   /**
    * The error associated with loading a private key, etc, if any.
    */
   protected String mError;
   
   /**
    * Creates a new private key cryptor. The private key filename and
    * password for the private key must be set before trying to load
    * a private key.
    */
   public PKeyCryptor()
   {
      mPkCryptor = null;
      mPrivateKey = null;
      mPkFilename = null;
      mPkPassword = null;
      mError = "";
   }   
   
   /**
    * Creates a new private key cryptor.
    * 
    * @param pkeyFilename the name of the file to store the private key in.
    * @param password the password to store the file with.
    */
   public PKeyCryptor(String pkeyFilename, String password)
   {
      mPkCryptor = null;
      mPrivateKey = null;
      mPkFilename = pkeyFilename;
      mError = "";
      
      storePkPasswordInMemory(password);
   }
   
   /**
    * Convenience method. Encrypts and stores the private key password
    * in memory.
    * 
    * @param password the plain-text password to store.
    * @return true if successfully stored, false if not.
    */
   protected boolean storePkPasswordInMemory(String password)
   {
      boolean rval = false;
      
      // get a new password cryptor
      mPwCryptor = new Cryptor();
      
      // store the password
      mPkPassword = mPwCryptor.encrypt(password);
      
      if(mPkPassword != null)
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
   protected String getPkPassword()
   {
      String password = null;
      
      if(mPwCryptor != null)
      {
         password = mPwCryptor.decrypt(mPkPassword);
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
   protected boolean storePkInMemory(KeyManager km) 
   {
      boolean rval = false;
      
      // store the encrypted private key in memory
      if(km.getPrivateKey() != null)
      {
         // get a fresh cryptor
         mPkCryptor = new Cryptor();
         
         // store the encrypted private key
         byte[] encodedBytes = km.getPrivateKey().getEncoded();
         mPrivateKey = mPkCryptor.encrypt(encodedBytes);
         
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Clears the private key from memory.
    */
   protected void clearPkFromMemory()
   {
      mPrivateKey = null;
   }
   
   /**
    * Convenience method. Gets the private key as an encoded array of bytes.
    *
    * @return the encoded private key bytes or null.
    */
   protected byte[] getPkEncodedBytes()
   {
      byte[] encodedKey = null;
      
      if(mPrivateKey == null)
      {
         String password = getPkPassword();
         if(password != null)
         {
            // load key from disk
            KeyManager km = new KeyManager();
            if(km.loadPrivateKey(mPkFilename, password))
            {
               // store the private key in memory
               storePkInMemory(km);
            }
            else
            {
               setError(km.getError());
               
               LoggerManager.debug("dbcommon", 
               "PKeyCryptor: ERROR - invalid password!");
            }
         }
         else
         {
            LoggerManager.debug("dbcommon", 
            "PKeyCryptor: ERROR - cannot use null password!");
         }
      }

      // make sure private key has been stored
      if(mPrivateKey != null)
      {
         // decrypt the private key
         encodedKey = mPkCryptor.decrypt(mPrivateKey);
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
    * in memory.
    * 
    * @return true if successful, false if not.
    */
   public boolean generateKeys()
   {
      return generateKeys(getPrivateKeyFilename(), getPkPassword());
   }
   
   /**
    * Generates a new set of public and private keys using the private key
    * filename in memory. Will overwrite the old keys stored in memory.
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
    * the old keys stored in memory.
    * 
    * @param pkeyFilename the private key filename to use.
    * @param password the password to use.
    * @return true if successful, false if not.
    */
   public boolean generateKeys(String pkeyFilename, String password)
   {
      boolean rval = false;
      
      if(password != null)
      {
         // update private key file name;
         mPkFilename = pkeyFilename;
         
         // create a key manager and generate a pair of public/private keys
         KeyManager km = new KeyManager();
         if(km.generateKeyPair())
         {
            // store the new password
            if(storePkPasswordInMemory(password))
            {
               // get the private key password
               password = getPkPassword();
               if(password != null)
               {
                  // store private key on disk
                  if(km.storePrivateKey(mPkFilename, password))
                  {
                     // store the keys in memory
                     if(storePkInMemory(km))
                     {
                        // store the public key in memory
                        mPublicKey = km.getPublicKeyString();
                     
                        rval = true;
                     }
                     else
                     {
                        LoggerManager.debug("dbcommon", 
                        "PKeyCryptor: ERROR - could not store keys!");
                     }
                  }
                  else
                  {
                     LoggerManager.debug("dbcommon", 
                     "PKeyCryptor: ERROR - could not store key file!");
                  }
               }
               else
               {
                  LoggerManager.debug("dbcommon", 
                  "PKeyCryptor: ERROR - could not decrypt password!");
               }
            }
            else
            {
               LoggerManager.debug("dbcommon", 
               "PKeyCryptor: ERROR - could not store encrypted password!");
            }
         }
         else
         {
            LoggerManager.debug("dbcommon", 
            "PKeyCryptor: ERROR - could not generate keys!");
         }
      }
      else
      {
         LoggerManager.debug("dbcommon", 
         "PKeyCryptor: ERROR - cannot generate keys with null password!");
      }

      return rval;
   }
   
   /**
    * Retrieves the encrypted private key password from memory.
    * 
    * @return the encryped private key password or null.
    */
   public String getEncryptedPkPassword()
   {
      return mPkPassword;
   }
   
   /**
    * Decrypts the encrypted in-memory private key password and returns
    * it in plain-text.
    * 
    * @return the plain-text password.
    */
   public String getPlainTextPkPassword()
   {
      return getPkPassword();
   }

   /**
    * Gets the private key as java.security.PrivateKey object.
    *
    * @return the private key or null.
    */
   public PrivateKey getPrivateKey()
   {
      PrivateKey pkey = null;
      
      byte[] encodedKey = getPkEncodedBytes();
      if(encodedKey != null)
      {
         // decode the private key
         pkey = KeyManager.decodePrivateKey(encodedKey);
      }
      else
      {
         LoggerManager.debug("dbcommon", 
         "PKeyCryptor: ERROR - could not get encoded key!");
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
      
      byte[] encodedKey = getPkEncodedBytes();
      if(encodedKey != null)
      {
         // encode the private key as a string
         pkey = KeyManager.encodeKey(encodedKey);
      }
      else
      {
         LoggerManager.debug("dbcommon", 
         "PKeyCryptor: ERROR - could not get encoded key!");
      }
      
      return pkey;
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
      return verify(getPkPassword());
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
    * @param pkeyFilename the private key filename.
    * @param password the password to set and verify.
    * @return true if verified, false if not. 
    */
   public boolean verify(String pkeyFilename, String password)
   {
      boolean rval = false;
      
      if(setPassword(password))
      {
         setPrivateKeyFilename(pkeyFilename);
         
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
      clearPkFromMemory();
      
      return storePkPasswordInMemory(password);
   }
   
   /**
    * Sets the private key filename.
    * 
    * @param pkeyFilename the name of the private key file.
    */
   public void setPrivateKeyFilename(String pkeyFilename)
   {
      clearPkFromMemory();
      
      mPkFilename = pkeyFilename;
   }
   
   /**
    * Gets the previously set private key filename.
    * 
    * @return the private key filename.
    */
   public String getPrivateKeyFilename()
   {
      return mPkFilename;
   }
   
   /**
    * Clears the private key password and private key from memory.
    */
   public void clear()
   {
      clearPkFromMemory();
      mPkPassword = null;
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
}
