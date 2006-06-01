/*
 * Copyright (c) 2005-2006 Digital Bazar, Inc.  All rights reserved.
 */
package com.db.data.format;

import com.db.logging.Logger;
import com.db.logging.LoggerManager;

import java.io.ByteArrayInputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;

/**
 * A PDF header. It is at least 8 bytes in size
 * 
 * The format of a PDF Header is such:
 * 
 * | bytes 1-4  | bytes 5+ | bytes ? |
 * -----------------------------------
 * |    %PDF    | -version |   eol   |
 * 
 * @author Dave Longley
 */
public class PDFHeader
{
   /**
    * The version of the pdf.
    */
   protected String mVersion;
   
   /**
    * The size of this header.
    */
   protected int mSize;

   /**
    * Whether or not this header is valid.
    */
   protected boolean mValid;
   
   /**
    * Minimum header size.
    */
   public static final int MIN_HEADER_SIZE = 8;
   
   /**
    * Constructs a PDF Header with default values.
    */
   public PDFHeader()
   {
      mVersion = "1.4";
      mSize = 0;
      mValid = true;
   }
   
   /**
    * Converts the header into a byte array.
    * 
    * @return the header into a byte array. 
    */
   public byte[] convertToBytes()
   {
      String header = "%PDF-" + getVersion() + "\r\n";
      
      return header.getBytes();
   }
   
   /**
    * Converts the header from a byte array with at least 8 bytes.
    * 
    * @param b the byte array to convert from.
    * @param offset the offset to start converting from.
    * @param length the number of valid bytes in the byte array.
    * @return true if successful, false if not.
    */
   public boolean convertFromBytes(byte[] b, int offset, int length)
   {
      boolean rval = false;
      
      // reset version
      setVersion("");

      if(b != null && length >= offset + MIN_HEADER_SIZE)
      {
         try
         {
            ByteArrayInputStream bais =
               new ByteArrayInputStream(b, offset, length - offset);
            
            InputStreamReader isr = new InputStreamReader(bais);
            BufferedReader br = new BufferedReader(isr);
            
            // read a line, should be the header
            String header = br.readLine();
            
            int index = header.indexOf("%PDF");
            if(index == 0 && header.length() > 5)
            {
               setVersion(header.substring(index + 5));
            }
            
            setSize(header.length());
         }
         catch(Throwable t)
         {
            getLogger().debug(Logger.getStackTrace(t));
         }
         
         rval = true;
      }

      setValid(rval);
      
      return rval;
   }
   
   /**
    * Sets the version of the pdf.
    * 
    * @param version the version of the pdf.
    * @return true if set, false if not.
    */
   public boolean setVersion(String version)
   {
      boolean rval = false;

      mVersion = version;
      rval = true;
      
      return rval;
   }
   
   /**
    * Gets the version of the pdf.
    * 
    * @return the version of the pdf.
    */
   public String getVersion()
   {
      return mVersion;
   }
   
   /**
    * Sets the size of this header.
    * 
    * @param size the size of this header.
    */
   public void setSize(int size)
   {
      mSize = size;
   }

   /**
    * Gets the size of this header.
    * 
    * @return the size of this header.
    */
   public int getSize()
   {
      return mSize;
   }
   
   /**
    * Returns whether or not the header is valid.
    * 
    * @return true if valid, false if not.
    */
   public boolean isValid()
   {
      return mValid;
   }
   
   /**
    * Sets whether or not the header is valid.
    * 
    * @param valid true to set to valid, false to set to invalid.
    */
   public void setValid(boolean valid)
   {
      mValid = valid;
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbdata");
   }
}
