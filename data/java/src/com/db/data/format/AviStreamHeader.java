/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

import java.io.IOException;
import java.io.OutputStream;

/**
 * An AVI Stream Header ('strh').
 * 
 * AVI Format is as follows:
 * 
 * AVI Form Header ('RIFF' size 'AVI ' data)
 *    Header List ('LIST' size 'hdrl' data)
 *       AVI Header ('avih' size data)
 *       Video Stream Header List ('LIST' size 'strl' data)
 *          Video Stream Header ('strh' size data)
 *          Video Stream Format ('strf' size data)
 *          Video Stream Data ('strd' size data) - IGNORED, for DRIVERS
 *       Audio Stream Header List ('LIST' size 'strl' data)
 *          Audio Stream Header ('strh' size data)
 *          Audio Stream Format ('strf' size data)
 *          Audio Stream Data ('strd' size data) - IGNORED, for DRIVERS
 *    Info List ('LIST' size 'INFO' data)
 *       Index Entry ({'ISBJ','IART','ICMT',...} size data)
 *    Movie List ('LIST' size 'movi' data)
 *       Movie Entry ({'00db','00dc','01wb'} size data)
 *    Index Chunk ('idx1' size data)
 *       Index Entry ({'00db','00dc','01wb',...})
 * 
 * ---------------------------------------------------------------
 * In a Stream Header 'strh' (FOURCC means a four-character code):
 * ---------------------------------------------------------------
 * FOURCC type - 'vids' for a video stream, 'auds' for an audio stream
 * FOURCC handler - the installable compressor or decompressor for the data
 * DWORD flags
 * DWORD reserved
 * DWORD initial frames
 * DWORD scale
 * DWORD rate
 * DWORD start
 * DWORD length
 * DWORD suggested buffer size
 * DWORD quality
 * DWORD sample size
 * 
 * The flags for the Stream Header:
 * 
 * AVISF_DISABLED - whether or not the data should only be rendered when
 * explicitly enabled by the user
 * 
 * AVISF_VIDEO_PALCHANGES - whether or not palette changes are embedded
 * in the file (chunks tagged like '00pc')
 * 
 * @author Dave Longley
 */
public class AviStreamHeader
{
   /**
    * The AVI stream header RIFF header.
    */
   protected RiffChunkHeader mRiffHeader;
   
   /**
    * The data for this chunk, not including its header.
    */
   protected byte[] mData;
   
   /**
    * Constructs a new AviStreamHeader.
    */
   public AviStreamHeader()
   {
      // create RIFF header
      mRiffHeader = new RiffChunkHeader("strh");
   }
   
   /**
    * Writes this AviStreamHeader, including the RIFF header, to an
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
    * Returns whether or not this AviStreamHeader is valid.
    * 
    * @return true if valid, false if not.
    */
   public boolean isValid()
   {
      return mRiffHeader.isValid();
   }
   
   /**
    * Gets the size of this AviStreamHeader, not including its chunk header.
    * 
    * @return the size of this AviStreamHeader.
    */
   public long getSize()
   {
      return mRiffHeader.getChunkSize();
   }
}
