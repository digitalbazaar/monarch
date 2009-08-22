/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media.video;

import java.io.IOException;
import java.io.OutputStream;

import com.db.data.UnsignedBinaryIO;
import com.db.data.media.RiffChunkHeader;

/**
 * An AVI main header ('avih').
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
 * -------------------------------------------
 * In the AVI Header
 * (a DWORD is 4 bytes, 14 DWORDS = 56 bytes):
 * -------------------------------------------
 * DWORD microseconds per frame
 * DWORD maximum bytes per second
 * DWORD padding granularity
 * DWORD flags
 * DWORD total frames
 * DWORD initial frames (how far in front audio data is of video data)*
 * DWORD streams - 2 for video+audio streams
 * DWORD suggested buffer size - large enough to contain largest chunk
 * DWORD width - in pixels
 * DWORD height - in pixels
 * DWORD scale - time scale in samples per second = rate / scale
 * DWORD rate - see scale
 * DWORD start - starting time for the AVI file, usually zero
 * DWORD length - total time of the file using rate & scale units
 * 
 * The flags for the AVI Header 'avih':
 * 
 * AVIF_HASINDEX (bit 4) - indicates whether or not the AVI has an Index Chunk
 * 
 * AVIF_MUSTUSEINDEX (bit 5) - indicates whether or not the index should
 * determine the order of the data
 * 
 * AVIF_ISINTERLEAVED (bit 8) - indicates whether or not the file is
 * interleaved
 * 
 * AVIF_WASCAPTUREFILE (bit 16) - indicates whether or not the file is used for
 * capturing real-time video
 * 
 * AVIF_COPYRIGHTED (bit 17) - indicates whether or not the file contains
 * copyrighted data
 * 
 * *Note: The initial frames data member specifies the number of frames
 * the audio data is ahead of the video data. Typically, audio data should
 * be moved forward enough frames to allow 0.75 seconds of audio data to
 * be preloaded before the video data is loaded. This is only for interleaved
 * files -- this should be ZERO for non-interleaved files.
 * 
 * @author Dave Longley
 */
public class AviHeader
{
   /**
    * The AVI header RIFF header.
    */
   protected RiffChunkHeader mRiffHeader;
   
   /**
    * The data for this chunk, not including its header.
    */
   protected byte[] mData;
   
   /**
    * Indicates whether or not the AVI has an Index Chunk.
    */
   public static final int AVIF_HASINDEX = 0x00000010;
   
   /**
    * Indicates whether or not the index should determine the order of
    * the data.
    */
   public static final int AVIF_MUSTUSEINDEX = 0x00000020;
   
   /**
    * Indicates whether or not the file is interleaved.
    */
   public static final int AVIF_ISINTERLEAVED = 0x00000100;
   
   /**
    * Indicates whether or not the file is used for capturing real-time video.
    */
   public static final int AVIF_WASCAPTUREFILE = 0x00010000;
   
   /**
    * Indicates whether or not the file contains copyrighted data.
    */
   public static final int AVIF_COPYRIGHTED = 0x00020000;
   
   /**
    * Constructs a new AviHeader.
    */
   public AviHeader()
   {
      mRiffHeader = new RiffChunkHeader("avih");
      
      // create data for this empty header
      mData = new byte[56];
   }
   
   /**
    * Reads a DWORD from the internal data and discards an IO error.
    *
    * @param offset the offset to read from.
    * 
    * @return the read DWORD.
    */
   protected long readDWord(int offset)
   {
      long rval = 0;
      
      try
      {
         rval = UnsignedBinaryIO.readUnsignedInt(
            mData, offset, mData.length - offset);
      }
      catch(IOException ignore)
      {
         // bad header
      }
      
      return rval;
   }
   
   /**
    * Writes this AviHeader, including the RIFF header, to an OutputStream.
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
    * Converts this AviHeader from a byte array.
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
      if(mRiffHeader.convertFromBytes(b, offset, length) &&
         mRiffHeader.getIdentifier().equals("avih"))
      {
         // make sure length has enough data for the chunk
         if(length >= getSize() && getSize() == 64)
         {
            System.arraycopy(b, offset + RiffChunkHeader.CHUNK_HEADER_SIZE,
               mData, 0, getSize() - RiffChunkHeader.CHUNK_HEADER_SIZE);
            
            // converted successfully
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Returns whether or not this header is valid.
    * 
    * @return true if valid, false if not.
    */
   public boolean isValid()
   {
      return mRiffHeader.isValid() &&
         mRiffHeader.getIdentifier().equals("avih");
   }
   
   /**
    * Gets the size of this AviHeader, excluding its chunk header.
    * 
    * @return the size of this AviHeader chunk.
    */
   public int getChunkSize()
   {
      // AVI header chunk size is 56 bytes      
      return (int)mRiffHeader.getChunkSize();
   }
   
   /**
    * Gets the size of this AviHeader including its chunk header.
    * 
    * @return the size of this AviHeader.
    */
   public int getSize()
   {
      return getChunkSize() + RiffChunkHeader.CHUNK_HEADER_SIZE;
   }
   
