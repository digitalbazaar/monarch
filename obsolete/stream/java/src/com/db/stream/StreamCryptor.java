/*
 * Copyright (c) 2003-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.stream;

import com.db.crypto.Cryptor;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;
import com.db.util.ByteBuffer;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * A class for encrypting and decrypting streaming data.
 * 
 * This class uses some specified encryption scheme to encrypt streaming
 * data. Once the data has been encrypted, the size of the original data
 * (the unencrypted size) is appended to the stream. This size, stored
 * as a long, takes up 8 bytes. This is required so that when the data
 * is decrypted, any extra padding can be stripped off.
 * 
 * So, at the end of the encrypted stream, 8 bytes of information will be
 * appended.
 * 
 * @author Dave Longley
 */
public abstract class StreamCryptor implements DataMutationAlgorithm
{
   /**
    * The underlying Cryptor that does encryption/decryption.
    */
   public Cryptor mCryptor;
   
   /**
    * The current crypt mode.
    */
   protected int mCryptMode;
   
   /**
    * Stores the total number of bytes that have been encrypted. 
    */
   protected long mNumBytesEncrypted;
   
   /**
    * Stores the total number of decrypted bytes. This number should
    * equal the total number of bytes encrypted once a decryption is
    * complete.
    */
   protected long mNumDecryptedBytes;
   
   /**
    * Encrypt mode option.
    */
   public static final int ENCRYPT = 0;
   
   /**
    * Decrypt mode option.
    */
   public static final int DECRYPT = 1;
   
   /**
    * Constructs a stream cryptor.
    */
   public StreamCryptor()
   {
      this(new Cryptor());
   }
   
   /**
    * Constructs a stream cryptor. The cryptor to use to do the
    * encryption and decryption must be specified.
    * 
    * @param cryptor the cryptor to use to do the encryption and decryption.
    */
   public StreamCryptor(Cryptor cryptor)
   {
      // store cryptor
      mCryptor = cryptor;
      
      // no bytes tallied yet
      mNumBytesEncrypted = 0;
      mNumDecryptedBytes = 0;
      
      // default mode is encryption
      setStreamCryptMode(StreamCryptor.ENCRYPT);
   }
   
   /**
    * Reads data from the passed source buffer, encrypts it, and writes it to
    * the passed destination buffer.
    * 
    * @param src the source ByteBuffer.
    * @param dest the destination ByteBuffer.
    * @param finish true to finish the encryption, false not to.
    */
   protected void encrypt(ByteBuffer src, ByteBuffer dest, boolean finish)
   {
      // update the total bytes encrypted
      mNumBytesEncrypted += src.getUsedSpace();
      
      // update encryption or finish encryption
      if(!finish)
      {
         // update encryption
         getCryptor().updateEncryption(
            src.getBytes(), src.getOffset(), src.getUsedSpace(), dest);
      }
      else
      {
         // finish encryption
         getCryptor().finishEncryption(
            src.getBytes(), src.getOffset(), src.getUsedSpace(), dest);
      }
      
      // clear data from source
      src.clear();
   }
   
   /**
    * Reads data from the passed source buffer, decrypts it, and writes it
    * to the passed destination buffer.
    * 
    * @param src the source ByteBuffer.
    * @param dest the destination ByteBuffer.
    * @param finish true to finish the decryption, false not to.
    * 
    * @return true if the data could be decrypted, false if not.
    */
   protected boolean decrypt(ByteBuffer src, ByteBuffer dest, boolean finish)
   {
      boolean rval = false;
      
      // store old used destination space
      int oldSpace = dest.getUsedSpace();
      
      // update decryption or finish decryption
      if(!finish)
      {
         // update decryption
         rval = getCryptor().updateDecryption(
            src.getBytes(), src.getOffset(), src.getUsedSpace(), dest);
      }
      else
      {
         // finish decryption
         rval = getCryptor().finishDecryption(
            src.getBytes(), src.getOffset(), src.getUsedSpace(), dest);
      }
      
      // clear data from source
      src.clear();
      
      // update total decrypted bytes
      mNumDecryptedBytes += dest.getUsedSpace() - oldSpace;
      
      return rval;
   }
   
   /**
    * Processes the entire passed stream of data and writes it to the
    * passed output stream. The streams will not be closed. Whether the
    * data is encrypted or decrypted will be based on the current mode of
    * this StreamCryptor.
    * 
    * @param is the input stream to read the data from.
    * @param os the stream to write the processed data to.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   protected void streamData(InputStream is, OutputStream os)
   throws IOException
   {
      // create mutator input stream
      MutatorInputStream mis = new MutatorInputStream(is, this);
      
      // process the data and write to output stream
      byte[] b = new byte[2048];
      int numBytes = -1;
      while((numBytes = mis.read(b)) != -1)
      {
         os.write(b, 0, numBytes);
      }
   }   
   
   /**
    * Resets this stream cryptor.
    */
   public void reset()
   {
      // reset cipher
      getCryptor().resetCipher();
      
      // reset bytes tallied
      mNumBytesEncrypted = 0;
      mNumDecryptedBytes = 0;
   }
   
