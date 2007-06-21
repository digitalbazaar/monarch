/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.crypto;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

import java.io.File;
import java.io.FileInputStream;
import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * A class for generating hash sums.
 * 
 * TEMPCODE: All of this code will be replaced by C++ libraries, it has just
 * been refactored slightly to accommodate changes to related code for the
 * time being.
 * 
 * @author Dave Longley
 */
public class Hasher
{
   /**
    * Gets a checksum for the passed text.
    *
    * @param text the text to run the algorithm on.
    * @param algorithm the algorithm to run.
    * 
    * @return an array of bytes that represents the checksum or null.
    * 
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
    * 
    * @return an array of bytes that represents the checksum or null.
    * 
    * @throws NoSuchAlgorithmException
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
            
            byte[] buffer = new byte[2048];
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
            getLogger().error(Hasher.class,
               "could not get file checksum,algorithm=" + algorithm +
               ",exception= " + t);
            getLogger().debug(Hasher.class, Logger.getStackTrace(t));
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
    * 
    * @return the checksum in a hexidecimal formatted string or null.
    * 
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
    * 
    * @return the checksum in a hexidecimal formatted string or null.
    * 
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
    * 
    * @return an array of bytes that represents the MD5 checksum or null.
    * 
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
    * 
    * @return an array of bytes that represents the MD5 checksum or null.
    * 
    * @throws NoSuchAlgorithmException
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
    * 
    * @return the md5 checksum in a hexidecimal formatted string or null.
    * 
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
    * 
    * @return the md5 checksum in a hexidecimal formatted string or null.
    * 
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
    * 
    * @return an array of bytes that represents the SHA1 checksum or null.
    * 
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
    * 
    * @return the SHA1 checksum in a hexidecimal formatted string or null.
    * 
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
    * 
    * @return the SHA1 checksum in a hexidecimal formatted string or null.
    * 
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
    * 
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
    * Gets the logger.
    * 
    * @return the logger.
    */
   public static Logger getLogger()
   {
      return LoggerManager.getLogger("dbcrypto");
   }
}
