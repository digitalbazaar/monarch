/*
 * Copyright (c) 2003-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media.audio;

import com.db.logging.LoggerManager;

import java.util.HashMap;

/**
 * The Id3Tag class is used to parse, display/edit and write ID3 version
 * 2.3 and 2.4 compliant tags.
 * 
 * @author Manu Sporny
 * @author Dave Longley
 */
public class Id3Tag
{
   /**
    * A ID3v2 header.
    */
   protected Id3v2Header mHeader;
   
   /**
    * For storing an extended header.
    */
   protected byte[] mExtendedHeader;
   
   /**
    * Whether or not this tag is valid.
    */
   protected boolean mValid;
   
   /**
    * The size of this tag.
    */
   protected int mSize;
   
   /**
    * A mapping of tag frame name to tag frame. 
    */
   protected HashMap<String, Id3TagFrame> mTagFrames;
   
   /**
    * Creates an ID3 tag.
    */
   public Id3Tag()
   {
      mHeader = null;
      mExtendedHeader = null;
      
      mSize = 0;
      mValid = false;
      mTagFrames = new HashMap<String, Id3TagFrame>();
   }

   /**
    * Parses the extended header of an ID3 tag.
    * 
    * @param tagOffset the beginning offset of the tag in the byte array.
    * @param b the byte array that contains the entire tag.
    * @param length the number of valid bytes in the byte array.
    * @return the ending offset of the extended header.
    */
   protected int parseExtendedHeader(byte[] b, int tagOffset, int length)
   {
      int tagSize = 0;
      int b1, b2, b3, b4;
      
      b1 = b[tagOffset] << 21;
      b2 = b[tagOffset + 1] << 14;
      b3 = b[tagOffset + 2] << 7;
      b4 = b[tagOffset + 3];
      tagSize = (b1 | b2 | b3 | b4) + 10;
      
      mExtendedHeader = new byte[tagSize + 1];
      for(int i = 0; i < tagSize; i++)
      {
         mExtendedHeader[i] = b[tagOffset + i];
      }
         
      return tagOffset + tagSize;
   }
   
   /**
    * Parses an ID3 tag frame from a given byte array starting at the given 
    * offset.
    * 
    * @param b the byte array which contains the tag.
    * @param tagOffset the offset at which the tag begins.
    * @param length the number of valid bytes in the byte array.
    * @return the ending offset of the tag.
    */
   protected int parseID3TagFrame(byte[] b, int tagOffset, int length)
   {
      Id3TagFrame id3tf = new Id3TagFrame();
      
      id3tf.convertFromBytes(b, tagOffset, length);
      
      //LoggerManager.debug("dbdata", "Parsed Tag (" + tagOffset + "-" + 
      //      (tagOffset + id3tf.getSize()) + "): " + id3tf.convertToString());
      
      mTagFrames.put(id3tf.getName(), id3tf);
      
      return tagOffset + id3tf.getSize();
   }
   
   /**
    * Adds a ID3 tag frame to this ID3 tag. 
    * 
    * @param name the name of the frame to add.
    * @param data the data associated with the frame.
    * @return true if the addition of the tag was successful, false otherwise
    */
   public boolean addTagFrame(String name, String data)
   {
      boolean rval = false;
      
      Id3TagFrame id3tf = new Id3TagFrame();
         
      id3tf.setName(name);
      id3tf.setData(data);
      
      mTagFrames.put(id3tf.getName(), id3tf);
      rval = true;
      
      return rval;
   }
   
   /**
    * Gets the ID3 tag frame from its ID3 tag name.
    * 
    * @param name the name of the frame.
    * @return the id3 tag frame or null if the name is not found.
    */
   public Id3TagFrame getTagFrame(String name)
   {
      return mTagFrames.get(name);
   }
   
   /**
    * Gets the ID3v2 header for this tag.
    * 
    * @return the ID3v2 header for this tag.
    */
   public Id3v2Header getHeader()
   {
      return mHeader;
   }
   
