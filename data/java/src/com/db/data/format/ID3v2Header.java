/*
 * Copyright (c) 2003-2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

/**
 * An ID3v2 header. The currently supported version.revision is 3.0.
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
 * ID3v2 version              $04 00
 * ID3v2 flags                %abcd0000
 * ID3v2 size             4 * %0xxxxxxx
 * 
 * @author Dave Longley
 */
public class ID3v2Header
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
    * The size of an ID3v2 header (or footer) in bytes.
    */
   public static final int HEADER_SIZE = 10;
   
   /**
    * Unsynchronized flag bit (describes ID3 tag content).
    */
   public static final int UNSYNCHRONIZED_BIT = 0x80; // bit 7
   
   /**
    * Extended header flag bit (describes ID3 tag content).
    */
   public static final int EXTENDEDHEADER_BIT = 0x40; // bit 6
   
   /**
    * Experimental flag bit (describes ID3 tag content).
    */
   public static final int EXPERIMENTAL_BIT = 0x20;   // bit 5
   
   /**
    * Footer flag bit (describes ID3 tag content).
    */
   public static final int FOOTER_BIT = 0x10;         // bit 4

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
    * Set to true if the footer flag is set.
    */
   protected boolean mFooterFlag;
   
   /**
    * The size of the ID3 tag. This is the sum of the size of the
    * extended header and the frames (and padding).
    */
   protected int mSize;
   
   /**
    * Constructs an ID3v2 header with default values.
    */
   public ID3v2Header()
   {
      mVersion = SUPPORTED_VERSION;
      mRevision = SUPPORTED_REVISION;
      mUnsynchronizedFlag = false;
      mExtendedHeaderFlag = false;
      mExperimentalFlag = false;
      mFooterFlag = false;
      mSize = 0;
   }
   
   /**
    * Constructs an ID3v2 header with the passed parameters.
    * 
    * @param version the version of the ID3 tag.
    * @param revision the revision of the version.
    * @param unsynchronized true if all frames are unsynchronized,
    *                       false if not.
    * @param extendedHeader true if the ID3 tag uses an extended header,
    *                       false if not.
    * @param experimental true if the ID3 tag is experimental, false if not.
    * @param footer true if the ID3 tag has a footer, false if not.
    * @param size the size of the extended header plus all of the frames.
    */
   public ID3v2Header(int version, int revision,
                      boolean unsynchronized, boolean extendedHeader,
                      boolean experimental, boolean footer,
                      int size)
   {
      mVersion = version;
      mRevision = revision;
      mUnsynchronizedFlag = unsynchronized;
      mExtendedHeaderFlag = extendedHeader;
      mExperimentalFlag = experimental;
      mFooterFlag = footer;
      mSize = size;
   }
   
   /**
    * Constructs an ID3v2 header with the passed parameters.
    * 
    * @param unsynchronized true if all frames are unsynchronized,
    *                       false if not.
    * @param extendedHeader true if the ID3 tag uses an extended header,
    *                       false if not.
    * @param experimental true if the ID3 tag is experimental, false if not.
    * @param footer true if the ID3 tag has a footer, false if not.
    * @param size the size of the extended header plus all of the frames.
    */
   public ID3v2Header(boolean unsynchronized, boolean extendedHeader,
                      boolean experimental, boolean footer,
                      int size)
   {
      mVersion = SUPPORTED_VERSION;
      mRevision = SUPPORTED_REVISION;
      mUnsynchronizedFlag = unsynchronized;
      mExtendedHeaderFlag = extendedHeader;
      mExperimentalFlag = experimental;
      mFooterFlag = footer;
      mSize = size;
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
         b |= UNSYNCHRONIZED_BIT;
      if(mExtendedHeaderFlag)
         b |= EXTENDEDHEADER_BIT;
      if(mExperimentalFlag)
         b |= EXPERIMENTAL_BIT;
      if(mFooterFlag)
         b |= FOOTER_BIT;
      
      return b;
   }
   
   /**
    * Converts size into a 4 byte array where each byte has its
    * most significant bit set to zero to allow for unsynchronization.
    * The array is ordered from most significant byte to least. 
    * 
    * @return a 4 byte array representing the size.
    */
   protected byte[] convertSizeToBytes()
   {
      byte[] size = new byte[4];
      
      // only 28 significant bits in the integer
      for(int i = 0; i < 4; i++)
         size[i] = (byte)((mSize >> (28 - ((i + 1) * 7))) & 0x7F);
      
      return size;
   }
   
   /**
    * Converts a 4 bytes from an array into an integer size. The 4 bytes are
    * ordered from most significant byte to least, and each byte has
    * its most significant bit set to zero.
    * 
    * @param b the byte array to read from.
    * @param offset the offset to start reading from.
    * @param length the number of valid bytes in the byte array.
    * @return true if successful, false if not. 
    */
   protected boolean convertBytesToSize(byte[] b, int offset, int length)
   {
      boolean rval = false;
      
      if(b != null && length >= offset + 4)
      {
         mSize = 0;
         
         // most significant byte first
         for(int i = 0; i < 4; i++)
         {
            mSize |= (b[offset + i] << ((3 - i) * 7));
         }
         
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Converts the header into a 10 byte array.
    * 
    * @return the header as a 10 byte array. 
    */
   public byte[] convertToBytes()
   {
      byte[] header = new byte[HEADER_SIZE];
      
      header[0] = 0x49; // "I"
      header[1] = 0x44; // "D"
      header[2] = 0x33; // "3"
      header[3] = (byte)(mVersion & 0xFF);
      header[4] = (byte)(mRevision & 0xFF);
      header[5] = getFlagByte();
      
      // get size as a 4 byte array
      byte[] size = convertSizeToBytes();
      
      header[6] = size[0];
      header[7] = size[1];
      header[8] = size[2];
      header[9] = size[3];
         
      return header;
   }
   
   /**
    * Converts the header from a byte array with at least 10 bytes.
    * 
    * @param b the byte array to convert from.
    * @param offset the offset to start converting from.
    * @param length the number of valid bytes in the byte array.
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
               rval = convertBytesToSize(b, offset + 6, length);
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
      mFooterFlag = (b & FOOTER_BIT) != 0;
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
    * Sets the footer flag.
    *
    * @param flag the boolean value to set the flag to. 
    */
   public void setFooterFlag(boolean flag)
   {
      mFooterFlag = flag;
   }
   
   /**
    * Gets the footer flag.
    * 
    * @return the status of the footer flag.
    */
   public boolean getFooterFlag()
   {
      return mFooterFlag;
   }
   
   /**
    * Sets the size. This is the extended header size plus the frame sizes.
    *
    * @param size the size to use. 
    */
   public void setSize(int size)
   {
      mSize = size;
   }
   
   /**
    * Gets the size. This is the extended header size plus the frame sizes.
    * 
    * @return the size of the extended header plus the frames.
    */
   public int getSize()
   {
      return mSize;
   }
   
   /**
    * Gets the total ID3 tag size.
    *
    * @return the total ID3 tag size. 
    */
   public int getTotalTagSize()
   {
      // add header and footer size
      return mSize + HEADER_SIZE + ((mFooterFlag) ? HEADER_SIZE : 0);
   }
}
