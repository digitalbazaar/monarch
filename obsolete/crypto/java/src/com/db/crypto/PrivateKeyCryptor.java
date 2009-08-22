/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.crypto;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

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
    * The encrypted private key as an EncryptedPrivateKeyInfo ASN.1 structure
    * in DER encoded format.
    */
   protected byte[] mEncryptedPrivateKey = null;
   
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
      mEncryptedPrivateKey = null;
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
      mEncryptedPrivateKey = null;
      mError = "";
      setPrivateKeyFilename(keyFilename);
      
      storePasswordInMemory(password);
   }
   
   /**
    * Convenience method. Encrypts and stores the private key password
    * in memory.
    * 
    * @param password the plain-text password to store.
    * 
    * @return true if successfully stored, false if not.
    */
   protected boolean storePasswordInMemory(String password)
   {
      boolean rval = false;
      
      getLogger().debug(getClass(),
         "Creating cryptor for storing encrypted password...");
      
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
    * Stores the private key in memory.
    * 
    * @param encodedBytes the private key in encoded bytes.
    * @param password the password for the private key.
    * 
    * @return true if the private key was stored, false if not.
    */
   protected boolean storePrivateKeyInMemory(
      byte[] encodedBytes, String password)
   {
      boolean rval = false;
      
      // store the encrypted private key in memory
      if(encodedBytes != null)
      {
         // store the encrypted private key
         mEncryptedPrivateKey =
            Cryptor.encryptPrivateKey(encodedBytes, password);
         rval = true;
      }
      
      return rval;
   }   
   
   /**
    * Convenience method. Takes a key manager and stores its private key
    * in an encrypted string.
    * 
    * @param km the key manager to get the private key from.
    * @param password the password to store the private key with.
    * 
    * @return true if successfully stored, false if not.
    */
   protected boolean storePrivateKeyInMemory(KeyManager km, String password)
   {
      boolean rval = false;
      
      // store the encrypted private key in memory
      if(km.getPrivateKey() != null)
      {
         // store the encrypted private key
         byte[] encodedBytes = km.getPrivateKey().getEncoded();
         rval = storePrivateKeyInMemory(encodedBytes, password);
      }
      
      return rval;
   }
   
   /**
    * Clears the private key from memory.
    */
   protected void clearPrivateKeyFromMemory()
   {
      mEncryptedPrivateKey = null;
      System.gc();
   }
   
   /**
    * Loads the private key from disk and encrypts it into memory.
    * 
    * @param password the password for unlocking the key.
    * 
    * @return true if successful, false if not.
    */
   protected boolean loadPrivateKeyFromFile(String password)
   {
      boolean rval = false;
      
      // see if we can load the private key from disk
      if(getPrivateKeyFilename() != null)
      {
         // create a key manager for loading the private key
         KeyManager km = new KeyManager();
         
         boolean loaded = false;
         if(getPrivateKeyFilename().toLowerCase().endsWith(".pem"))
         {
            // load private key from PEM format
            loaded = km.loadPEMPrivateKeyFromFile(
               getPrivateKeyFilename(), password);
         }
         else
         {
            // load private key from DER format
            loaded = km.loadPrivateKeyFromFile(
               getPrivateKeyFilename(), password);
         }
         
         if(loaded)
         {
            // store private key in memory
            storePrivateKeyInMemory(km, password);
            rval = true;
         }
         else
         {
            // set error from key manager
            setError(km.getError());
            
            // this is not an error condition, the password is just wrong
            getLogger().debug(getClass(),
               "Unable to unlock private key -- invalid password!");
         }
      }
      
      return rval;
   }
   
   /**
    * Stores a private key from the given key manager on disk.
    * 
    * @param km the key manager that has the private key to store.
    * @param password the password to store the key with.
    * 
    * @return true if the private key was successfully stored, false if not.
    */
   protected boolean storePrivateKeyInFile(KeyManager km, String password)
   {
      boolean rval = false;
      
      // see if we can save the private key to disk
      if(getPrivateKeyFilename() != null)
      {
         boolean stored = false;
         if(getPrivateKeyFilename().toLowerCase().endsWith(".pem"))
         {
            // store private key in PEM format
            stored = km.storePEMPrivateKey(
               getPrivateKeyFilename(), password);
         }
         else
         {
            // store private key in DER format
            stored = km.storePrivateKey(
               getPrivateKeyFilename(), password);
         }
         
         if(stored)
         {
            // store private key in memory
            storePrivateKeyInMemory(km, password);
            rval = true;
         }
         else
         {
            // set error from key manager
            setError(km.getError());
            
            getLogger().error(getClass(), "Unable to store private key!");
         }
      }
      
      return rval;
   }
   
   /**
    * Generates a new set of public and private keys. Will overwrite
    * the old keys stored in memory. The private key will not be written to
    * disk.
    * 
    * @param km the key manager to use to generate the key pair.
    * @param algorithm the algorithm to use (i.e. "DSA" or "RSA").
    * @param password the password to use.
    * 
    * @return true if successful, false if not.
    */
   protected boolean generateKeysInMemory(
      KeyManager km, String algorithm, String password)
   {
      boolean rval = false;
      
      if(password != null)
      {
         // generate a pair of public/private keys
         if(km.generateKeyPair(algorithm))
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
                  // store the private key in memory
                  if(storePrivateKeyInMemory(km, password))
                  {
                     // store the public key in memory
                     mPublicKey = km.getPublicKeyString();
                     rval = true;
                  }
                  else
                  {
                     getLogger().error(getClass(),
                        "Could not store private key in memory!");
                  }
               }
               else
               {
                  getLogger().error(getClass(),
                     "Could not decrypt password!");
               }
            }
            else
            {
               getLogger().error(getClass(),
                  "Could not store encrypted password in memory!");
            }
         }
         else
         {
            getLogger().error(getClass(),
               "Could not generate key pair!");
         }
      }
      else
      {
         getLogger().error(getClass(),
            "Password not set, cannot generate keys!");
      }

      return rval;      
   }
   
   /**
    * Gets the decrypted private key. This method will try to load the
    * private key from disk if it isn't available in memory.
    * 
    * @param password the password to unlock the private key.
    * 
    * @return the decrypted private key or null if it cannot be loaded.
    */
   protected PrivateKey getPrivateKey(String password)
   {
      PrivateKey rval = null;
      
      // check the password
      if(password != null)
      {
         // see if there is an encrypted private key in memory
         if(mEncryptedPrivateKey != null)
         {
            byte[] decrypted = Cryptor.decryptPrivateKey(
               mEncryptedPrivateKey, password);
            
            // decode the decrypted bytes
            rval = KeyManager.decodePrivateKey(decrypted);
         }
         else
         {
            // try to load private key from disk
            if(loadPrivateKeyFromFile(password))
            {
               // run getPrivateKey() again, this time the encrypted
               // private key should be in memory
               rval = getPrivateKey();
            }
         }
      }
      else
      {
         getLogger().error(getClass(),
            "Error while retrieving private key -- password is null!");
      }
      
      return rval;
   }
   
   /**
    * Gets the encrypted private key. This method will try to load the
    * private key from disk if it isn't available in memory.
    * 
    * @param password the password to unlock the private key if necessary.
    * 
    * @return the encrypted private key or null if it cannot be loaded.
    */
   protected byte[] getEncryptedPrivateKey(String password)   
   {
      byte[] rval = null;
      
      if(mEncryptedPrivateKey == null)
      {
         // load the private key
         if(getPrivateKey() != null)
         {
            rval = mEncryptedPrivateKey;
         }
      }
      else
      {
         rval = mEncryptedPrivateKey;
      }
      
      return rval;
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
    * @param algorithm the algorithm to use (i.e. "DSA" or "RSA").
    * @return true if successful, false if not.
    */
   public boolean generateKeys(String algorithm)
   {
      return generateKeys(
         getPrivateKeyFilename(), algorithm, getDecryptedPassword());
   }
   
   /**
    * Generates a new set of public and private keys using the private key
    * filename in memory. Will overwrite the old keys stored in memory and
    * write the private key to disk.
    * 
    * @param algorithm the algorithm to use (i.e. "DSA" or "RSA").
    * @param password the password to use.
    * 
    * @return true if successful, false if not.
    */
   public boolean generateKeys(String algorithm, String password)
   {
      return generateKeys(getPrivateKeyFilename(), algorithm, password);
   }
   
   /**
    * Generates a new set of public and private keys. Will overwrite
    * the old keys stored in memory and save the private key on disk.
    * 
    * @param keyFilename the private key filename to use.
    * @param algorithm the algorithm to use (i.e. "DSA" or "RSA").
    * @param password the password to use.
    * 
    * @return true if successful, false if not.
    */
   public boolean generateKeys(
      String keyFilename, String algorithm, String password)
   {
      boolean rval = false;
      
      // create a key manager for generating keys
      KeyManager km = new KeyManager();
      
      // generate the keys in memory first
      if(generateKeysInMemory(km, algorithm, password))
      {
         // set private key file name
         setPrivateKeyFilename(keyFilename);
         
         // store private key on disk
         if(storePrivateKeyInFile(km, password))
         {
            rval = true;
         }
         else
         {
            getLogger().error(getClass(), 
               "Could not store private key on disk!");
         }
      }

      return rval;
   }
   
   /**
    * Generates a new set of public and private keys. Will overwrite
    * the old keys stored in memory. The private key will not be written to
    * disk.
    * 
    * @param algorithm the algorithm to use (i.e. "DSA" or "RSA").
    * @param password the password to use.
    * 
    * @return true if successful, false if not.
    */
   public boolean generateKeysInMemory(String algorithm, String password)
   {
      boolean rval = false;
      
      // create a key manager for generating keys
      KeyManager km = new KeyManager();
      
      // generate the keys in memory
      if(generateKeysInMemory(km, algorithm, password))
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Decrypts the encrypted in-memory private key password and returns
    * it in plain-text.
    * 
    * @return the plain-text password.
    */
   public String getPassword()
   {
      return getDecryptedPassword();
   }
   
   /**
    * Sets the private key.
    * 
    * @param pkey the private key.
    * 
    * @return true if the private key was set, false if not.
    */
   public boolean setPrivateKey(PrivateKey pkey)
   {
      boolean rval = false;
      
      // store the encrypted private key in memory
      if(pkey != null)
      {
         // store the encrypted private key
         byte[] encodedBytes = pkey.getEncoded();
         rval = storePrivateKeyInMemory(encodedBytes, getDecryptedPassword());
      }
      
      return rval;
   }
   
   /**
    * Sets the private key from the passed encrypted key (PEM formatted)
    * that is locked with the passed password.
    *
    * @param pem the encrypted key in a PEM formatted string.
    * @param password the password to unlock the file.
    * 
    * @return true if successful, false if not.
    */
   public boolean setPEMEncryptedPrivateKey(String pem, String password)
   {
      boolean rval = false;
      
      try
      {
         KeyManager km = new KeyManager();
         
         // load the PEM encrypted key
         if(km.loadPEMPrivateKey(pem, password))
         {
            // set this cryptor's password
            if(setPassword(password))
            {
               // set this cryptor's private key
               rval = setPrivateKey(km.getPrivateKey());
            }
         }
         else
         {
            // set the error to the key manager's error
            setError(km.getError());
            
            getLogger().debug(getClass(),
               "Could not unlock encrypted PEM private key.");
         }
      }
      catch(Throwable t)
      {
         getLogger().error(getClass(),
            "Unable to load encrypted PEM private key.");
         getLogger().debug(getClass(), Logger.getStackTrace(t));
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
      return getPrivateKey(getDecryptedPassword());
   }
   
   /**
    * Gets the private key as a PKCS8(DER encoded)-Base64 string.
    *
    * @return the private key or null.
    */
   public String getPrivateKeyString()
   {
      String pkey = null;
      
      PrivateKey key = getPrivateKey();
      if(key != null)
      {
         // encode the private key as a base64-encoded string
         pkey = KeyManager.base64EncodeKey(key);
      }
      
      return pkey;
   }
   
   /**
    * Gets the private key as a PEM (PKCS8-Base64 with header/footer) string.
    *
    * @return the PEM private key or null.
    */
   public String getPEMPrivateKey()
   {
      String pem = null;
      
      String key = getPrivateKeyString();
      if(key != null)
      {
         pem =
            KeyManager.PRIVATE_KEY_PEM_HEADER +
            "\n" + key + "\n" +
            KeyManager.PRIVATE_KEY_PEM_FOOTER;
      }
      
      return pem;
   }
   
   /**
    * Gets the encrypted private key in PEM format
    * (PKCS8-Base64 with header/footer) string.
    *
    * @return the PEM formatted encrypted private key or null.
    */
   public String getPEMEncryptedPrivateKey()
   {
      String pem = null;
      
      byte[] encryptedKey = getEncryptedPrivateKey(getDecryptedPassword());
      if(encryptedKey != null)
      {
         String key = KeyManager.base64EncodeKey(encryptedKey);
         pem =
            KeyManager.ENCRYPTED_PRIVATE_KEY_PEM_HEADER +
            "\n" + key + "\n" +
            KeyManager.ENCRYPTED_PRIVATE_KEY_PEM_FOOTER;
      }
      
      return pem;
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
    * Gets the public key as a PEM (X.509-Base64 with a header/footer) string.
    *
    * @return the PEM public key or null.
    */
   public String getPEMPublicKey()
   {
      String pem = null;
      
      String key = getPublicKeyString();
      if(key != null)
      {
         pem =
            KeyManager.PUBLIC_KEY_PEM_HEADER +
            "\n" + key + "\n" +
            KeyManager.PUBLIC_KEY_PEM_FOOTER;
      }
      
      return pem;
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
    * 
    * @return true if verified, false if not. 
    */
   public boolean verify(String password)
   {
      return verify(getPrivateKeyFilename(), password);
   }
   
   /**
    * Sets the private key filename and the password and then tries to
    * verify that the password unlocks the private key stored in the
    * private key file. This method will clear the current private key
    * from memory.
    * 
    * @param keyFilename the private key filename.
    * @param password the password to set and verify.
    * 
    * @return true if verified, false if not. 
    */
   public boolean verify(String keyFilename, String password)
   {
      boolean rval = false;
      
      if(setPassword(password))
      {
         // get the private key filename
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
    * not update the private key file. This method will clear the private
    * key stored in memory.
    * 
    * @param password the plain-text password.
    * 
    * @return true if successful, false if not.
    */
   public boolean setPassword(String password)
   {
      clearPrivateKeyFromMemory();
      return storePasswordInMemory(password);
   }
   
   /**
    * Sets the private key filename. This method will clear the private key
    * stored in memory.
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
    * Clears the private key from memory.
    */
   public void clearPrivateKey()
   {
      clearPrivateKeyFromMemory();   
   }
   
   /**
    * Clears the private key password and private key from memory.
    */
   public void clear()
   {
      clearPrivateKeyFromMemory();
      mPasswordCryptor = null;
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
      return LoggerManager.getLogger("dbcrypto");
   }
}
