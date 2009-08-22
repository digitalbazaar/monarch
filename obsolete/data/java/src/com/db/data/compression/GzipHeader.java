/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.compression;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.zip.CRC32;
import java.util.zip.CheckedInputStream;
import java.util.zip.Deflater;

import com.db.data.UnsignedBinaryIO;

/**
 * An GZipHeader is a header for gzip-formatted data. This is the header
 * for one gzip "member".
 * 
 * It has the following format:
 * 
 * | byte 0 | byte 1 | byte 2 | byte 3 | bytes 4-7 | byte 8 | byte 9 | optional 
 * |  ID1   |  ID2   |   CM   |   FLG  |   MTIME   |   XFL  |   OS   | optional
 * 
 * ID1 (IDentification 1) = 31 (0x1f)
 * ID2 (IDentification 2) = 139 (0x8b)
 * CM (Compression Method) = bits 0-7 are reserved, CM = 8 denotes "deflate"
 * FLG (FLaGs) =
 *    bit 0 FTEXT
 *    bit 1 FHCRC
 *    bit 2 FEXTRA
 *    bit 3 FNAME
 *    bit 4 FCOMMENT
 *    bit 5 reserved
 *    bit 6 reserved
 *    bit 7 reserved
 * MTIME (Modification TIME) = the time in Unix format
 *    (seconds since 00:00:00 GMT, Jan 1, 1970).
 * XFL (eXtra FLags) =
 *    when "deflate" (CM = 8) is set, then XFL are set as follows:
 *    XFL = 2 - compressor used maximum compression, slowest algorithm
 *    XFL = 4 - compressor used fastest algorithm
 * OS (Operating System) =
 *    The type of file system the compression took place on:
 *    
 *    0 - FAT filesystem (MS-DOS, OS/2, NT/Win32)
 *    1 - Amiga
 *    2 - VMS (or OpenVMS)
 *    3 - Unix
 *    4 - VM/CMS
 *    5 - Atari TOS
 *    6 - HPFS filesystem (OS/2, NT)
 *    7 - Macintosh
 *    8 - Z-System
 *    9 - CP/M
 *   10 - TOPS-20
 *   11 - NTFS filesystem (NT)
 *   12 - QDOS
 *   13 - Acorn RISCOS
 *  255 - unknown
 * (if FEXTRA flag is set)
 * XLEN (2 bytes) the length of the extra field
 *    DATA data of length XLEN
 * (if FNAME flag is set)
 * file name, zero-terminated
 * (if FCOMMENT set)
 * file comment, zero-terminated
 * (if FHCRC set)
 * CRC16 (2 bytes)
 * 
 *    
 * Currentl relevant flag information (should be updated if this
 * class see more usage):
 * 
 * If the FHCRC bit is set in the flags then a CRC16 for the header is
 * present and occurs immediately before the compressed data. It contains
 * the two least significant bytes of the CRC32 for all the bytes in the gzip
 * header up to but not including the CRC16.
 *  
 * Gzip format information from: http://www.gzip.org/zlib/rfc-gzip.html 
 * 
 * @author Dave Longley
 */
public class GzipHeader
{
   /**
    * For computing the CRC-32 of this header.
    */
   protected CRC32 mCrc32;
   
   /**
    * The length of the "extra field."
    */
   protected int mXLen;
   
   /**
    * True if the FNAME flag is set.
    */
   protected boolean mFNameFlagSet;
   
   /**
    * True if the FCOMMENT flag is set.
    */
   protected boolean mFCommentFlagSet;
   
   /**
    * True if the FHCRC flag is set.
    */
   protected boolean mFHCrcFlagSet;
   
   /**
    * The first identification byte for a gzip header.
    */
   public static final int GZIP_ID1 = 0x1f;
   
   /**
    * The second identification byte for a gzip header.
    */
   public static final int GZIP_ID2 = 0x8b;
   
   /**
    * The FTEXT flag.
    */
   public static final byte GZIP_FTEXT = 0x01;
   
   /**
    * The FHCRC flag.
    */
   public static final int GZIP_FHCRC = 0x02;
   
   /**
    * The FEXTRA flag.
    */
   public static final int GZIP_FEXTRA = 0x04;
   
   /**
    * The FNAME flag.
    */
   public static final int GZIP_FNAME = 0x08; 
   
