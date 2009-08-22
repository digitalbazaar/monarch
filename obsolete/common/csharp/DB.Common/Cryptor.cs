/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.IO;
using System.Security;
using System.Security.Cryptography;
using System.Text;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// A class for encrypting and decrypting data and signing and
   /// verifying data.
   /// </summary>
   ///
   /// <author>Dave Longley</author>
   public class Cryptor
   {
      /// <summary>
      /// The symmetric key used to decrypt the encrypted data.
      /// </summary>
      protected byte[] mKey = null;

      /// <summary>
      /// The salt used in the encryption process.
      /// </summary>
      protected static readonly byte[] SALT =
      {0xBD,0x11,0x79,0x31,0x07,0x2A,0x41,0x0B};

      /// <summary>
      /// The initialization vector for DES encryption/decryption.
      /// <summary>
      protected static readonly byte[] DES_IV =
      {0xBD,0x11,0x79,0x31,0x07,0x2A,0x41,0x0B};
      
      /// <summary>
      /// Creates a Cryptor. This object is used to encrypt and
      /// decrypt data. If it is used to encrypt data that is
      /// stored in memory, then when the data is not being used
      /// it should be encrypted, and when it needs to be
      /// used it should be decrypted until it is not used again.
      /// </summary>
      public Cryptor()
      {
      }
      
      /// <summary>
      /// Gets a checksum for the passed data.
      /// </summary>
      ///
      /// <param name="data">the data to run the algorithm on.</param>
      /// <param name="algorithm">the algorithm to run.</param>
      /// <returns>an array of bytes that represents the checksum or
      /// null.</returns>
      public static byte[] GetChecksumBytes(byte[] data,
                                            HashAlgorithm algorithm)
      {
         byte[] checksum = null;
         
         if(data != null)
         {
            // obtain the checksum using the passed algorithm
            checksum = algorithm.ComputeHash(data);
         }
         
         return checksum;
      }       
      
      /// <summary>
      /// Gets a checksum for the passed text.
      /// </summary>
      ///
      /// <param name="text">the text to run the algorithm on.</param>
      /// <param name="algorithm">the algorithm to run.</param>
      /// <returns>an array of bytes that represents the checksum or
      /// null.</returns>
      public static byte[] GetChecksumBytes(string text,
                                            HashAlgorithm algorithm)
      {
         UTF8Encoding utf8 = new UTF8Encoding();
         return GetChecksumBytes(utf8.GetBytes(text), algorithm);
      }   

      /// <summary>
      /// Gets the checksum for the passed file.
      /// </summary>
      ///
      /// <param name="fileInfo">the file to run the algorithm on.</param>
      /// <param name="algorithm">the algorithm to run.</param>
      /// <returns>an array of bytes that represents the checksum or
      /// null.</returns>
      public static byte[] GetChecksumBytes(FileInfo fileInfo,
                                            HashAlgorithm algorithm)
      {
         byte[] checksum = null;
         
         if(fileInfo != null && fileInfo.Exists)
         {
            FileStream fs = null;
            
            try
            {
               // open stream
               fs = fileInfo.OpenRead();
               
               // obtain the checksum using the passed algorithm
               checksum = algorithm.ComputeHash(fs);
               
               // close stream
               fs.Close();
            }
            catch(Exception e)
            {
               Logger.Debug(e.ToString());
               Logger.Error(
                     "could not Get file checksum,algorithm=" + algorithm);
            }
            
            if(fs != null)
            {
               try
               {
                  fs.Close();
               }
               catch
               {
               }
            }
         }
         
         return checksum;
      }
      
      /// <summary>
      /// Gets the checksum for the passed data.
      /// </summary>
      ///
      /// <param name="data">the data to run the algorithm on.</param>
      /// <param name="algorithm">the algorithm to run.</param>
      /// <returns>the checksum in a hexidecimal formatted string or
      /// null.</returns>
      public static string GetChecksumString(byte[] data,
                                             HashAlgorithm algorithm)
      {
         return ConvertBytesToHex(GetChecksumBytes(data, algorithm));
      }

      /// <summary>
      /// Gets the checksum for the passed text.
      /// </summary>
      ///
      /// <param name="text">the text to run the algorithm on.</param>
      /// <param name="algorithm">the algorithm to run.</param>
      /// <returns>the checksum in a hexidecimal formatted string or
      /// null.</returns>
      public static string GetChecksumString(string text,
                                             HashAlgorithm algorithm)
      {
         UTF8Encoding utf8 = new UTF8Encoding();
         return GetChecksumString(utf8.GetBytes(text), algorithm);
      }
      
      /// <summary>
      /// Gets the checksum for the passed file.
      /// </summary>
      ///
      /// <param name="fileInfo">the file to run the algorithm on.</param>
      /// <param name="algorithm">the algorithm to run.</param>
      /// <returns>the checksum in a hexidecimal formatted string or
      /// null.</returns>
      public static string GetChecksumString(FileInfo fileInfo,
                                             HashAlgorithm algorithm)
      {
         // obtain the checksum using the passed algorithm and convert to hex
         return ConvertBytesToHex(GetChecksumBytes(fileInfo, algorithm));
      }
      
      /// <summary>
      /// Gets the MD5 checksum for the passed data.
      /// </summary>
      ///
      /// <param name="data">the data to run the MD5 algorithm on.</param>
      /// <returns>an array of bytes that represents the MD5 checksum or
      /// null.</returns>
      public static byte[] GetMD5ChecksumBytes(byte[] data)
      {
         return GetChecksumBytes(data, new MD5CryptoServiceProvider());
      }
      
      /// <summary>
      /// Gets the MD5 checksum for the passed text.
      /// </summary>
      ///
      /// <param name="text">the text to run the MD5 algorithm on.</param>
      /// <returns>an array of bytes that represents the MD5 checksum or
      /// null.</returns>
      public static byte[] GetMD5ChecksumBytes(string text)
      {
         return GetChecksumBytes(text, new MD5CryptoServiceProvider());
      }
      
      /// <summary>
      /// Gets the MD5 checksum for the passed file.
      /// </summary>
      ///
      /// <param name="fileInfo">the file to run the MD5 algorithm on.</param>
      /// <returns>an array of bytes that represents the MD5 checksum or
      /// null.</returns>
      public static byte[] GetMD5ChecksumBytes(FileInfo fileInfo)
      {
         return GetChecksumBytes(fileInfo, new MD5CryptoServiceProvider());
      }
      
      /// <summary>
      /// Gets the MD5 checksum for the passed data.
      /// </summary>
      ///
      /// <param name="data">the data to run the MD5 algorithm on.</param>
      /// <returns>the md5 checksum in a hexidecimal formatted string or
      /// null.</returns>
      public static string GetMD5ChecksumString(byte[] data)
      {
         return GetChecksumString(data, new MD5CryptoServiceProvider());
      }      
      
      /// <summary>
      /// Gets the MD5 checksum for the passed text.
      /// </summary>
      ///
      /// <param name="text">the text to run the MD5 algorithm on.</param>
      /// <returns>the md5 checksum in a hexidecimal formatted string or
      /// null.</returns>
      public static string GetMD5ChecksumString(string text)
      {
         return GetChecksumString(text, new MD5CryptoServiceProvider());
      }
      
      /// <summary>
      /// Gets the MD5 checksum for the passed file.
      /// </summary>
      ///
      /// <param name="fileInfo">the file to run the MD5 algorithm on.</param>
      /// <returns>the md5 checksum in a hexidecimal formatted string or
      /// null.</returns>
      public static string GetMD5ChecksumString(FileInfo fileInfo)
      {
         return GetChecksumString(fileInfo, new MD5CryptoServiceProvider());
      }

      /// <summary>
      /// Gets the SHA1 checksum for the passed data.
      /// </summary>
      ///
      /// <param name="data">the data to run the SHA1 algorithm on.</param>
      /// <returns>an array of bytes that represents the SHA1 checksum or
      /// null.</returns>
      public static byte[] GetSHA1ChecksumBytes(byte[] data)
      {
         return GetChecksumBytes(data, new SHA1CryptoServiceProvider());
      }      

      /// <summary>
      /// Gets the SHA1 checksum for the passed text.
      /// </summary>
      ///
      /// <param name="text">the text to run the SHA1 algorithm on.</param>
      /// <returns>an array of bytes that represents the SHA1 checksum or
      /// null.</returns>
      public static byte[] GetSHA1ChecksumBytes(string text)
      {
         return GetChecksumBytes(text, new SHA1CryptoServiceProvider());
      }
      
      /// <summary>
      /// Gets the SHA1 checksum for the passed file.
      /// </summary>
      ///
      /// <param name="fileInfo">the file to run the SHA1 algorithm on.</param>
      /// <returns>an array of bytes that represents the SHA1 checksum or
      /// null.</returns>
      public static byte[] GetSHA1ChecksumBytes(FileInfo fileInfo)
      {
         return GetChecksumBytes(fileInfo, new SHA1CryptoServiceProvider());
      }

      /// <summary>
      /// Gets the SHA1 checksum for the passed data.
      /// </summary>
      ///
      /// <param name="data">the data to run the SHA1 algorithm on.</param>
      /// <returns>the SHA1 checksum in a hexidecimal formatted string or
      /// null.</returns>
      public static string GetSHA1ChecksumString(byte[] data)
      {
         return GetChecksumString(data, new SHA1CryptoServiceProvider());
      }            
      
      /// <summary>
      /// Gets the SHA1 checksum for the passed text.
      /// </summary>
      ///
      /// <param name="text">the text to run the SHA1 algorithm on.</param>
      /// <returns>the SHA1 checksum in a hexidecimal formatted string or
      /// null.</returns>
      public static string GetSHA1ChecksumString(string text)
      {
         return GetChecksumString(text, new SHA1CryptoServiceProvider());
      }
      
      /// <summary>
      /// Gets the SHA1 checksum for the passed file.
      /// </summary>
      ///
      /// <param name="fileInfo">the file to run the SHA1 algorithm on.</param>
      /// <returns>the SHA1 checksum in a hexidecimal formatted string or
      /// null.</returns>
      public static string GetSHA1ChecksumString(FileInfo fileInfo)
      {
         return GetChecksumString(fileInfo, new SHA1CryptoServiceProvider());
      }
      
      /// <summary>
      /// Converts an array of bytes into a hexidecimal string.
      /// </summary>
      /// 
      /// <param name="bytes">the array of bytes to convert.</param>
      /// <returns>the hexidecimal string.</returns>
      public static string ConvertBytesToHex(byte[] bytes)
      {
         string s = "";
         
         if(bytes != null)
         {
            // use a string builder to build the hexidecimal string
            // make sure the buffer is large enough
            StringBuilder sb = new StringBuilder(bytes.Length * 2);
            for(int i = 0; i < bytes.Length; i++)
            {
               // interpret byte as an int chopped down to byte size
               int hex = bytes[i] & 0xff;
            
               // if its less than 10 hex, append a zero
               if(hex < 0x10)
               {
                  sb.Append('0');
               }
            
               // convert int to hexadecimal
               sb.Append(hex.ToString("x"));
            }
            
            s = sb.ToString();
         }
         
         return s;   
      }
      
      /// <summary>
      /// Signs the passed byte array with the passed private key and
      /// returns the signature.
      /// </summary>
      ///
      /// <param name="data">the data to sign.</param>
      /// <param name="key">the PKCS8 privateKey to sign with.</param>
      /// <returns>the signature if successfully signed, null if not.</returns>
      public static byte[] Sign(byte[] data, PKCS8PrivateKey privateKey)
      {
         byte[] sig = null;
         
         // make sure there is a key to sign with
         if(privateKey != null)
         {
            try
            {
               // get the parameters
               DSAParameters dsap = privateKey.DSAParameters;
               
               // get hash
               byte[] hash = GetSHA1ChecksumBytes(data);
               
               Logger.Debug("SIGN CHECKSUM: " +
                            ConvertBytesToHex(hash));
               
               // sign hash
               sig = DSASignature.CreateSignature(hash, dsap);
            }
            catch(Exception e)
            {
               Console.WriteLine(e.StackTrace);
               Logger.Debug(e.ToString());
            }
         }

         return sig;
      }

      /// <summary>
      /// Signs the passed byte array with the passed private key and
      /// returns the signature.
      /// </summary>
      ///
      /// <param name="data">the data to sign.</param>
      /// <param name="key">the PKCS8 privateKey to sign with.</param>
      /// <returns>the signature if successfully signed, null if not.</returns>
      public static byte[] Sign(byte[] data, byte[] privateKey)
      {
         byte[] rval = null;
         
         if(privateKey != null)
         {
            PKCS8PrivateKey key = new PKCS8PrivateKey(privateKey);
            rval = Sign(data, key);
         }
         
         return rval;
      }
      
      /// <summary>
      /// Signs the passed byte array with the passed private key and
      /// returns the signature.
      /// </summary>
      ///
      /// <param name="data">the data to sign.</param>
      /// <param name="key">the Base64-PKCS8 privateKey to sign with.</param>
      /// <returns>the signature if successfully signed, null if not.</returns>
      public static byte[] Sign(byte[] data, string privateKey)
      {
         return Sign(data, Convert.FromBase64String(privateKey));
      }
      
      /// <summary>
      /// Signs the passed string with the passed private key and
      /// returns the signature.
      /// </summary>
      ///
      /// <param name="text">the string to sign.</param>
      /// <param name="key">the PKCS8 privateKey to sign with.</param>
      /// <returns>the signature if successfully signed, null if not.</returns>
      public static byte[] Sign(string text, PKCS8PrivateKey privateKey)
      {
         UTF8Encoding utf8 = new UTF8Encoding();
         return Sign(utf8.GetBytes(text), privateKey);
      }      
      
      /// <summary>
      /// Signs the passed string with the passed private key and
      /// returns the signature.
      /// </summary>
      ///
      /// <param name="text">the string to sign.</param>
      /// <param name="key">the PKCS8 privateKey to sign with.</param>
      /// <returns>the signature if successfully signed, null if not.</returns>
      public static byte[] Sign(string text, byte[] privateKey)
      {
         UTF8Encoding utf8 = new UTF8Encoding();
         return Sign(utf8.GetBytes(text), privateKey);
      }

      /// <summary>
      /// Signs the passed string with the passed private key and
      /// returns the signature.
      /// </summary>
      ///
      /// <param name="text">the string to sign.</param>
      /// <param name="key">the Base64-PKCS8 privateKey to sign with.</param>
      /// <returns>the signature if successfully signed, null if not.</returns>
      public static byte[] Sign(string text, string privateKey)
      {
         UTF8Encoding utf8 = new UTF8Encoding();
         return Sign(utf8.GetBytes(text), privateKey);
      }
      
      /// <summary>
      /// Attempts to verify the signature for the passed byte array
      /// using the passed X509-encoded public key.
      /// </summary>
      ///
      /// <param name="sig">the signature to verify.</param>
      /// <param name="data">the data the signature is for.</param>
      /// <param name="publicKey">the public key to verify the
      /// signature.</param>
      /// <returns>true if verified, false if not.</returns>
      public static bool Verify(byte[] sig, byte[] data,
                                X509PublicKey publicKey)
      {
         bool rval = false;

         if(publicKey != null)
         {
            try
            {
               // get the parameters
               DSAParameters dsap = publicKey.DSAParameters;
               
               // get hash
               byte[] hash = GetSHA1ChecksumBytes(data);
               
               Logger.Debug("VERIFY CHECKSUM: " +
                            ConvertBytesToHex(hash));
            
               // verify the digital signature
               rval = DSASignature.VerifySignature(hash, sig, dsap);
            }
            catch(Exception e)
            {
               Logger.Debug(e.ToString());
            }
         }
         
         return rval;
      }

      /// <summary>
      /// Attempts to verify the signature for the passed byte array
      /// using the passed X509-encoded public key.
      /// </summary>
      ///
      /// <param name="sig">the signature to verify.</param>
      /// <param name="data">the data the signature is for.</param>
      /// <param name="publicKey">the public key to verify the
      /// signature.</param>
      /// <returns>true if verified, false if not.</returns>
      public static bool Verify(byte[] sig, byte[] data, byte[] publicKey)
      {
         bool rval = false;
         
         if(publicKey != null)
         {
            X509PublicKey key = new X509PublicKey(publicKey);
            rval = Verify(sig, data, key);
         }
         
         return rval;
      }      

      /// <summary>
      /// Attempts to verify the signature for the passed byte array
      /// using the passed Base64-X509-encoded public key.
      /// </summary>
      ///
      /// <param name="sig">the signature to verify.</param>
      /// <param name="data">the data the signature is for.</param>
      /// <param name="publicKey">the public key to verify the
      /// signature.</param>
      /// <returns>true if verified, false if not.</returns>
      public static bool Verify(byte[] sig, byte[] data, string publicKey)
      {
         return Verify(sig, data, Convert.FromBase64String(publicKey));
      }
      
      /// <summary>
      /// Attempts to verify the signature for the passed string
      /// using the passed X509-encoded public key.
      /// </summary>
      ///
      /// <param name="sig">the signature to verify.</param>
      /// <param name="text">the text the signature is for.</param>
      /// <param name="publicKey">the public key to verify the
      /// signature.</param>
      /// <returns>true if verified, false if not.</returns>
      public static bool Verify(byte[] sig, string text,
                                X509PublicKey publicKey)
      {
         UTF8Encoding utf8 = new UTF8Encoding();
         return Verify(sig, utf8.GetBytes(text), publicKey);
      }
      
      /// <summary>
      /// Attempts to verify the signature for the passed string
      /// using the passed X509-encoded public key.
      /// </summary>
      ///
      /// <param name="sig">the signature to verify.</param>
      /// <param name="text">the text the signature is for.</param>
      /// <param name="publicKey">the public key to verify the
      /// signature.</param>
      /// <returns>true if verified, false if not.</returns>
      public static bool Verify(byte[] sig, string text, byte[] publicKey)
      {
         UTF8Encoding utf8 = new UTF8Encoding();
         return Verify(sig, utf8.GetBytes(text), publicKey);
      }      

      /// <summary>
      /// Attempts to verify the signature for the passed string
      /// using the passed Base64-X509-encoded public key.
      /// </summary>
      ///
      /// <param name="sig">the signature to verify.</param>
      /// <param name="text">the text the signature is for.</param>
      /// <param name="publicKey">the public key to verify the
      /// signature.</param>
      /// <returns>true if verified, false if not.</returns>
      public static bool Verify(byte[] sig, string text, string publicKey)
      {
         UTF8Encoding utf8 = new UTF8Encoding();
         return Verify(sig, utf8.GetBytes(text), publicKey);
      }

      /// <summary>
      /// Generates an internal symmetric key. The DES algorithm will be used.
      /// </summary>
      ///
      /// <returns>true if successful, false if not.</returns>
      public bool GenerateInternalSymmetricKey()
      {
         Key = GenerateSymmetricKey("DES");
         return Key != null;
      }
      
      /// <summary>
      /// Generates a symmetric key using the provided algorithm.
      /// </summary>
      ///
      /// <param name="algorithm">the algorithm to use to generate
      /// the key.</param>
      /// <returns>the generated key or null if failure.</returns>
      public static byte[] GenerateSymmetricKey(string algorithm)
      {
         Logger.Debug("generating symmetric key...");
         
         byte[] key = null;

         try
         {
            // obtain a key from the algorithm
            if(algorithm == "DES")
            {
               // generate a DES key
               DESCryptoServiceProvider des = new DESCryptoServiceProvider();
               des.GenerateKey();
               key = des.Key;
            }
            
            Logger.Debug("symmetric key generation complete.");
         }
         catch(Exception e)
         {
            Logger.Debug(e.ToString());
         }

         return key;
      }
      
      /// <summary>
      /// Generates a secret key from a password.
      /// </summary>
      ///
      /// <param name="password">the password to use to generate
      /// the key.</param>
      /// <returns>the generated password based secret key.</returns>
      public static byte[] GeneratePasswordKey(string password)
      {
         byte[] key = null;
         
         try
         {
            // get key bytes
            PasswordDeriveBytes pdb = new PasswordDeriveBytes(password, SALT);
            //key = pdb.CryptDeriveKey("DES", "SHA1", 64, DES_IV);
            key = pdb.GetBytes(8);
         }
         catch(Exception e)
         {
            Logger.Debug(e.ToString());
         }

         return key;
      }

      /// <summary>
      /// Encrypts an array of bytes of length starting at offset using
      /// the passed key.
      /// </summary>
      ///
      /// <param name="data">the array of bytes to encrypt.</param>
      /// <param name="offset">the offset to start at.</param>
      /// <param name="length">the number of bytes to encrypt.</param>
      /// <param name="key">the key to encrypt with.</param>
      /// <param name="algorithm">the algorithm for the key.</param>
      /// <returns>the encrypted data.</returns>
      public static byte[] Encrypt(byte[] data, int offset, int length,
                                   byte[] key, string algorithm)
      {
         byte[] encrypted = null;
         
         if(key != null && data != null)
         {
            try
            {
               ICryptoTransform encryptor = null;
            
               if(algorithm == "DES")
               {
                  // create an encryptor
                  DESCryptoServiceProvider des = new DESCryptoServiceProvider();
                  encryptor = des.CreateEncryptor(key, DES_IV);
               }
               
               if(encryptor != null)
               {
                  // create memory stream to write to
                  MemoryStream ms = new MemoryStream();

                  // create the crypto stream
                  CryptoStream cs =
                     new CryptoStream(ms, encryptor, CryptoStreamMode.Write);
                  
                  cs.Write(data, 0, data.Length);
                  cs.Close();
                  
                  // get encrypted bytes
                  encrypted = ms.ToArray();
                  ms.Close();
               }
            }
            catch(Exception e)
            {
               Logger.Debug(e.ToString());
            }
         }

         return encrypted;
      }
      
      /// <summary>
      /// Encrypts an array of bytes with the passed key.
      /// </summary>
      ///
      /// <param name="data">the array of bytes to encrypt.</param>
      /// <param name="key">the key to encrypt with.</param>
      /// <param name="algorithm">the algorithm for the key.</param>
      /// <returns>the encrypted data.</returns>
      public static byte[] Encrypt(byte[] data, byte[] key, string algorithm)
      {
         byte[] encrypted = null;
         
         if(data != null)
         {
            encrypted = Encrypt(data, 0, data.Length, key, algorithm);
         }
         
         return encrypted;
      }

      /// <summary>
      /// Encrypts an array of bytes of length starting at offset using
      /// an internal symmetric key.
      /// </summary>
      ///
      /// <param name="data">the array of bytes to encrypt.</param>
      /// <param name="offset">the offset to start at.</param>
      /// <param name="length">the number of bytes to encrypt.</param>
      /// <returns>the encrypted data.</returns>
      public virtual byte[] Encrypt(byte[] data, int offset, int length)
      {
         if(Key == null)
         {
            GenerateInternalSymmetricKey();
         }
         
         return Encrypt(data, offset, length, Key, "DES");
      }
      
      /// <summary>
      /// Encrypts an array of bytes using an internal symmetric key.
      /// </summary>
      ///
      /// <param name="data">the array of bytes to encrypt.</param>
      /// <returns>the encrypted data.</returns>
      public virtual byte[] Encrypt(byte[] data)
      {
         return Encrypt(data, 0, data.Length);
      }

      /// <summary>
      /// Encrypts an array of bytes using a password.
      /// </summary>
      ///
      /// <param name="data">the array of bytes to encrypt.</param>
      /// <param name="password">the password to generate the key.</param>
      /// <returns>the encrypted data.</returns>
      public static byte[] Encrypt(byte[] data, string password)
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
               byte[] key = GeneratePasswordKey(password);
               
               // do encryption
               encrypted = Encrypt(data, key, "DES");
            }
            catch(Exception e)
            {
               Logger.Debug(e.ToString());
            }
         }
         
         return encrypted;
      }
      
      /// <summary>
      /// Decrypts an array of bytes of length starting at offset using
      /// the specified key.
      /// </summary>
      ///
      /// <param name="data">the array of bytes to decrypt.</param>
      /// <param name="offset">the offset to start at.</param>
      /// <param name="length">the number of bytes to decrypt.</param>
      /// <param name="key">the key to decrypt with.</param>
      /// <param name="algorithm">the algorithm for the key.</param>
      /// <returns>the decrypted data.</returns>
      public static byte[] Decrypt(byte[] data, int offset, int length,
                                   byte[] key, string algorithm)
      {
         byte[] decrypted = null;
         
         if(key != null && data != null)
         {
            try
            {
               ICryptoTransform decryptor = null;
            
               if(algorithm == "DES")
               {
                  // create a decryptor
                  DESCryptoServiceProvider des = new DESCryptoServiceProvider();
                  decryptor = des.CreateDecryptor(key, DES_IV);
               }
               
               if(decryptor != null)
               {
                  // create memory stream to read from
                  MemoryStream ms = new MemoryStream(data);

                  // create the crypto stream
                  CryptoStream cs =
                     new CryptoStream(ms, decryptor, CryptoStreamMode.Read);
                     
                  // decrypted data should not be larger than encrypted
                  decrypted = new byte[data.Length];
                  
                  // read all encrypted data
                  int totalRead = 0;
                  int pos = 0;
                  int readSize = data.Length;
                  int numBytes = 0;
                  while((numBytes = cs.Read(decrypted, pos, readSize)) != 0)
                  {
                     pos += numBytes;
                     readSize -= numBytes;
                     totalRead += numBytes;
                  }
                  
                  // close streams
                  ms.Close();
                  cs.Close();
                  
                  // resize data according to total read
                  byte[] bytes = new byte[totalRead];
                  Array.Copy(decrypted, 0, bytes, 0, totalRead);
                  decrypted = bytes;
               }            
            }
            catch(Exception e)
            {
               decrypted = null;
               Logger.Debug(e.ToString());
            }
         }

         return decrypted;
      }
      
      /// <summary>
      /// Decrypts an encrypted array of bytes using the specified key.
      /// </summary>
      ///
      /// <param name="data">the array of bytes to decrypt.</param>
      /// <param name="key">the key to decrypt with.</param>
      /// <param name="algorithm">the algorithm for the key.</param>
      /// <returns>the decrypted data.</returns>
      public static byte[] Decrypt(byte[] data, byte[] key, string algorithm)
      {
         byte[] decrypted = null;
         
         if(data != null)
         {
            decrypted = Decrypt(data, 0, data.Length, key, algorithm);
         }
         
         return decrypted;
      }

      /// <summary>
      /// Decrypts an array of bytes of length starting at offset using
      /// an internal symmetric key.
      /// </summary>
      ///
      /// <param name="data">the array of bytes to decrypt.</param>
      /// <param name="offset">the offset to start at.</param>
      /// <param name="length">the number of bytes to decrypt.</param>
      /// <returns>the decrypted data.</returns>
      public virtual byte[] Decrypt(byte[] data, int offset, int length)
      {
         return Decrypt(data, offset, length, Key, "DES"); 
      }

      /// <summary>
      /// Decrypts an encrypted array of bytes using an internal symmetric key.
      /// </summary>
      ///
      /// <param name="data">the array of bytes to decrypt.</param>
      /// <returns> the decrypted data.
      public virtual byte[] Decrypt(byte[] data)
      {
         return Decrypt(data, 0, data.Length);
      }

      /// <summary>
      /// Decrypts an encrypted array of bytes using a password.
      /// </summary>
      ///
      /// <param name="data">the array of bytes to decrypt.</param>
      /// <param name="password">the password to generate the key.</param>
      /// <returns>the decrypted data.</returns>
      public static byte[] Decrypt(byte[] data, string password)
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
               byte[] key = GeneratePasswordKey(password);
               
               // do decryption
               decrypted = Decrypt(data, key, "DES");
            }
            catch(Exception e)
            {
               Logger.Debug(e.ToString());
            }
         }

         return decrypted;
      }

      /// <summary>
      /// Encrypts a string of text.
      /// </summary>
      ///
      /// <param name="text">the string to encrypt.</param>
      /// <returns>the encrypted string or null.</returns>
      public virtual string Encrypt(string text)
      {
         string encrypted = null;
         
         if(text != null)
         {
            // create a key if one does not exist
            if(Key == null)
            {
               GenerateInternalSymmetricKey();
            }

            if(Key != null)
            {
               try
               {
                  Logger.Debug("encrypting text");
                  
                  // UTF-8-encode and encrypt the text
                  UTF8Encoding utf8 = new UTF8Encoding();
                  byte[] data = Encrypt(utf8.GetBytes(text));

                  if(data != null)
                  {
                     // transform it back into a string using Base64-encoding
                     encrypted = Convert.ToBase64String(data);
                  }
                  
                  Logger.Debug("text encrypted");
               }
               catch(Exception e)
               {
                  Logger.Debug(e.ToString());
               }
            }
         }

         return encrypted;
      }

      /// <summary>
      /// Encrypts a string of text using a password.
      /// </summary>
      ///
      /// <param name="text">the string to encrypt.</param>
      /// <param name="password">the password to generate the key.</param>
      /// <returns>the encrypted string or null.</returns>
      public static string Encrypt(string text, string password)
      {
         string encrypted = null;
         
         if(text != null)
         {
            try
            {
               // UTF-8-encode and encrypt the text
               UTF8Encoding utf8 = new UTF8Encoding();
               byte[] data = Encrypt(utf8.GetBytes(text), password);

               if(data != null)
               {
                  // transform it back into a string using Base64-encoding
                  encrypted = Convert.ToBase64String(data);
               }
            }
            catch(Exception e)
            {
               Logger.Debug(e.ToString());
            }
         }

         return encrypted;
      }

      /// <summary>
      /// Decrypts an encrypted string of text.
      /// </summary>
      ///
      /// <param name="data">the string of data to decrypt.</param>
      /// <returns>the decrypted text or null.</returns>
      public virtual string Decrypt(string data)
      {
         string decrypted = null;
         
         if(Key != null && data != null)
         {
            try
            {
               // transform the string into an array of bytes, and decrypt
               byte[] bytes = Decrypt(Convert.FromBase64String(data));

               if(bytes != null)
               {
                  // UTF-8-encode and return the new string
                  UTF8Encoding utf8 = new UTF8Encoding();
                  decrypted = utf8.GetString(bytes);
               }
            }
            catch(Exception e)
            {
               Logger.Debug(e.ToString());
            }
         }

         return decrypted;
      }

      /// <summary>
      /// Decrypts an encrypted string of text using a password.
      /// </summary>
      ///
      /// <param name="data">the string of data to decrypt.</param>
      /// <param name="password">the password to generate the key.</param>
      /// <returns>the decrypted text or null.</returns>
      public static string Decrypt(string data, string password)
      {
         string decrypted = null;
         
         if(data != null)
         {
            try
            {
               // transform the string into an array of bytes, and decrypt
               byte[] bytes = Decrypt(Convert.FromBase64String(data), password);

               if(bytes != null)
               {
                  // UTF-8-encode and return the new string
                  UTF8Encoding utf8 = new UTF8Encoding();
                  decrypted = utf8.GetString(bytes);
               }
            }
            catch(Exception e)
            {
               Logger.Debug(e.ToString());
            }
         }

         return decrypted;      
      }
      
      /// <summary>
      /// Gets/Sets the internal symmetric key.
      /// </summary>
      public virtual byte[] Key
      {
         get
         {
            return mKey;
         }
         
         set
         {
            mKey = value;
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
