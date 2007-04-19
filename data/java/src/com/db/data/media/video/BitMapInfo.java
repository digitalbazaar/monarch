/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media.video;

import java.io.IOException;
import java.io.OutputStream;

/**
 * A BITMAPINFO structure.
 * 
 * The structure contains:
 * -----------------------
 * BITMAPINFOHEADER - header for this BITMAPINFO
 * 
 * RGBQUAD[] - an array of RGBQUAD elements 
 * OR
 * An array of 16-bit unsigned integers that specify indices into a palette
 * 
 * @author Dave Longley
 */
public class BitMapInfo
{
   /**
    * The header for the structure.
    */
   protected BitMapInfoHeader mHeader;
   
   /**
    * The data for structure.
    */
   protected byte[] mData;
   
   /**
    * Constructs a new BitMapInfo.
    */
   public BitMapInfo()
   {
      // create header
      mHeader = new BitMapInfoHeader();
      
      // create empty data
      mData = new byte[0];
   }
   
   /**
    * Writes this BitMapInfo to an OutputStream.
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
    * Converts this BitMapInfo from a byte array.
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
      
      // convert header
      if(mHeader.convertFromBytes(b, offset, length))
      {
         // make sure length has enough data
         if(length >= mHeader.getBitMapInfoSize())
         {
            mData = new byte[mHeader.getBitMapInfoSize()];
            System.arraycopy(b, offset, mData, 0, getSize());
            
            // converted successfully
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the size of this BitMapInfo.
    * 
    * @return the size of this BitMapInfo.
    */
   public int getSize()
   {
      return mData.length;
   }
}