   /**
    * Gets the number of microseconds per frame.
    * 
    * @return the number of microseconds per frame.
    */
   public long getMicrosecondsPerFrame()
   {
      return readDWord(0);
   }
   
   /**
    * Gets the maximum number of bytes per second.
    * 
    * @return the maximum number of bytes per second.
    */
   public long getMaxBytesPerSecond()
   {
      return readDWord(4);
   }
   
   /**
    * Gets the padding granularity (typically 2048). Data should be padded
    * to multiples of this value.
    * 
    * @return the padding granularity (typically 2048).
    */
   public long getPaddingGranularity()
   {
      return readDWord(8);
   }
   
   /**
    * Returns true if the AVIF_HASINDEX flag is on.
    * 
    * @return true if the AVIF_HASINDEX flag is on, false if not.
    */
   public boolean isAviHasIndex()
   {
      boolean rval = false;
      
      long flags = readDWord(12);
      if((flags & AVIF_HASINDEX) == AVIF_HASINDEX)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns true if the AVIF_MUSTUSEINDEX flag is on.
    * 
    * @return true if the AVIF_MUSTUSEINDEX flag is on, false if not.
    */
   public boolean isAviMustUseIndex()
   {
      boolean rval = false;
      
      long flags = readDWord(12);
      if((flags & AVIF_MUSTUSEINDEX) == AVIF_MUSTUSEINDEX)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns true if the AVIF_ISINTERLEAVED flag is on.
    * 
    * @return true if the AVIF_ISINTERLEAVED flag is on, false if not.
    */
   public boolean isAviIsInterleaved()
   {
      boolean rval = false;
      
      long flags = readDWord(12);
      if((flags & AVIF_ISINTERLEAVED) == AVIF_ISINTERLEAVED)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns true if the AVIF_WASCAPTUREFILE flag is on.
    * 
    * @return true if the AVIF_WASCAPTUREFILE flag is on, false if not.
    */
   public boolean isAviWasCaptureFile()
   {
      boolean rval = false;
      
      long flags = readDWord(12);
      if((flags & AVIF_WASCAPTUREFILE) == AVIF_WASCAPTUREFILE)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Returns true if the AVIF_COPYRIGHTED flag is on.
    * 
    * @return true if the AVIF_COPYRIGHTED flag is on, false if not.
    */
   public boolean isAviCopyrighted()
   {
      boolean rval = false;
      
      long flags = readDWord(12);
      if((flags & AVIF_COPYRIGHTED) == AVIF_COPYRIGHTED)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the total frames.
    * 
    * @return the total frames.
    */
   public long getTotalFrames()
   {
      return readDWord(16);
   }
   
   /**
    * Gets the initial frames.
    * 
    * @return the initial frames.
    */
   public long getInitialFrames()
   {
      return readDWord(20);
   }
   
   /**
    * Gets the number of streams.
    * 
    * @return the number of streams.
    */
   public long getStreamCount()
   {
      return readDWord(24);
   }
   
   /**
    * Gets the suggested playback buffer size in bytes.
    * 
    * @return the suggested playback buffer size in bytes.
    */
   public long getSuggestedBufferSize()
   {
      return readDWord(28);
   }
   
   /**
    * Gets the width in pixels.
    * 
    * @return the width in pixels.
    */
   public long getWidth()
   {
      return readDWord(32);
   }
   
   /**
    * Gets the height in pixels.
    * 
    * @return the height in pixels.
    */
   public long getHeight()
   {
      return readDWord(36);
   }
   
   /**
    * Gets the time scale (typically 30).
    * 
    * @return the time scale (typically 30).
    */
   public long getTimeScale()
   {
      return readDWord(40);
   }
   
   /**
    * Gets the data rate.
    * 
    * @return the data rate.
    */
   public long getDataRate()
   {
      return readDWord(44);
   }
   
   /**
    * Gets the frame rate (data rate / time scale).
    * 
    * @return the frame rate (data rate / time scale).
    */
   public double getFrameRate()
   {
      double rval = 0.0;
      
      if(getTimeScale() != 0)
      {
         rval = (double)getDataRate() / getTimeScale();
      }
      
      return rval;
   }
   
   /**
    * Gets the starting time in time scale units (typically 0).
    * 
    * @return the starting time in time scale units (typically 0).
    */
   public long getStartingTime()
   {
      return readDWord(48);
   }
   
   /**
    * Gets the video length in time scale units.
    * 
    * @return the video length in time scale units.
    */
   public long getVideoLength()
   {
      return readDWord(52);
   }
   
   /**
    * Gets the time as calculated from microseconds/frame * total frames.
    * 
    * @return the time as calculated from microseconds/frame * total frames.
    */
   public long getTotalTime()
   {
      return getMicrosecondsPerFrame() * getTotalFrames();
   }
   
   /**
    * Gets the time, in seconds, as calculated from microseconds/frame * total
    * frames.
    * 
    * @return the time, in seconds, as calculated from microseconds/frame *
    *         total frames.
    */
   public double getTotalSeconds()
   {
      double rval = 0.0;
      
      if(getTotalFrames() != 0)
      {
         rval = getMicrosecondsPerFrame() / 1000000D;
         rval *= getTotalFrames();
      }
      
      return rval;
   }
}