   /**
    * Parses a set of bytes into a valid ID3 tag that can be modified, and
    * written back into a set of bytes.
    * 
    * @param b the byte array to read the ID3 tag from.
    * @param offset the offset to start reading the tag at.
    * @param length the number of valid bytes in the passed byte array.
    * @return true if parsing the tags was successful, false otherwise
    */
   public boolean convertFromBytes(byte[] b, int offset, int length)
   {
      boolean rval = false;
      
      // try to convert byte array into a valid ID3v2 header
      mHeader = new Id3v2Header();
      if(mHeader.convertFromBytes(b, offset, length))
      {
         int tagOffset = offset;
         mSize = mHeader.getTotalTagSize();
         
         if(mSize <= length)
         {
            if(mHeader.getExtendedHeaderFlag())
            {
               tagOffset = parseExtendedHeader(b, tagOffset, length);
            }
            else
            {
               tagOffset = Id3v2Header.HEADER_SIZE;
            }
               
            while(tagOffset < mSize)
            {
               tagOffset = parseID3TagFrame(b, tagOffset, length);
            }
              
            //LoggerManager.getLogger("dbdata").debug(
            //   "Total tag size: "+ mSize);
            //LoggerManager.getLogger("dbdata").debug("   Parsing ID3 Tag...");
            rval = true;
         }
         else
         {
            LoggerManager.getLogger("dbdata").debug(getClass(),
               "Could not parse ID3 tag, not enough data.");
            LoggerManager.getLogger("dbdata").debug(getClass(),
               mSize + " bytes needed, " + length + " bytes given");
         }
      }
      else
      {
         LoggerManager.getLogger("dbdata").debug(getClass(),
            "No existing ID3v2 header found.");
      }
      
      return rval;
   }
   
   /**
    * Converts all of the internal information of this ID3 tag object to a byte
    * array that can be written directly to the beginning of an MPEG audio 
    * stream in ID3v2.4.0 format.
    * 
    * @return the byte array of the tag.
    */
   public byte[] convertToBytes()
   {
      byte[] id3Bytes = null;
      
      // if this is a new id3 tag, create a new header
      if(mHeader == null)
      {
         mHeader = new Id3v2Header();
      }

      // calculate the size for the ID3 header data
      int tagSize = 0;
      
      // add extended tag header size
      if(mExtendedHeader != null)
      {
         tagSize += mExtendedHeader.length;
      }
      
      // add tag frames
      for(Id3TagFrame id3tf: mTagFrames.values())
      {
         tagSize += id3tf.getSize();
      }
      
      // set the size in the header
      mHeader.setSize(tagSize);
      
      // get total tag size
      int totalTagSize = mHeader.getTotalTagSize();
      
      // create the byte array
      id3Bytes = new byte[totalTagSize];
      int offset = 0;

      // copy the tag header into the byte array
      byte[] header = mHeader.convertToBytes();
      System.arraycopy(header, 0, id3Bytes, 0, Id3v2Header.HEADER_SIZE);
      offset += Id3v2Header.HEADER_SIZE;
      
      // copy the extended tag header (if there is one) into the byte array
      if(mExtendedHeader != null)
      {
         System.arraycopy(mExtendedHeader, 0,
                          id3Bytes, offset,
                          mExtendedHeader.length);
         
         offset += mExtendedHeader.length;
      }
      
      // copy all of the individual tag frames into the byte array
      for(Id3TagFrame id3tf: mTagFrames.values())
      {
         byte[] b = id3tf.convertToBytes();
         
         System.arraycopy(b, 0, id3Bytes, offset, b.length);
         offset += b.length;
      }
      
      return id3Bytes;
   }

   /**
    * Converts this ID3 tag into a human-readable string.
    * 
    * @return the human readable string associated with this ID3 tag
    */
   public String convertToString()
   {
      String mStr = "";

      // convert tag frames to strings
      for(Id3TagFrame id3tf: mTagFrames.values())
      {
         mStr += id3tf.convertToString() + "\n";
      }
      
      return mStr;
   }
   
   /**
    * Converts this ID3Tag to a string.
    * 
    * @return the ID3Tag as a string.
    */
   @Override
   public String toString()
   {
      return convertToString();
   }
}
