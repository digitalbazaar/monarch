/*
 * Copyright (c) 2005-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

import java.io.IOException;
import java.io.OutputStream;

/**
 * A RIFF List Header.
 * 
 * The format of a RIFF List is such:
 * 
 * | bytes 1-4  | bytes 5-8 | bytes 9-12 | bytes 13+ |
 * ---------------------------------------------------
 * |   "LIST"   | list size |   "XXXX"   | list data |
 * 
 * @author Dave Longley
 */
public class RiffListHeader
{
   /**
    * A Riff Chunk Header.
    */
   protected RiffChunkHeader mChunkHeader;
   
   /**
    * The 4 byte list identifier.
    */
   protected String mId;
   
   /**
    * The header size for a list chunk.
    */
   public static final int LIST_HEADER_SIZE = 12;
   
   /**
    * Constructs a Riff List Header with default values.
    */
   public RiffListHeader()
   {
      this(null, LIST_HEADER_SIZE);
   }
   
   /**
    * Constructs a Riff List Header with the passed parameters.
    * 
    * @param id the identifier of the list.
    */
   public RiffListHeader(String id)
   {
      this(id, LIST_HEADER_SIZE);
   }   
   
   /**
    * Constructs a Riff List Header with the passed parameters.
    * 
    * @param id the identifier of the list.
    * @param size the size of the list.
    */
   public RiffListHeader(String id, int size)
   {
      mChunkHeader = new RiffChunkHeader("LIST", size);
      
      if(id == null || id.length() != 4)
      {
         mId = "";
      }
      else
      {
         mId = id;
      }
   }
   
   /**
    * Writes this RiffListHeader to an OutputStream.
    * 
    * @param os the OutputStream to write to.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public void writeTo(OutputStream os) throws IOException
   {
      // write chunk header
      mChunkHeader.writeTo(os);
      
      // write LIST ID
      os.write((byte)mId.charAt(0));
      os.write((byte)mId.charAt(1));
      os.write((byte)mId.charAt(2));
      os.write((byte)mId.charAt(3));
   }   
   
   /**
    * Converts the header into a 12 byte array.
    * 
    * @return the header as a 12 byte array. 
    */
   public byte[] convertToBytes()
   {
      byte[] chunk = mChunkHeader.convertToBytes();
      
      byte[] list = new byte[LIST_HEADER_SIZE];
      System.arraycopy(chunk, 0, list, 0, chunk.length);
      
      if(mId.length() == 4)
      {
         list[8]  = (byte)mId.charAt(0);
         list[9]  = (byte)mId.charAt(1);
         list[10] = (byte)mId.charAt(2);
         list[11] = (byte)mId.charAt(3);
      }

      return list;
   }
   
   /**
    * Converts the header from a byte array with at least 12 bytes.
    * 
    * @param b the byte array to convert from.
    * @param offset the offset to start converting from.
    * @param length the number of valid bytes in the buffer following the
    *               offset.
    * 
    * @return true if successful, false if not.
    */
   public boolean convertFromBytes(byte[] b, int offset, int length)
   {
      boolean rval = false;
      
      if(b != null && length >= LIST_HEADER_SIZE)
      {
         if(mChunkHeader.convertFromBytes(b, offset, length))
         {
            // make sure chunk identifier is LIST
            if(mChunkHeader.getIdentifier().equals("LIST"))
            {
               mId = "";
               mId += (char)b[offset + 8];
               mId += (char)b[offset + 9];
               mId += (char)b[offset + 10];
               mId += (char)b[offset + 11];
               
               rval = true;
            }
         }
      }
      
      setValid(rval);
      
      return rval;      
   }
   
   /**
    * Sets the 4 byte identifier for this list if the passed string is
    * 4 characters long.
    * 
    * @param id the identifier to set.
    * 
    * @return true if set, false if not.
    */
   public boolean setIdentifier(String id)
   {
      boolean rval = false;
      
      if(id != null && id.length() == 4)
      {
         mId = id;
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the 4 byte identifier for this list.
    * 
    * @return the list identifier.
    */
   public String getIdentifier()
   {
      return mId;
   }
   
   /**
    * Sets the list size.
    *
    * @param size the size of the Riff list. 
    */
   public void setListSize(long size)
   {
      mChunkHeader.setChunkSize(size);
   }
   
   /**
    * Gets the list size.
    * 
    * @return the size of the Riff list.
    */
   public long getListSize()
   {
      return mChunkHeader.getChunkSize();
   }
   
   /**
    * Returns whether or not this list is valid.
    * 
    * @return true if valid, false if not.
    */
   public boolean isValid()
   {
      return mChunkHeader.isValid();
   }
   
   /**
    * Sets whether or not this list is valid.
    * 
    * @param valid true to set to valid, false to set to invalid.
    */
   public void setValid(boolean valid)
   {
      mChunkHeader.setValid(valid);
   }
}
