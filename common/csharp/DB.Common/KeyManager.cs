/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.IO;
using System.Security;
using System.Security.Cryptography;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// A class that provides management code for private, public,
   /// and secret keys.
   /// </summary>
   ///
   /// <author>Dave Longley</author>
   public class KeyManager
   {
      /// <summary>
      /// The last generated private key.
      /// </summary>
      protected PKCS8PrivateKey mPrivateKey;
      
      /// <summary>
      /// The last generated public key.
      /// </summary>
      protected X509PublicKey mPublicKey;
      
      /// <summary>
      /// The last error that occurred.
      /// </summary>
      protected string mError;
      
      /// <summary>
      /// Creates a KeyManager. For every new set of keys, a new
      /// KeyManager should be instantiated.
      /// </summary>
      public KeyManager()
      {
         mPrivateKey = null;
         mPublicKey = null;
         
         mError = "";
      }

      /// <summary>
      /// Generates a pair of keys, one public, one private and stores
      /// them internally. The keys are generated using the DSA algorithm.
      /// They can be obtained by using the appropriate property.
      /// </summary>
      ///
      /// <returns>true if successful, false if not.</returns>
      public virtual bool GenerateKeyPair()
      {
         bool rval = false;
         
         try
         {
            // uses digital signature algorithm (DSA)
            DSA dsa = new DSACryptoServiceProvider();
            DSAParameters dsap = dsa.ExportParameters(true);
            
            // store the private key using PKCS8
            mPrivateKey = new PKCS8PrivateKey(dsap);
            
            // store the public key using X509
            mPublicKey = new X509PublicKey(dsap);

            rval = true;
         }
         catch(Exception e)
         {
            Logger.Error("could not generate key pair, exception thrown!");
            Logger.Debug(e.ToString());
         }

         return rval;
      }
      
      /// <summary>
      /// Stores the passed private key in a file with the passed filename
      /// and password.
      /// </summary>
      ///
      /// <param name="key">the PKCS#8 private key to store.</param>
      /// <param name="filename">the name of the file to store the
      /// key in.</param>
      /// <param name="password">the password to lock the file with.</param>
      /// <returns>true if successful, false if not.</returns>
      public static bool StorePrivateKey(PKCS8PrivateKey key,
                                         string filename, string password)
      {
         bool rval = false;
         
         if(key != null)
         {
            rval = StorePrivateKey(key.Bytes, filename, password);
         }
         
         return rval;
      }

      /// <summary>
      /// Stores the passed private key in a file with the passed filename
      /// and password.
      /// </summary>
      ///
      /// <param name="key">the private key (PKCS8-encoded) to
      /// store.</param>
      /// <param name="filename">the name of the file to store the
      /// key in.</param>
      /// <param name="password">the password to lock the file with.</param>
      /// <returns>true if successful, false if not.</returns>
      public static bool StorePrivateKey(byte[] key,
                                         string filename, string password)
      {
         bool rval = false;
         
         if(key != null)
         {
            try
            {
               // encrypt the key with the passed password
               byte[] bytes = Cryptor.Encrypt(key, password);
            
               // delete existing file
               File.Delete(filename);
            
               // create private key file, write encrypted bytes
               FileStream fs = new FileStream(filename, FileMode.CreateNew);
               fs.Write(bytes, 0, bytes.Length);
               fs.Close();
               
               rval = true;
            }
            catch(Exception e)
            {
               Logger.Error("Unable to write private key to file," +
                            "filename=" + filename);
               Logger.Debug(e.ToString());
            }
         }

         return rval;
      }
      
      /// <summary>
      /// Stores the passed private key in a file with the passed filename
      /// and password.
      /// </summary>
      ///
      /// <param name="key">the private key (Base64-PKCS8-encoded) to
      /// store.</param>
      /// <param name="filename">the name of the file to store the
      /// key in.</param>
      /// <param name="password">the password to lock the file with.</param>
      /// <returns>true if successful, false if not.</returns>
      public static bool StorePrivateKey(string key,
                                         string filename, string password)
      {
         bool rval = false;
         
         rval = StorePrivateKey(
            Convert.FromBase64String(key), filename, password);

         return rval;
      }      

      /// <summary>
      /// Stores a private key in a file with the passed filename
      /// and password.
      /// </summary>
      ///
      /// <param name="filename">the name of the file to store the
      /// key in.</param>
      /// <param name="password">the password to lock the file with.</param>
      /// <returns>true if successful, false if not.</returns>
      public virtual bool StorePrivateKey(string filename, string password)
      {
         return StorePrivateKey(mPrivateKey, filename, password);
      }
      
      /// <summary>
      /// Stores the passed public key in a file with the passed filename.
      /// </summary>
      ///
      /// <param name="key">the X.509 public key to store.</param>
      /// <param name="filename">the name of the file to store the
      /// key in.</param>
      /// <returns>true if successful, false if not.</returns>
      public static bool StorePublicKey(X509PublicKey key, string filename)
      {
         bool rval = false;
         
         if(key != null)
         {
            rval = StorePublicKey(key.Bytes, filename); 
         }
         
         return rval;
      }      
      
      /// <summary>
      /// Stores the passed public key in a file with the passed filename.
      /// </summary>
      ///
      /// <param name="key">the public key (X509-encoded)
      /// to store.</param>
      /// <param name="filename">the name of the file to store the
      /// key in.</param>
      /// <returns>true if successful, false if not.</returns>
      public static bool StorePublicKey(byte[] key, string filename)
      {
         bool rval = false;
         
         if(key != null)
         {
            try
            {
               // delete existing file
               File.Delete(filename);
            
               // create public key file, write bytes
               FileStream fs = new FileStream(filename, FileMode.CreateNew);
               fs.Write(key, 0, key.Length);
               fs.Close();
               
               rval = true;
            }
            catch(Exception e)
            {
               Logger.Error("Unable to write public key to file," +
                            "filename=" + filename);
               Logger.Debug(e.ToString());
            }
         }
         
         return rval;
      }         

      /// <summary>
      /// Stores the passed public key in a file with the passed filename.
      /// </summary>
      ///
      /// <param name="key">the public key (Base64-X509-encoded)
      /// to store.</param>
      /// <param name="filename">the name of the file to store the
      /// key in.</param>
      /// <returns>true if successful, false if not.</returns>
      public static bool StorePublicKey(string key, string filename)
      {
         bool rval = false;
         
         rval = StorePublicKey(Convert.FromBase64String(key), filename);
         
         return rval;
      }   

      /// <summary>
      /// Stores a public key in a file with the passed filename.
      /// </summary>
      ///
      /// <param name="filename">the name of the file to store the
      /// key in.</param>
      /// <returns>true if successful, false if not.</returns>
      public virtual bool StorePublicKey(string filename)
      {
         return StorePublicKey(mPublicKey, filename);
      }
      
      /// <summary>
      /// Loads a private key from the passed bytes, that is locked with
      /// the passed password.
      /// </summary>
      ///
      /// <param name="bytes">the encrypted bytes that contain the
      /// private key.</param>
      /// <param name="password">the password to unlock the file.</param>
      /// <returns>true if successful, false if not.</returns>
      public virtual bool LoadPrivateKey(byte[] bytes, string password)
      {
         bool rval = false;
         
         try
         {
            // decrypt the key with the passed password
            bytes = Cryptor.Decrypt(bytes, password);

            // if bytes are not null, set private key
            if(bytes != null)
            {
               mPrivateKey = new PKCS8PrivateKey(bytes);
               rval = true;       
            }
            else
            {
               mError = "password-invalid";
            }
         }
         catch(Exception e)
         {
            Logger.Error("Unable to load private key.");
            mError = "key-not-found";
            Logger.Debug(e.ToString());
         }
         
         return rval;
      }

      /// <summary>
      /// Loads a private key from the file with the passed filename,
      /// that is locked with the passed password.
      /// </summary>
      ///
      /// <param name="filename">the file that contains the private key.</param>
      /// <param name="password">the password to unlock the file.</param>
      /// <returns>true if successful, false if not.</returns>
      public virtual bool LoadPrivateKey(string filename, string password)
      {
         bool rval = false;
         
         try
         {
            // open encrypted private key file, get bytes
            FileInfo fileInfo = new FileInfo(filename);
            FileStream fs = fileInfo.OpenRead();
            byte[] bytes = new byte[(int)fileInfo.Length];
            
            // read in the entire file
            int offset = 0;
            int count = (int)fileInfo.Length;
            int numBytes = 0;
            while(count > 0 && numBytes != -1)
            {
               numBytes = fs.Read(bytes, offset, count);
               offset += numBytes;
               count -= numBytes;
            }
            
            // close the stream
            fs.Close();
            
            rval = LoadPrivateKey(bytes, password);
         }
         catch(Exception e)
         {
            Logger.Error("Unable to load private key,filename= " + filename);
            mError = "key-not-found";
            Logger.Debug(e.ToString());
         }

         return rval;
      }
      
      /// <summary>
      /// Loads a public key from the file with the passed filename.
      /// </summary>
      ///
      /// <param name="filename">the file that contains the public key.</param>
      /// <returns>true if successful, false if not.</returns>
      public virtual bool LoadPublicKey(string filename)
      {
         bool rval = false;
         
         try
         {
            // open public key file, get bytes
            FileInfo fileInfo = new FileInfo(filename);
            FileStream fs = fileInfo.OpenRead();
            byte[] bytes = new byte[(int)fileInfo.Length];
            
            // read in the entire file
            int offset = 0;
            int count = (int)fileInfo.Length;
            while(count > 0)
            {
               int numBytes = fs.Read(bytes, offset, count);
               if(numBytes == 0)
               {
                  break;
               }
               
               offset += numBytes;
               count -= numBytes;
            }         
         
            // close stream
            fs.Close();
         }
         catch(Exception e)
         {
            Logger.Error("Unable to load public key,filename=" + filename);
            mError = "key-not-found";
            Logger.Debug(e.ToString());
         }

         return rval;
      }   

      /// <summary>
      /// Returns the internal private key. It is PKCS#8-encoded.
      /// </summary>
      /// 
      /// <returns>the internal private key.</returns>
      public virtual PKCS8PrivateKey PrivateKey
      {
         get
         {
            return mPrivateKey;
         }
      }
      
      /// <summary>
      /// Returns the internal private key. It is Base64-PKCS8-encoded.
      /// </summary>
      /// 
      /// <returns>the internal private key.</returns>
      public virtual string PrivateKeyString
      {
         get
         {
            return Convert.ToBase64String(mPrivateKey.Bytes);
         }
      }      
      
      /// <summary>
      /// Returns the internal public key. It is X.509-encoded.
      /// </summary>
      /// 
      /// <returns>the internal public key.</returns>
      public virtual X509PublicKey PublicKey
      {
         get
         {
            return mPublicKey;
         }
      }
      
      /// <summary>
      /// Returns the internal public key. It is Base64-X509-encoded.
      /// </summary>
      /// 
      /// <returns>the internal public key.</returns>
      public virtual string PublicKeyString
      {
         get
         {
            return Convert.ToBase64String(mPublicKey.Bytes);
         }
      }

      /// <summary>
      /// Clears the error flag.
      /// </summary>
      public virtual void ClearError()
      {
         mError = "";
      }
      
      /// <summary>
      /// Returns the last error that occurred.
      /// </summary>
      /// 
      /// <returns>the last error that occurred.</returns>
      public virtual string Error
      {
         get
         {
            return mError;
         }
      }
      
      /// <summary>
      /// Gets the logger.
      /// </summary>
      /// 
      /// <returns>the logger.</returns>
      public static Logger Logger
      {
         get
         {
            return LoggerManager.GetLogger("dbcommon");
         }
      }
   }
}
