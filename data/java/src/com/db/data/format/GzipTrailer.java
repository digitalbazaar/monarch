/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

import java.io.ByteArrayInputStream;
import java.io.IOException;

/**
 * An GzipTrailer is a trailer for gzip-formatted data. This is the trailer
 * for one gzip "member".
 * 
 * It has the following format:
 * 
 * | bytes 0-3 | bytes 4-7 | 
 * |   CRC32   |   ISIZE   |
 * 
 * CRC32 (CRC-32) = The Cyclic Redundancy Check value for the
 *                  uncompressed data.
 * ISIZE (Input SIZE) = The original size of the uncompressed data modulo 2^32.
 *  
 * Gzip format information from: http://www.gzip.org/zlib/rfc-gzip.html 
 * 
 * @author Dave Longley
 */
public class GzipTrailer
{
   /**
    * The CRC-32 value for this trailer.
    */
   protected long mCrc32Value;
   
   /**
    * The ISize for this trailer.
    */
   protected long mISize;
   
   /**
    * Creates a new GzipTrailer.
    */
   public GzipTrailer()
   {
   }
   
   /**
    * Tries to convert this trailer from an array of bytes.
    * 
    * @param buffer the array of bytes to convert from.
    * @param offset the offset in the array of bytes to start converting from.
    * @param length the number of bytes in the array to convert from.
    * 
    * @return the number of extra bytes required to convert this trailer from
    *         the passed array of bytes, 0 if no more bytes are required,
    *         a positive number if more bytes are required, a negative 
    *         number if fewer bytes are required.
    */
   public int convertFromBytes(byte[] buffer, int offset, int length)
   {
      int rval = 8;
      
      // make sure there are at least 8 bytes available -- this is
      // the trailer size
      if(length < 8)
      {
         rval = 8 - length;
      }
      else
      {
         try
         {
            // get a byte array input stream
            ByteArrayInputStream bais = new ByteArrayInputStream(
               buffer, offset, length);
            
            // read CRC-32 value as an unsigned int
            setCrc32Value(UnsignedBinaryIO.readUnsignedInt(bais));
            
            // read the ISIZE as an unsigned int
            setISize(UnsignedBinaryIO.readUnsignedInt(bais));
            
            // close the byte array input stream
            bais.close();
            
            // no bytes required
            rval = 0;
         }
         catch(IOException ignore)
         {
            // ignore any IO exception, return that more bytes are needed
         }
      }
      
      return rval;
   }
   
   /**
    * Converts this trailer to a byte array.
    * 
    * @return the byte array.
    */
   public byte[] convertToBytes()
   {
      byte[] rval = new byte[8];
      
      // write the CRC-32 value
      UnsignedBinaryIO.writeUnsignedInt(getCrc32Value(), rval, 0);
      
      // write the ISIZE
      UnsignedBinaryIO.writeUnsignedInt(getISize(), rval, 4);
      
      return rval;
   }
   
   /**
    * Sets the CRC-32 value for this trailer.
    * 
    * @param value the CRC-32 value for this trailer.
    */
   public void setCrc32Value(long value)
   {
      mCrc32Value = value;
   }
   
   /**
    * Gets the CRC-32 value of this trailer.
    * 
    * @return the CRC-32 value of this trailer.
    */
   public long getCrc32Value()
   {
      return mCrc32Value;
   }
   
   /**
    * Sets the ISIZE for this trailer.
    * 
    * @param iSize the ISIZE for this trailer.
    */
   public void setISize(long iSize)
   {
      mISize = iSize;
   }
   
   /**
    * Gets the ISIZE of this trailer.
    *
    * @return the ISIZE of this trailer.
    */
   public long getISize()
   {
      return mISize;
   }
}
