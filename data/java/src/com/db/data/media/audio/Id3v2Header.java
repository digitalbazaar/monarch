/*
 * Copyright (c) 2003-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media.audio;

/**
 * An Id3v2 header. The currently supported version.revision is 3.0. That
 * means that ID3v2.3.0 tags are supported.
 * 
 * The format of the header is such:
 * 
 * | bytes 1-3 | byte 4  |  byte 5  | byte 6  | byte 7-9 |
 * -------------------------------------------------------
 * |   "ID3"   | version | revision |  flags  |   size   |
 * 
 * In hex form:
 *  
 * 49 44 33 yy yy xx zz zz zz zz
 *
 * ID3v2/file identifier      "ID3"
 * ID3v2 version              $03 00
 * ID3v2 flags                %abc00000
 * ID3v2 size             4 * %0xxxxxxx
 * 
 * The first three bytes of the header are always "ID3" which indicates
 * that the data represents an ID3v2 tag. The next two bytes provide the
 * version (only 3.0 is currently supported -- this is the most wide-spread
 * version).
 * 
 * @author Dave Longley
 */
public class Id3v2Header
{
   /**
    * Supported version.
    */
   public static final int SUPPORTED_VERSION = 3;
   
   /**
    * Supported revision.
    */
   public static final int SUPPORTED_REVISION = 0;
   
   /**
    * The size of an ID3v2 header in bytes.
    */
   public static final int HEADER_SIZE = 10;
   
   /**
    * Unsynchronized flag (bit 7).
    */
   public static final int UNSYNCHRONIZED_BIT = 0x80;
   
   /**
    * Extended header flag (bit 6).
    */
   public static final int EXTENDEDHEADER_BIT = 0x40;
   
   /**
    * Experimental flag (bit 5).
    */
   public static final int EXPERIMENTAL_BIT = 0x20;
   
   /**
    * The version of the ID3 tag.
    */
   protected int mVersion;
   
   /**
    * The revision of the ID3 tag.
    */
   protected int mRevision;

   /**
    * Set to true if the unsynchronized flag is set.
    */
   protected boolean mUnsynchronizedFlag;
   
   /**
    * Set to true if the extended header flag is set.
    */
   protected boolean mExtendedHeaderFlag;
   
   /**
    * Set to true if the experimental flag is set.
    */
   protected boolean mExperimentalFlag;
   
   /**
    * The size of the ID3 tag. This is the sum of the size of the
    * extended header and the frames (and padding).
    */
   protected int mTagSize;
   
   /**
    * Constructs an ID3v2 header with default values.
    */
   public Id3v2Header()
   {
      mVersion = SUPPORTED_VERSION;
      mRevision = SUPPORTED_REVISION;
      mUnsynchronizedFlag = false;
      mExtendedHeaderFlag = false;
      mExperimentalFlag = false;
      mTagSize = 0;
   }
   
   /**
    * Returns a byte with its bits set according to the header flags. 
    * 
    * @return the flag byte of this header.
    */
   protected byte getFlagByte()
   {
      byte b = 0x00;
      
      if(mUnsynchronizedFlag)
      {
         b |= UNSYNCHRONIZED_BIT;
      }
      
      if(mExtendedHeaderFlag)
      {
         b |= EXTENDEDHEADER_BIT;
      }
      
      if(mExperimentalFlag)
      {
         b |= EXPERIMENTAL_BIT;
      }
      
      return b;
   }
   
   /**
    * Converts the header into a 10 byte array.
    * 
    * @param b the byte array to write the header to.
    * @param offset the offset at which to start writing.
    */
   public void convertToBytes(byte[] b, int offset)
   {
      b[offset] = 0x49; // "I"
      b[offset + 1] = 0x44; // "D"
      b[offset + 2] = 0x33; // "3"
      b[offset + 3] = (byte)(mVersion & 0xFF);
      b[offset + 4] = (byte)(mRevision & 0xFF);
      b[offset + 5] = getFlagByte();
      
      // get size
      convertIntToSynchsafeBytes(mTagSize, b, offset + 6);
   }
   
