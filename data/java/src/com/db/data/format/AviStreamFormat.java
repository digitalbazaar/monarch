/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

import java.io.IOException;
import java.io.OutputStream;

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
      
      // write stream data
      os.write(mData);
   }   
   
   /**
    * Returns whether or not this AviStreamFormat is valid.
    * 
    * @return true if valid, false if not.
    */
   public boolean isValid()
   {
      return mRiffHeader.isValid();
   }
   
   /**
    * Gets the size of this AviStreamFormat, not including its chunk header.
    * 
    * @return the size of this AviStreamFormat.
    */
   public long getSize()
   {
      return mRiffHeader.getChunkSize();
   }
}
