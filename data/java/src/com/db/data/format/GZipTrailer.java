/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

import java.io.ByteArrayInputStream;
import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;

/**
 * An GZipTrailer is a trailer for gzip-formatted data. This is the trailer
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
 * GZip format information from: http://www.gzip.org/zlib/rfc-gzip.html 
 * 
 * @author Dave Longley
 */
public class GZipTrailer
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
    * Creates a new GZipTrailer.
    */
   public GZipTrailer()
   {
   }

   /**
    * Reads unsigned byte that has the most significant bit first (left). This
    * is the default way in which bytes are read in Java.
    * 
    * The byte must be stored as an int because Java's bytes are unsigned.
    * 
    * @param is the input stream to read from.
    * 
    * @return the unsigned byte.
    * 
    * @throws IOException
    */
   protected int readUnsignedByte(InputStream is) throws IOException
   {
      // read the byte
      int b = is.read();
      
      // ensure we aren't at the end of the stream
      if(b == -1)
      {
         // throw an exception because this method should only be
         // called when there is more data to be read
         throw new EOFException();
      }
      
      return b;
   }
   
   /**
    * Reads an unsigned short that is stored in the Intel or
    * Little Endian byte order. That is, least significant byte first.
    * 
    * An unsigned short is stored in 2 bytes: 00 00
    * 
    * The short must be read as an int because Java's shorts are unsigned.
    * 
    * @param is the input stream to read from.
    * 
    * @return the unsigned short.
    * 
    * @throws IOException
    */
   protected int readUnsignedShort(InputStream is) throws IOException
   {
      int rval = 0;
      
      // read the first unsigned byte
      int b1 = readUnsignedByte(is);
      
      // read the second unsigned byte
      int b2 = readUnsignedByte(is);
      
      // shift the second byte to the left, it is more significant
      // OR it with the first byte to combine them
      rval = (b2 << 8) | b1;
      
      return rval;
   }
   
   /**
    * Reads unsigned integer (2 bytes) that is stored in the Intel or
    * Little Endian byte order. That is, least significant byte first.
    * 
    * An unsigned int is stored in 4 bytes: 00 00 00 00
    * 
    * The integer must be stored as a long because Java uses unsigned
    * integers for "int". 
    * 
    * @param is the input stream to read the integer from.
    * 
    * @return the unsigned integer.
    * 
    * @throws IOException
    */
   protected long readUnsignedInt(InputStream is) throws IOException
   {
      long rval = 0;
      
      // read the first unsigned short
      long s1 = readUnsignedShort(is);
      
      // read the second unsigned short
      long s2 = readUnsignedShort(is);
      
      // shift the second short to the left, it is more significant
      // OR it with the first byte to combine them
      rval = s2 << 16 | s1;
      
      return rval;
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
            mCrc32Value = readUnsignedInt(bais);
            
            // read the ISIZE as an unsigned int
            mISize = readUnsignedInt(bais);
            
            // close the byte array input stream
            bais.close();
         }
         catch(IOException ignore)
         {
            // ignore any IO exception, return that more bytes are needed
         }
      }
      
      return rval;
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
    * Gets the ISIZE of this trailer.
    *
    * @return the ISIZE of this trailer.
    */
   public long getISize()
   {
      return mISize;
   }
}
