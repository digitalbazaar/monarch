/*
 * Copyright (c) 2004-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media.audio;

/**
 * An Id3TagFrameHeader stores the ID of an ID3 tag frame, its size, and
 * some flags.
 * 
 * @author Dave Longley
 */
public class Id3TagFrameHeader
{
   /**
    * The size of an ID3v2 tag frame header in bytes.
    */
   public static final int HEADER_SIZE = 10;
   
   /**
    * Tag altered discard/preserve flag (bit 7 of flag byte 1).
    */
   public static final int TAG_ALTERED_DISCARD_FRAME_BIT = 0x80;
   
   /**
    * File altered discard/preserve flag (bit 6 of flag byte 1).
    */
   public static final int FILE_ALTERED_DISCARD_FRAME_BIT = 0x40;
   
   /**
    * Read only flag (bit 5 of flag byte 1).
    */
   public static final int READ_ONLY_BIT = 0x20;
   
   /**
    * Compression flag (bit 7 of flag byte 2).
    */
   public static final int COMPRESSION_BIT = 0x80;
   
   /**
    * Encryption flag (bit 6 of flag byte 2).
    */
   public static final int ENCRYPTION_BIT = 0x40;
   
   /**
    * Grouping flag (bit 5 of flag byte 2).
    */
   public static final int GROUPING_BIT = 0x20;
   
   /**
    * The ID of the frame.
    */
   protected String mId;
   
   /**
    * Sets the description of the frame. This is useful for differentiation
    * when multiple frames with the same ID are used but contain different
    * data.
    */
   protected String mDescription;
   
   /**
    * The size of the frame, in bytes.
    */
   protected int mFrameSize;
   
   /**
    * This flag indicates whether the frame should be preserved or discarded
    * if the frame is unknown and the tag is altered in any way. 
    */
   protected boolean mTagAlteredDiscardFrame;
   
   /**
    * This flag indicates whether the frame should be preserved or discarded
    * if the frame is unknown and the file, excluding the tag, is altered
    * in any way.
    */
   protected boolean mFileAlteredDiscardFrame;
   
   /**
    * This flag indicates whether or not the frame should be read only.
    */
   protected boolean mReadOnly;
   
   /**
    * This flag indicates whether or not the frame is zlib-compressed. If it
    * is, then 4 bytes can be found at the end of the frame header that
    * indicate the decompressed size for the frame data.
    */
   protected boolean mCompressed;
   
   /**
    * This flag indicates whether or not the frame is encrypted.
    */
   protected boolean mEncrypted;
   
   /**
    * This flag indicates whether or not the frame belongs to a group of
    * other frames.
    */
   protected boolean mGrouped;
   
   /**
    * Creates an Id3TagFrameHeader.
    */
   public Id3TagFrameHeader()
   {
      mId = "";
      mDescription = "";
      mFrameSize = 0;
      mTagAlteredDiscardFrame = false;
      mFileAlteredDiscardFrame = false;
      mReadOnly = false;
      mCompressed = false;
      mEncrypted = false;
      mGrouped = false;
   }
   
   /**
    * Sets the flags based on the first flag byte.
    * 
    * @param b the first flag byte.
    */
   protected void setFlags1(byte b)
   {
      mTagAlteredDiscardFrame = (b & TAG_ALTERED_DISCARD_FRAME_BIT) != 0;
      mFileAlteredDiscardFrame = (b & FILE_ALTERED_DISCARD_FRAME_BIT) != 0;
      mReadOnly = (b & READ_ONLY_BIT) != 0;
   }
   
   /**
    * Gets the first flag byte.
    * 
    * @return the first flag byte.
    */
   protected byte getFlagByte1()
   {
      byte b = 0x00;
      
      if(mTagAlteredDiscardFrame)
      {
         b |= TAG_ALTERED_DISCARD_FRAME_BIT;
      }
      
      if(mFileAlteredDiscardFrame)
      {
         b |= FILE_ALTERED_DISCARD_FRAME_BIT;
      }
      
      if(mReadOnly)
      {
         b |= READ_ONLY_BIT;
      }
      
      return b;
   }
   
   /**
    * Sets the flags based on the second flag byte.
    * 
    * @param b the second flag byte.
    */
   protected void setFlags2(byte b)
   {
      mCompressed = (b & COMPRESSION_BIT) != 0;
      mEncrypted = (b & ENCRYPTION_BIT) != 0;
      mGrouped = (b & GROUPING_BIT) != 0;
   }
   
   /**
    * Gets the second flag byte.
    * 
    * @return the second flag byte.
    */
   protected byte getFlagByte2()
   {
      byte b = 0x00;
      
      if(mCompressed)
      {
         b |= COMPRESSION_BIT;
      }
      
      if(mEncrypted)
      {
         b |= ENCRYPTION_BIT;
      }
      
      if(mGrouped)
      {
         b |= GROUPING_BIT;
      }
      
      return b;
   }
   
   /**
    * Converts this tag frame header from the given byte array.
    * 
    * @param b the buffer with the data to convert from.
    * @param offset the offset at which to begin converting.
    * @param length the number of valid bytes in the byte array.
    */
   public void convertFromBytes(byte[] b, int offset, int length)
   {
      // convert ID
      setId(new String(b, offset, 4));
      
      // convert frame size
      setFrameSize(convertBytesToInt(b, offset + 4));
      
      // convert flags
      setFlags1(b[offset + 8]);
      setFlags2(b[offset + 9]); 
   }
   
