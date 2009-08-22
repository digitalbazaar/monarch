/*
 * Copyright (c) 2005-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media;

/**
 * A RIFF Form header. The file size does *NOT* include the header size. So
 * it should, usually, be the length of the file - 8.
 * 
 * The format of the header is such:
 * 
 * | bytes 1-4  | bytes 5-8 | bytes 9-12 | bytes 13+ |
 * ---------------------------------------------------
 * |   "RIFF"   | file size |   "XXXX"   | file data |
 * 
 * For AVI files:
 * 
 * | bytes 1-4  | bytes 5-8 | bytes 9-12 | bytes 13+ |
 * ---------------------------------------------------
 * |   "RIFF"   | file size |   "AVI "   | file data |
 * 
 * In hex form:
 *  
 * 52 49 46 46 yy yy yy yy xx xx xx xx zz ...
 * 
 * AVI hex form:
 *  
 * 52 49 46 46 yy yy yy yy 41 56 49 20 zz ...
 *
 * @author Dave Longley
 */
public class RiffFormHeader extends RiffListHeader
{
   /**
    * The header size for a form chunk.
    */
   public static final int FORM_HEADER_SIZE = LIST_HEADER_SIZE;
   
   /**
    * Constructs a Riff Form header with default values.
    */
   public RiffFormHeader()
   {
      this(null);
   }
   
   /**
    * Constructs a Riff Form header with the passed parameters.
    * 
    * @param form the form of RIFF.
    */
   public RiffFormHeader(String form)
   {
      super(form);
      mChunkHeader.setIdentifier("RIFF");
   }
   
   /**
    * Constructs a Riff Form header with the passed parameters.
    * 
    * @param form the form of RIFF.
    * @param fileSize the file size of the file the header is for.
    */
   public RiffFormHeader(String form, int fileSize)
   {
      super(form, fileSize);
      mChunkHeader.setIdentifier("RIFF");
   }
   
   /**
    * Converts the header from a byte array with at least 12 bytes.
    * 
    * @param b the byte array to convert from.
    * @param offset the offset to start converting from.
    * @param length the number of valid bytes beyond the offset.
    * 
    * @return true if successful, false if not.
    */
   @Override
   public boolean convertFromBytes(byte[] b, int offset, int length)
   {
      boolean rval = false;
      
      if(b != null && length >= LIST_HEADER_SIZE)
      {
         if(mChunkHeader.convertFromBytes(b, offset, length))
         {
            // make sure chunk identifier is RIFF
            if(mChunkHeader.getIdentifier().equals("RIFF"))
            {
               String id = "";
               id += (char)b[offset + 8];
               id += (char)b[offset + 9];
               id += (char)b[offset + 10];
               id += (char)b[offset + 11];
               
               setIdentifier(id);

               rval = true;
            }
         }
      }
      
      setValid(rval);
      
      return rval;      
   }
   
   /**
    * Sets the file size. This is the size of the entire RIFF formatted file.
    *
    * @param fileSize the size of the entire RIFF formatted file. 
    */
   public void setFileSize(long fileSize)
   {
      setChunkSize(fileSize);
   }
   
   /**
    * Gets the file size. This is the size of the entire RIFF formatted file.
    * 
    * @return the size of the entire RIFF formatted file.
    */
   public long getFileSize()
   {
      return getChunkSize();
   }
   
   /**
    * Gets the header size.
    *
    * @return the header size. 
    */
   public int getHeaderSize()
   {
      return LIST_HEADER_SIZE;
   }
}
