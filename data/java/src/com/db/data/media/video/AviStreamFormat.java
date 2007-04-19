/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media.video;

import java.io.IOException;
import java.io.OutputStream;

import com.db.data.media.RiffChunkHeader;

/**
 * An AVI Stream Format ('strf').
 * 
 * AVI Format is as follows:
 * 
 * AVI Form Header ('RIFF' size 'AVI ' data)
 *    Header List ('LIST' size 'hdrl' data)
 *       AVI Header ('avih' size data)
 *          Video Stream Header List ('LIST' size 'strl' data)
 *             Video Stream Header ('strh' size data)
 *             Video Stream Format ('strf' size data)
 *             Video Stream Data ('strd' size data) - IGNORED, for DRIVERS
 *          Audio Stream Header List ('LIST' size 'strl' data)
 *             Audio Stream Header ('strh' size data)
 *             Audio Stream Format ('strf' size data)
 *             Audio Stream Data ('strd' size data) - IGNORED, for DRIVERS
 *    Info List ('LIST' size 'INFO' data)
 *       Index Entry ({'ISBJ','IART','ICMT',...} size data)
 *    Movie List ('LIST' size 'movi' data)
 *       Movie Entry ({'00db','00dc','01wb'} size data)
 *    Index Chunk ('idx1' size data)
 *       Index Entry ({'00db','00dc','01wb',...})
 * 
 * --------------------------
 * In a Stream Format 'strf':
 * --------------------------
 * A BITMAPINFO structure for a Video Stream Format chunk.
 * A WAVEFORMATEX or PCMWAVEFORMAT structure for an Audio Stream Format chunk.
 * 
 * @author Dave Longley
 */
public class AviStreamFormat
{
   /**
    * The AVI stream format RIFF header.
    */
   protected RiffChunkHeader mRiffHeader;
   
   /**
    * The data for this chunk, not including its header.
    */
   protected byte[] mData;
   
   /**
    * Constructs a new AviStreamFormat.
    */
   public AviStreamFormat()
   {
      // create RIFF header
      mRiffHeader = new RiffChunkHeader("strf");
      
      // create empty data
      mData = new byte[0];
   }
   
   /**
    * Writes this AviStreamFormat, including the RIFF header, to an
    * OutputStream.
    * 
    * @param os the OutputStream to write to.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public void writeTo(OutputStream os) throws IOException
   {
      // write RIFF header
      mRiffHeader.writeTo(os);
      
      // write data
      os.write(mData);
   }
   
   /**
    * Converts this AviStreamFormat from a byte array.
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
      
      // convert the RIFF header
      if(mRiffHeader.convertFromBytes(b, offset, length) && isValid())
      {
         // make sure length has enough data for the chunk
         if(length >= getSize())
         {
            mData = new byte[getChunkSize()];
            System.arraycopy(b, offset + RiffChunkHeader.CHUNK_HEADER_SIZE,
               mData, 0, getChunkSize());
            
            // converted successfully
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Returns whether or not this AviStreamFormat is valid.
    * 
    * @return true if valid, false if not.
    */
   public boolean isValid()
   {
      return mRiffHeader.isValid() &&
         mRiffHeader.getIdentifier().equals("strf");
   }
   
   /**
    * Gets the size of this AviStreamFormat, excluding its chunk header.
    * 
    * @return the size of this AviStreamFormat chunk.
    */
   public int getChunkSize()
   {
      // AVI stream formats are smaller than 32-bits
      return (int)mRiffHeader.getChunkSize();
   }
   
   /**
    * Gets the size of this AviStreamFormat, including its chunk header.
    * 
    * @return the size of this AviStreamFormat.
    */
   public int getSize()
   {
      return getChunkSize() + RiffChunkHeader.CHUNK_HEADER_SIZE;
   }
   
   /**
    * Gets the data in this stream format. This data can be parsed into a
    * BitMapInfo or WaveFormatEx structure.
    * 
    * @return the data in this stream format.
    */
   public byte[] getData()
   {
      return mData;
   }
}