   /**
    * Encrypts the entire passed stream of data and writes it to the
    * passed output stream. The streams will not be closed.
    * 
    * @param is the input stream to read the data from.
    * @param os the stream to write the encrypted data to.
    * 
    * @return true if successfully encrypted, false if not.
    */
   public boolean encrypt(InputStream is, OutputStream os)
   {
      boolean rval = false;
      
      // reset
      reset();
      
      // set mode to encrypt
      setStreamCryptMode(ENCRYPT);
      
      try
      {
         // stream data
         streamData(is, os);
         rval = true;
      }
      catch(Exception e)
      {
         getLogger().error(
            getClass(), "Could not encrypt stream!,exception= " + e);
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
      
      return rval;
   }
   
   /**
    * Opens the passed source file, encrypts it, and writes it to the
    * destination file.
    * 
    * @param src the file to read the data from.
    * @param dest the file to write the encrypted data to.
    * 
    * @return true if successfully encrypted, false if not.
    */
   public boolean encrypt(File src, File dest)
   {
      boolean rval = false;
      
      try
      {
         // open the file streams
         FileInputStream fis = new FileInputStream(src);
         FileOutputStream fos = new FileOutputStream(dest);
         
         // encrypt the data and write to file
         rval = encrypt(fis, fos);
         
         // close the file streams
         fis.close();
         fos.close();
         
         rval = true;
      }
      catch(Exception e)
      {
         getLogger().error(
            getClass(), "Could not encrypt file!,exception= " + e);
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
      
      return rval;
   }
   
   /**
    * Opens the passed source file, encrypts it, and writes it to the
    * destination file.
    * 
    * @param src the file to read the data from.
    * @param dest the file to write the encrypted data to.
    * 
    * @return true if successfully encrypted, false if not.
    */
   public boolean encrypt(String src, String dest)
   {
      return encrypt(new File(src), new File(dest));
   }
   
   /**
    * Decrypts the entire passed stream of data and writes it to the
    * passed output stream. The streams will not be closed.
    * 
    * @param is the input stream to read the data from.
    * @param os the stream to write the decrypted data to.
    * 
    * @return true if successfully decrypted, false if not.
    */
   public boolean decrypt(InputStream is, OutputStream os)
   {
      boolean rval = false;
      
      // reset
      reset();
      
      // set mode to decrypt
      setStreamCryptMode(DECRYPT);
      
      try
      {
         // stream data
         streamData(is, os);
         rval = true;
      }
      catch(Exception e)
      {
         getLogger().error(
            getClass(), "Could not decrypt stream!,exception= " + e);
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
      
      return rval;
   }
   
   /**
    * Opens the passed source file, decrypts it, and writes it to the
    * destination file.
    * 
    * @param src the file to read the data from.
    * @param dest the file to write the decrypted data to.
    * 
    * @return true if successfully decrypted, false if not.
    */
   public boolean decrypt(File src, File dest)
   {
      boolean rval = false;
      
      try
      {
         // open the file streams
         FileInputStream fis = new FileInputStream(src);
         FileOutputStream fos = new FileOutputStream(dest);
         
         // decrypt the data and write to file
         rval = decrypt(fis, fos);
         
         // close the file stream
         fis.close();
         fos.close();
         
         rval = true;
      }
      catch(Exception e)
      {
         getLogger().debug(getClass(), Logger.getStackTrace(e));
      }
      
      return rval;
   }
   
   /**
    * Opens the passed source file, decrypts it, and writes it to the
    * destination file.
    * 
    * @param src the file to read the data from.
    * @param dest the file to write the decrypted data to.
    * 
    * @return true if successfully decrypted, false if not.
    */
   public boolean decrypt(String src, String dest)
   {
      return decrypt(new File(src), new File(dest));
   }
   
   /**
    * Gets data out of the source ByteBuffer, mutates it in some implementation
    * specific fashion, and then puts it in the destination ByteBuffer. The
    * actual number of mutated bytes is returned, which may be zero if there
    * are not enough bytes in the source buffer to produce mutated bytes.
    * 
    * Note: The destination buffer will be resized to accommodate any mutated
    * bytes.
    * 
    * @param src the source ByteBuffer with bytes to mutate.
    * @param dest the destination ByteBuffer to write the mutated bytes to.
    * @param finish true to finish the mutation algorithm, false not to.
    * 
    * @return true if there was enough data in the source buffer to run the
    *         mutation algorithm (which may or may not produce mutated bytes).
    */
   public boolean mutateData(ByteBuffer src, ByteBuffer dest, boolean finish)
   {
      boolean rval = false;
      
      if(!src.isEmpty() || finish)
      {
         switch(getStreamCryptMode())
         {
            case ENCRYPT:
               encrypt(src, dest, finish);
               rval = true;
               break;
            case DECRYPT:
               rval = decrypt(src, dest, finish);
               break;
         }
      }
      
      return rval;
   }
   
   /**
    * Sets the cryptor used to do encryption and decryption.
    * 
    * @param cryptor the cryptor to set.
    */
   public void setCryptor(Cryptor cryptor)
   {
      mCryptor = cryptor;
   }

   /**
    * Gets the cryptor used to do encryption and decryption.
    * 
    * @return the cryptor.
    */
   public Cryptor getCryptor()
   {
      return mCryptor;
   }
   
   /**
    * Sets the current mode for this cryptor. Valid options
    * are ENCRYPT and DECRYPT.
    * 
    * @param mode the mode to set.
    */
   public void setStreamCryptMode(int mode)
   {
      mCryptMode = mode;
   }
   
   /**
    * Gets the current mode for this cryptor. Valid options
    * are ENCRYPT and DECRYPT.
    * 
    * @return the current stream crypt mode.
    */
   public int getStreamCryptMode()
   {
      return mCryptMode;
   }
   
   /**
    * Gets the total number of bytes that have been encrypted.
    * 
    * @return the total number of bytes that have been encrypted.
    */
   public long getNumBytesEncrypted()
   {
      return mNumBytesEncrypted;
   }
   
   /**
    * Gets the total number of decrypted bytes.
    * 
    * @return the total number of decrypted bytes.
    */
   public long getNumDecryptedBytes()
   {
      return mNumDecryptedBytes;
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public static Logger getLogger()
   {
      return LoggerManager.getLogger("dbstream");
   }
}