   /**
    * The FCOMMENT flag.
    */
   public static final int GZIP_FCOMMENT = 0x16;
   
   /**
    * Creates a new empty GzipHeader.
    */
   public GzipHeader()
   {
      // create a new CRC-32 for computing the CRC-16 of this header
      mCrc32 = new CRC32();
   }
   
   /**
    * Gets the OS byte to use based on the current operating system.
    * 
    * @return the OS byte to use.
    */
   protected int getOSByte()
   {
      int rval = 0;
      
      String osName = System.getProperty("os.name");
      
      if(osName.startsWith("Windows"))
      {
         rval = 0;
      }
      else if(osName.startsWith("OS/2"))
      {
         rval = 0;
      }
      else if(osName.startsWith("Mac OS"))
      {
         rval = 7;
      }
      else
      {
         rval = 3;
      }
      
      return rval;
   }
   
   /**
    * Reads an advanced header (has several custom tags set).
    * 
    * @param is the unput stream to read from.
    * 
    * @return the number of bytes required to read the entire header.
    * 
    * @throws IOException
    */
   protected int readAdvancedHeader(InputStream is)
   throws IOException
   {
      int rval = 0;
      
      // skip the "extra field"
      is.skip(mXLen);

      // skip file name if appropriate
      if(mFNameFlagSet)
      {
         // read file name until 0 byte is found
         while(is.available() > 0 && is.read() != 0);
      }
      
      // see if data is still available
      if(is.available() > 0)
      {
         // skip file comment if appropriate
         if(mFCommentFlagSet)
         {
            // file name skipped, now skip file comment
            while(is.available() > 0 && is.read() != 0);
         }
         
         // see if data is still available
         if(is.available() > 0)
         {
            // see if a CRC-16 is used
            if(mFHCrcFlagSet)
            {
               if(is.available() >= 2)
               {
                  // get the CRC-32 value for this header
                  long crc32Value = mCrc32.getValue();

                  // read the CRC-16 value (after computing the crc-32)
                  int crc16Value = UnsignedBinaryIO.readUnsignedShort(is);
                  
                  // AND the CRC-32 value to get the least significant bytes
                  int value = (int)(crc32Value & 0xffff);
                  
                  // throw exception if the CRC-16 doesn't match
                  if(crc16Value != value)
                  {
                     throw new IOException("GZIP Header CRC-16 is invalid!");
                  }
                  
                  // return whatever is still available as extra
                  rval = -is.available();
               }
               else
               {
                  rval = 2 - is.available();
               }
            }
            else
            {
               // return whatever is still available as extra
               rval = -is.available();
            }
         }
         else
         {
            // we need at least one more byte
            rval++;
         }
      }
      else
      {
         // we need at least one more byte
         rval++;
      }
      
      return rval;
   }
   