   /**
    * Converts the header from a byte array with at least 10 bytes.
    * 
    * @param b the byte array to convert from.
    * @param offset the offset to start converting from.
    * @param length the number of valid bytes in the byte array.
    * 
    * @return true if successful, false if not.
    */
   public boolean convertFromBytes(byte[] b, int offset, int length)
   {
      boolean rval = false;
      
      if(b != null && length >= HEADER_SIZE)
      {
         // check for "ID3"
         if(b[offset] == 0x49 && b[offset + 1] == 0x44 &&
            b[offset + 2] == 0x33)
         {
            int version = SUPPORTED_VERSION & 0xFF;
            int revision = SUPPORTED_REVISION & 0xFF;
            
            // check version and revision
            if(b[offset + 3] <= version && b[offset + 4] <= revision) 
            {
               mVersion = version;
               mRevision = revision;
               setFlags(b[offset + 5]);
               mTagSize = convertSynchsafeBytesToInt(b, offset + 6);
               rval = true;
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Sets the version.
    *
    * @param version the version to use. 
    */
   public void setVersion(int version)
   {
      mVersion = version;
   }
   
   /**
    * Gets the version.
    * 
    * @return the version.
    */
   public int getVersion()
   {
      return mVersion;
   }
   
   /**
    * Sets the revision of the version.
    *
    * @param revision the revision to use.
    */
   public void setRevision(int revision)
   {
      mRevision = revision;
   }
   
   /**
    * Gets the revision of the version.
    * 
    * @return the revision of the version.
    */
   public int getRevision()
   {
      return mRevision;
   }
   
   /**
    * Sets the flags based on the passed byte value.
    * 
    * @param b the flag byte to use.
    */
   public void setFlags(byte b)
   {
      mUnsynchronizedFlag = (b & UNSYNCHRONIZED_BIT) != 0;
      mExtendedHeaderFlag = (b & EXTENDEDHEADER_BIT) != 0;
      mExperimentalFlag = (b & EXPERIMENTAL_BIT) != 0;
   }
   
   /**
    * Sets the unsychronized flag.
    *
    * @param flag the boolean value to set the flag to. 
    */
   public void setUnsynchronizedFlag(boolean flag)
   {
      mUnsynchronizedFlag = flag;
   }
   
   /**
    * Gets the unsynchronized flag.
    * 
    * @return the status of the unsynchronized flag.
    */
   public boolean getUnsynchronizedFlag()
   {
      return mUnsynchronizedFlag;
   }
   
   /**
    * Sets the extended header flag.
    *
    * @param flag the boolean value to set the flag to. 
    */
   public void setExtendedHeaderFlag(boolean flag)
   {
      mExtendedHeaderFlag = flag;
   }
   
   /**
    * Gets the extended header flag.
    * 
    * @return the status of the extended header flag.
    */
   public boolean getExtendedHeaderFlag()
   {
      return mExtendedHeaderFlag;
   }
   
   /**
    * Sets the experimental flag.
    *
    * @param flag the boolean value to set the flag to. 
    */
   public void setExperimentalFlag(boolean flag)
   {
      mExperimentalFlag = flag;
   }
   
   /**
    * Gets the experimental flag.
    * 
    * @return the status of the experimental flag.
    */
   public boolean getExperimentalFlag()
   {
      return mExperimentalFlag;
   }
   
   /**
    * Sets the tag size. This is the extended header size plus the frame sizes.
    *
    * @param tagSize the tag size to use. 
    */
   public void setTagSize(int tagSize)
   {
      mTagSize = tagSize;
   }
   
   /**
    * Gets the tag size. This is the extended header size plus the frame sizes.
    * 
    * @return the tag size, which is the extended header plus the frames.
    */
   public int getTagSize()
   {
      return mTagSize;
   }
   
   /**
    * Converts the given integer into a 32-bit (4 byte) synchsafe byte array.
    * 
    * Each byte has its most significant bit is cleared to allow for
    * unsynchronization (so that an unrecognized ID3 tag will not be
    * played as audio data). The byte-order is Big Endian.
    * 
    * @param integer the integer to convert.
    * @param b the byte array to write the 4 bytes to.
    * @param offset the offset at which to start writing.
    */
   public static void convertIntToSynchsafeBytes(
      int integer, byte[] b, int offset)
   {
      // only 28 significant bits in the integer
      for(int i = 0; i < 4; i++)
      {
         b[offset + i] = (byte)((integer >> (28 - ((i + 1) * 7))) & 0x7F);
      }
   }
   
   /**
    * Converts a 32-bit (4 byte) synchsafe byte array into an integer. The
    * byte-order is Big Endian and the most significant bit of each byte
    * is cleared to allow for unsynchronization (so that an unrecognized
    * ID3 tag will not be played as audio data).
    * 
    * @param b the byte array to read from.
    * @param offset the offset to start reading from.
    * 
    * @return the converted size. 
    */
   public static int convertSynchsafeBytesToInt(byte[] b, int offset)
   {
      int rval = 0;
      
      // most significant byte first
      for(int i = 0; i < 4; i++)
      {
         rval |= (b[offset + i] << ((3 - i) * 7));
      }
      
      return rval;
   }
}
