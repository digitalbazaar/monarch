/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// A convenient private key cryptor class. Used to generate and store
   /// encrypted private keys on disk and in memory.
   /// </summary>
   ///
   /// <author>Dave Longley</author>
   public class PKeyCryptor
   {
      /// <summary>
      /// The cryptor for the private key.
      /// </summary>
      protected Cryptor mPkCryptor;

      /// <summary>
      /// The encrypted private key.
      /// </summary>
      protected byte[] mPrivateKey = null;
      
      /// <summary>
      /// The plain-text (Base64-X.509-encoded) public key.
      /// </summary>
      protected string mPublicKey;

      /// <summary>
      /// The cryptor for the private key password.
      /// </summary>
      protected Cryptor mPwCryptor;

      /// <summary>
      /// The encrypted private key password.
      /// </summary>
      protected string mPkPassword;
      
      /// <summary>
      /// The name of the private key file.
      /// </summary>
      protected string mPkFilename;
      
      /// <summary>
      /// The error associated with loading a private key, etc, if any.
      /// </summary>
      protected string mError;
      
      /// <summary>
      /// Creates a new private key cryptor. The private key filename and
      /// password for the private key must be set before trying to load
      /// a private key.
      /// </summary>
      public PKeyCryptor()
      {
         mPkCryptor = null;
         mPrivateKey = null;
         mPkFilename = null;
         mPkPassword = null;
         mError = "";
      }   
      
      /// <summary>
      /// Creates a new private key cryptor.
      /// </summary>
      /// 
      /// <param name="pkeyFilename">the name of the file to store the
      /// private key in.</param>
      /// <param name="password">the password to store the file with.</param>
      public PKeyCryptor(string pkeyFilename, string password)
      {
         mPkCryptor = null;
         mPrivateKey = null;
         mPkFilename = pkeyFilename;
         mError = "";
         
         StorePkPasswordInMemory(password);
      }
      
      /// <summary>
      /// Convenience method. Encrypts and stores the private key password
      /// in memory.
      /// </summary>
      /// 
      /// <param name="password">the plain-text password to store.</param>
      /// <returns>true if successfully stored, false if not.</returns>
      protected virtual bool StorePkPasswordInMemory(string password)
      {
         bool rval = false;
         
         // get a new password cryptor
         mPwCryptor = new Cryptor();
         
         // store the password
         mPkPassword = mPwCryptor.Encrypt(password);
         
         if(mPkPassword != null)
         {
            rval = true;
         }
         
         return rval;
      }
      
      /// <summary>
      /// Convenience property. Retrieves the encrypted private key password
      /// from memory and decrypts it into plain-text.
      /// </summary>
      /// 
      /// <returns>the decrypted plain-text password or null if there was
      /// an error.</returns>
      protected virtual string Password
      {
         get
         {
            string password = null;
         
            if(mPwCryptor != null)
            {
               password = mPwCryptor.Decrypt(mPkPassword);
            }
         
            return password;
         }
      }
      
      /// <summary>
      /// Convenience method. Takes a key manager and stores its private key
      /// in an encrypted string.
      /// </summary>
      /// 
      /// <param name="km">the key manager to get the private key from.</param>
      /// <returns>true if successfully stored, false if not.</returns>
      protected virtual bool StorePkInMemory(KeyManager km) 
      {
         bool rval = false;
         
         // store the encrypted private key in memory
         if(km.PrivateKey != null)
         {
            // get a fresh cryptor
            mPkCryptor = new Cryptor();
            
            // store the encrypted private key
            byte[] encodedBytes = km.PrivateKey.Bytes;
            mPrivateKey = mPkCryptor.Encrypt(encodedBytes);
            
            rval = true;
         }
         
         return rval;
      }
      
      /// <summary>
      /// Clears the private key from memory.
      /// </summary>
      protected virtual void ClearPkFromMemory()
      {
         mPrivateKey = null;
      }
      
      /// <summary>
      /// Loads the private key into memory, encrypted.
      /// </summary>
      /// 
      /// <returns>true if successfully loaded, false if not.</returns>
      protected virtual bool LoadPrivateKeyIntoMemory()
      {
         bool rval = false;
         
         // store password locally so it only decrypts once here
         string password = Password;
         if(password != null)
         {
            // load key from disk
            KeyManager km = new KeyManager();
            if(km.LoadPrivateKey(mPkFilename, password))
            {
               // store the private key in memory
               StorePkInMemory(km);
               rval = true;
            }
            else
            {
               Error = km.Error;
               Logger.Debug("PKeyCryptor: ERROR - invalid password!");
            }
         }
         else
         {
            Logger.Debug("PKeyCryptor: ERROR - cannot use null password!");
         }
         
         return rval;
      }
      
      /// <summary>
      /// Convenience method. Gets the private key as an encoded array of bytes.
      /// </summary>
      ///
      /// <returns>the encoded private key bytes or null.</returns>
      protected virtual byte[] EncodedPrivateKey
      {
         get
         {
            byte[] encodedKey = null;
         
            if(mPrivateKey == null)
            {
               LoadPrivateKeyIntoMemory();
            }

            // make sure private key has been stored
            if(mPrivateKey != null)
            {
               // decrypt the private key
               encodedKey = mPkCryptor.Decrypt(mPrivateKey);
            }
         
            return encodedKey;
         }
      }
      
      /// <summary>
      /// Generates a new set of public and private keys using the
      /// password and keyfile in memory. Will overwrite the old keys stored
      /// in memory.
      /// </summary>
      /// 
      /// <returns>true if successful, false if not.</returns>
      public virtual bool GenerateKeys()
      {
         return GenerateKeys(Password);
      }
      
      /// <summary>
      /// Generates a new set of public and private keys. Will overwrite
      /// the old keys stored in memory.
      /// </summary>
      /// 
      /// <param name="password">the password to use.</param>
      /// <returns>true if successful, false if not.</returns>
      public virtual bool GenerateKeys(string password)
      {
         bool rval = false;
         
         if(password != null)
         {
            // store the new password
            if(StorePkPasswordInMemory(password))
            {
               // create a key manager and generate a pair of public/private keys
               KeyManager km = new KeyManager();
               if(km.GenerateKeyPair())
               {
                  // get the private key password
                  password = Password;
                  if(password != null)
                  {
                     // store the keys in memory
                     if(StorePkInMemory(km))
                     {
                        // store the public key in memory
                        mPublicKey = km.PublicKeyString;
                        rval = true;
                     }
                     else
                     {
                        Logger.Debug(
                        "PKeyCryptor: ERROR - could not store keys!");
                     }
                  }
                  else
                  {
                     Logger.Debug( 
                     "PKeyCryptor: ERROR - could not decrypt password!");
                  }
               }
               else
               {
                  Logger.Debug( 
                  "PKeyCryptor: ERROR - could not store encrypted password!");
               }
            }
            else
            {
               Logger.Debug("PKeyCryptor: ERROR - could not generate keys!");
            }
         }
         else
         {
            Logger.Debug( 
            "PKeyCryptor: ERROR - cannot generate keys with null password!");
         }

         return rval;
      }
      
      /// <summary>
      /// Stores the internal private key on disk.
      /// </summary>
      /// 
      /// <param name="filename">the name of the key file to save to.</param>
      public virtual bool StorePrivateKey(string filename)
      {
         bool rval = false;
         
         // update private key file name
         mPkFilename = filename;
         
         // store private key on disk
         if(KeyManager.StorePrivateKey(PrivateKey, mPkFilename, Password))
         {
            rval = true;
         }
         else
         {
            Logger.Debug("PKeyCryptor: ERROR - could not store key file!");
         }
         
         return rval;
      }
      
      /// <summary>
      /// Retrieves the encrypted private key password from memory.
      /// </summary>
      /// 
      /// <returns>the encryped private key password or null.</returns>
      public virtual string EncryptedPassword
      {
         get
         {
            return mPkPassword;
         }
      }
      
      /// <summary>
      /// Decrypts the encrypted in-memory private key password and returns
      /// it in plain-text.
      /// </summary>
      /// 
      /// <returns>the plain-text password.</returns>
      public virtual string PlainTextPassword
      {
         get
         {
            return Password;
         }
      }

      /// <summary>
      /// Gets the private key as a PKCS8PrivateKey object.
      /// </summary>
      ///
      /// <returns>the private key or null.</returns>
      public virtual PKCS8PrivateKey PrivateKey
      {
         get
         {
            PKCS8PrivateKey pkey = null;
         
            byte[] encodedKey = EncodedPrivateKey;
            if(encodedKey != null)
            {
               pkey = new PKCS8PrivateKey(encodedKey);
            }
            else
            {
               Logger.Debug("PKeyCryptor: ERROR - could not get encoded key!");
            }
         
            return pkey;
         }
      }
      
      /// <summary>
      /// Gets the private key as a Base64-PKCS#8 string.
      /// </summary>
      ///
      /// <returns>the private key or null.</returns>
      public virtual string PrivateKeyString
      {
         get
         {
            string pkey = null;
         
            byte[] encodedKey = EncodedPrivateKey;
            if(encodedKey != null)
            {
               // encode the private key as a string
               pkey = Convert.ToBase64String(encodedKey);
            }
            else
            {
               Logger.Debug("PKeyCryptor: ERROR - could not get encoded key!");
            }
         
            return pkey;
         }
      }
      
      /// <summary>
      /// Gets the public key as a Base64-X.509 string.
      /// </summary>
      ///
      /// <returns>the public key or null.</returns>
      public virtual string PublicKeyString
      {
         get
         {
            return mPublicKey;
         }
      }
      
      /// <summary>
      /// Tries to verify that the password stored in memory unlocks the
      /// private key stored in the private key file.
      /// </summary>
      /// 
      /// <returns>true if verified, false if not.</returns>
      public virtual bool Verify()
      {
         return Verify(Password);
      }
      
      /// <summary>
      /// Sets the password and then tries to verify that the password
      /// unlocks the private key stored in the private key file.
      /// </summary>
      /// 
      /// <param name="password">the password to set and verify.</param>
      /// <returns></returns> true if verified, false if not. 
      public virtual bool Verify(string password)
      {
         return Verify(PrivateKeyFilename, password);
      }
      
      /// <summary>
      /// Sets the private key filename and the password and then tries to
      /// verify that the password unlocks the private key stored in the
      /// private key file.
      /// </summary>
      /// 
      /// <param name="pkeyFilename">the private key filename.</param>
      /// <param name="password">the password to set and verify.</param>
      /// <returns>true if verified, false if not.</returns> 
      public virtual bool Verify(string pkeyFilename, string password)
      {
         bool rval = false;
         
         if(SetPassword(password))
         {
            PrivateKeyFilename = pkeyFilename;
            if(PrivateKey != null)
            {
               rval = true;
            }
         }
         
         return rval;
      }
      
      /// <summary>
      /// Sets the private key password that is stored in memory. This does
      /// not update the private key file.
      /// </summary>
      /// 
      /// <param name="password">the plain-text password.</param>
      /// <returns>true if successful, false if not.</returns>
      public virtual bool SetPassword(string password)
      {
         ClearPkFromMemory();
         return StorePkPasswordInMemory(password);
      }
      
      /// <summary>
      /// Gets/Sets the private key filename.
      /// </summary>
      public virtual string PrivateKeyFilename
      {
         get
         {
            return mPkFilename;
         }
         
         set
         {
            ClearPkFromMemory();
            mPkFilename = value;
         }
      }
      
      /// <summary>
      /// Clears the private key password and private key from memory.
      /// </summary>
      public virtual void Clear()
      {
         ClearPkFromMemory();
         mPkPassword = null;
      }
      
      /// <summary>
      /// Gets/Sets the error that was associated with loading or generating
      /// a private key, etc, if there was any.
      /// </summary>
      /// 
      /// <returns>the error associated with loading a private key, etc,
      /// if any.</returns>
      public virtual string Error
      {
         get
         {
            return mError;
         }
         
         set
         {
            mError = value;
         }
      }
      
      /// <summary>
      /// Gets the logger.
      /// </summary>
      /// 
      /// <returns>the logger.</returns>
      public virtual Logger Logger
      {
         get
         {
            return LoggerManager.GetLogger("dbcommon");
         }
      }
   }
}
