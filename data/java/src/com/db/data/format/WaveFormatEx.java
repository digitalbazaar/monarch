/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

import java.io.IOException;
import java.io.OutputStream;

/**
 * A WAVEFORMATEX structure.
 * 
 * ----------------------------------------------------
 * (a DWORD is 4 bytes, a WORD is 2 bytes)
 * (2 DWORDS = 8 bytes + 5 WORDS (10 bytes) = 18 bytes:
 * ----------------------------------------------------
 * WORD format tag - the waveform-audio format type
 * WORD number of channels - 1 for mono, 2 for stereo
 * DWORD number of samples/sec (hertz)
 * DWORD number of average bytes/sec = (samples/sec * block align)
 * WORD block align - minimum atomic unit of data
 * WORD bits/sample
 * WORD extra size - # bytes of extra info, often zero
 * 
 * @author Dave Longley
 */
public class WaveFormatEx
{
   /**
    * The data for structure.
    */
   protected byte[] mData;
   
   /**
    * Constructs a new WaveFormatEx.
    */
   public WaveFormatEx()
   {
      mData = new byte[18];
   }
   
   /**
    * Writes this WaveFormatEx to an OutputStream.
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
    * Converts this WaveFormatEx from a byte array.
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
      if(length >= 18)
      {
         System.arraycopy(b, offset, mData, 0, getSize());
         
         // converted successfully
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the size of this WaveFormatEx.
    * 
    * @return the size of this WaveFormatEx.
    */
   public int getSize()
   {
      return mData.length;
   }
}