   /**
    * Tries to convert this header from an array of bytes. Throws an
    * IOException if the passed data is not in the gzip header format.
    * 
    * @param buffer the array of bytes to convert from.
    * @param offset the offset in the array of bytes to start converting from.
    * @param length the number of bytes in the array to convert from.
    * 
    * @return the number of extra bytes required to convert this header from
    *         the passed array of bytes, 0 if no more bytes are required,
    *         a positive number if more bytes are required, a negative 
    *         number if fewer bytes are required.
    *         
    * @throws IOException
    */
   public int convertFromBytes(byte[] buffer, int offset, int length)
   throws IOException
   {
      int rval = 10;
      
      // make sure there are at least 10 bytes available -- this is
      // the minimum header size
      if(length < 10)
      {
         rval = 10 - length;
      }
      else
      {
         // get a byte array input stream
         ByteArrayInputStream bais = new ByteArrayInputStream(
            buffer, offset, length);
         
         // get a checked input stream for computing the CRC-32 of the data
         mCrc32.reset();
         CheckedInputStream cis = new CheckedInputStream(bais, mCrc32);
            
         // we know 10 bytes are available, so no need to check yet
         
         // read ID1
         int ID1 = UnsignedBinaryIO.readUnsignedByte(cis);
            
         // read ID2
         int ID2 = UnsignedBinaryIO.readUnsignedByte(cis);
         
         // ensure ID1 and ID2 are valid
         if(ID1 == GZIP_ID1 && ID2 == GZIP_ID2)
         {
            // read CM
            int CM = UnsignedBinaryIO.readUnsignedByte(cis);
            
            // ensure "deflate" flag is set (CM = 8)
            if(CM == Deflater.DEFLATED)
            {
               // for storing the required bytes
               int requiredBytes = 0;
               
               // get flags
               int FLG = UnsignedBinaryIO.readUnsignedByte(cis);
               
               // skip modification time, extra flags, and operating system
               cis.skip(6);
               
               /*
               // skip modification time
               readUnsignedInt(cis);
               
               // skip extra flags
               readUnsignedByte(cis);
               
               // skip operating system
               readUnsignedByte(cis);*/
               
               // check to see if extra flag is set
               if((FLG & GZIP_FEXTRA) == GZIP_FEXTRA) 
               {
                  // get the extra length
                  mXLen = UnsignedBinaryIO.readUnsignedShort(cis);
                  requiredBytes += mXLen;
               }
               
               if((FLG & GZIP_FNAME) == GZIP_FNAME)
               {
                  mFNameFlagSet = true;
                  
                  // add at least one byte for the file name
                  requiredBytes++;
               }
               
               if((FLG & GZIP_FCOMMENT) == GZIP_FCOMMENT)
               {
                  mFCommentFlagSet = true;
                  
                  // add at least one byte for the file comment
                  requiredBytes++;
               }
               
               if((FLG & GZIP_FHCRC) == GZIP_FHCRC)
               {
                  setFHCrcFlag(true);
                  
                  // add at least 2 bytes for the CRC-16
                  requiredBytes += 2;
               }
               
               // ensure that enough bytes are available
               if(cis.available() - requiredBytes > 0)
               {
                  // read the advanced header
                  requiredBytes = readAdvancedHeader(cis);
               }
               
               // set the requiredBytes
               rval = requiredBytes;
            }
            else
            {
               // throw an exception
               throw new IOException("Deflate compression not set!");
            }
         }
         else
         {
            // throw an exception
            throw new IOException("Data not in GZIP header format!");
         }
            
         // close the byte array input stream
         bais.close();
      }
      
      return rval;
   }
   
   /**
    * Sets the FHCRC flag.
    * 
    * @param flag true to turn on the FHCRC flag, false to shut it off.
    */
   public void setFHCrcFlag(boolean flag) 
   {
      mFHCrcFlagSet = flag;
   }
   
   /**
    * Converts this header to an array of bytes.
    * 
    * @return the array of bytes.
    */
   public byte[] convertToBytes()
   {
      byte[] rval = null;
      
      // get the size of the header
      int size = 10;
      if(mFHCrcFlagSet)
      {
         size += 2;
      }
      
      // create the buffer
      rval = new byte[size];
      
      // write the ID bytes
      UnsignedBinaryIO.writeUnsignedByte(GZIP_ID1, rval, 0);
      UnsignedBinaryIO.writeUnsignedByte(GZIP_ID2, rval, 1);
      
      // write the CM byte
      UnsignedBinaryIO.writeUnsignedByte(Deflater.DEFLATED, rval, 2);
      
      // write the flag byte
      if(mFHCrcFlagSet)
      {
         UnsignedBinaryIO.writeUnsignedByte(GZIP_FHCRC, rval, 3);
      }
      else
      {
         // no flags set
         UnsignedBinaryIO.writeUnsignedByte(0, rval, 3);
      }
      
      // get the current time as the modification time
      long time = Math.round(((double)System.currentTimeMillis()) / 1000);
      
      // write the MTIME (modification time)
      UnsignedBinaryIO.writeUnsignedInt(time, rval, 4);
      
      // write the XFL (extra flags), no extra flags
      UnsignedBinaryIO.writeUnsignedByte(0, rval, 8);
      
      // write the OS byte
      UnsignedBinaryIO.writeUnsignedByte(getOSByte(), rval, 9);
      
      // see if crc-16 is required
      if(mFHCrcFlagSet)
      {
         mCrc32.reset();
         mCrc32.update(rval, 0, 10);
         
         // get the crc-32 value
         long value = mCrc32.getValue();
         
         // get the least significant bytes for the crc-32 value
         int crc16Value = (int)(value & 0xffff);
         
         // write the crc-16
         UnsignedBinaryIO.writeUnsignedShort(crc16Value, rval, 10);
      }
      
      return rval;
   }
}