   /**
    * Converts this tag frame header to a byte array.
    * 
    * @param b the byte array to write to.
    * @param offset the offset at which to begin writing.
    */
   public void convertToBytes(byte[] b, int offset)
   {
      // copy ID
      byte[] id = getId().getBytes();
      b[offset] = b[offset + 1] = b[offset + 2] = b[offset + 3] = 0;
      System.arraycopy(id, 0, b, offset, id.length);
      
      // set size
      convertIntToBytes(getFrameSize(), b, offset + 4);
      
      // set flags
      b[offset + 8] = getFlagByte1();
      b[offset + 9] = getFlagByte2();
   }
   
   /**
    * Sets the ID of the frame.
    * 
    * @param id the ID to use.
    */
   public void setId(String id)
   {
      mId = id;
   }
   
   /**
    * Gets the ID of the frame.
    * 
    * @return the ID of the frame.
    */
   public String getId()
   {
      return mId;
   }
   
   /**
    * Sets the description of the frame.
    * 
    * @param description the description to use.
    */
   public void setDescription(String description)
   {
      mDescription = description;
   }
   
   /**
    * Gets the description of the frame.
    * 
    * @return the description of the frame.
    */
   public String getDescription()
   {
      return mDescription;
   }   
   
   /**
    * Sets the size of the frame (total frame length - header size of 10).
    * 
    * @param size the size, in bytes, of the frame, excluding the header.
    */
   public void setFrameSize(int size)
   {
      mFrameSize = size;
   }
   
   /**
    * Gets the size of the frame (total frame length - header size of 10).
    * 
    * @return the size, in bytes, of the frame, excluding the header.
    */
   public int getFrameSize()
   {
      return mFrameSize;
   }
   
   /**
    * Sets whether or not the frame should be discarded if it is unknown
    * and the tag is altered.
    * 
    * @param discard true to discard, false not to.
    */
   public void setTagAlteredDiscardFrame(boolean discard)
   {
      mTagAlteredDiscardFrame = discard;
   }
   
   /**
    * Gets whether or not the frame should be discarded if it is unknown
    * and the tag is altered.
    * 
    * @return true to discard, false not to.
    */
   public boolean getTagAlteredDiscardFrame()
   {
      return mTagAlteredDiscardFrame;
   }
   
   /**
    * Sets whether or not the frame should be discarded if it is unknown
    * and the file, but not the tag, is altered.
    * 
    * @param discard true to discard, false not to.
    */
   public void setFileAlteredDiscardFrame(boolean discard)
   {
      mFileAlteredDiscardFrame = discard;
   }
   
   /**
    * Gets whether or not the frame should be discarded if it is unknown
    * and the file, but not the tag, is altered.
    * 
    * @return true to discard, false not to.
    */
   public boolean getFileAlteredDiscardFrame()
   {
      return mFileAlteredDiscardFrame;
   }
   
   /**
    * Sets whether or not the frame is read only.
    * 
    * @param readOnly true if the frame is read only, false if not.
    */
   public void setReadOnly(boolean readOnly)
   {
      mReadOnly = readOnly;
   }
   
   /**
    * Gets whether or not the frame is read only.
    * 
    * @return true if the frame is read only, false if not.
    */
   public boolean isReadOnly()
   {
      return mReadOnly;
   }
   
   /**
    * Sets whether or not the frame is compressed using zlib.
    * 
    * @param compressed true if the frame is compressed, false if not.
    */
   public void setCompressed(boolean compressed)
   {
      mCompressed = compressed;
   }
   
   /**
    * Gets whether or not the frame is compressed using zlib.
    * 
    * @return true if the frame is compressed, false if not.
    */
   public boolean isCompressed()
   {
      return mCompressed;
   }
   
   /**
    * Sets whether or not the frame is encrypted.
    * 
    * @param encrypted true if the frame is encrypted, false if not.
    */
   public void setEncrypted(boolean encrypted)
   {
      mEncrypted = encrypted;
   }
   
   /**
    * Gets whether or not the frame is encrypted.
    * 
    * @return true if the frame is encrypted, false if not.
    */
   public boolean isEncrypted()
   {
      return mEncrypted;
   }
   
   /**
    * Sets whether or not the frame is grouped.
    * 
    * @param grouped true if the frame is grouped, false if not.
    */
   public void setGrouped(boolean grouped)
   {
      mGrouped = grouped;
   }
   
   /**
    * Gets whether or not the frame is grouped.
    * 
    * @return true if the frame is grouped, false if not.
    */
   public boolean isGrouped()
   {
      return mGrouped;
   }
   
   /**
    * Converts this ID3 tag frame header to a human-readable string.
    * 
    * @return the human readable string.
    */
   @Override
   public String toString()
   {
      return
         "[ID3TagFrameHeader]\n" +
         "Frame ID=" + getId() + "\n" +
         "Frame Size=" + getFrameSize() + "\n";
   }
   
   /**
    * Converts the given integer into a 32-bit (4 byte) byte array. The
    * byte-order is Big Endian.
    * 
    * @param integer the integer to convert.
    * @param b the byte array to write the 4 bytes to.
    * @param offset the offset at which to start writing.
    */
   public static void convertIntToBytes(int integer, byte[] b, int offset)
   {
      for(int i = 0; i < 4; i++)
      {
         b[offset + i] = (byte)((integer >> ((3 - i) * 8)) & 0xFF);
      }
   }
   
   /**
    * Converts a 32-bit (4 byte) byte array into an integer. The byte-order
    * is Big Endian.
    * 
    * @param b the byte array to read from.
    * @param offset the offset to start reading from.
    * 
    * @return the converted size. 
    */
   public static int convertBytesToInt(byte[] b, int offset)
   {
      int rval = 0;
      
      // most significant byte first
      for(int i = 0; i < 4; i++)
      {
         rval |= (b[offset + i] << ((3 - i) * 8));
      }
      
      return rval;
   }
}
