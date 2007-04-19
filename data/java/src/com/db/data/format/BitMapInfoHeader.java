/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

import java.io.IOException;
import java.io.OutputStream;

/**
 * A BITMAPINFOHEADER structure.
 * 
 * -------------------------------------------------------------------------
 * (a DWORD is 4 bytes, a WORD is 2 bytes, a LONG is 8 bytes)
 * (5 DWORDS = 20 bytes + 2 WORDS (4 bytes) + 4 LONGS (32 bytes) = 56 bytes:
 * -------------------------------------------------------------------------
 * DWORD size - the size of the structure, in bytes (EXCLUDES color table/masks)
 * LONG width - width of the bitmap in pixels
 * LONG height - height of the bitmap in pixels
 * WORD planes - the number of planes for the device, SET TO 1
 * WORD bit count - the number of bits per pixel
 * DWORD compression - the type of compression
 * DWORD image size - the size, in bytes, of the image
 * LONG X pixels per meter - horizontal resolution
 * LONG Y pixels per meter - vertical resolution
 * DWORD number of color indices in  the color table, zero = maximum
 * DWORD number of color indices required for displaying the map, zero = all
 * 
 * @author Dave Longley
 */
public class BitMapInfoHeader
{
   /**
    * The data for structure.
    */
   protected byte[] mData;
   
   /**
    * Constructs a new BitMapInfoHeader.
    */
   public BitMapInfoHeader()
   {
      // create data
      mData = new byte[56];
   }
   
   /**
    * Writes this BitMapInfoHeader to an OutputStream.
    * 
    * @param os the OutputStream to write to.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public void writeTo(OutputStream os) throws IOException
   {
      // write data
      os.write(mData);
   }
   
   /**
    * Converts this BitMapInfoHeader from a byte array.
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
      
      // make sure length has enough data
      if(length >= 56)
      {
         System.arraycopy(b, offset, mData, 0, getSize());
         
         // converted successfully
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the size of this BitMapInfoHeader.
    * 
    * @return the size of this BitMapInfoHeader.
    */
   public int getSize()
   {
      return mData.length;
   }
}
