/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

/**
 * An MpegAudioFrame is a single frame with MPEG audio data.
 * 
 * MPEG Audio data is made up of frames. An MpegAudioFrame represents a single
 * frame. Each frame uses a *fixed* number of samples. Each frame contains a
 * header that is 4 bytes in length, (optionally) a CRC-16, and audio data. 
 *
 * @author Dave Longley
 */
public class MpegAudioFrame
{
   /**
    * This frame's header.
    */
   protected MpegAudioFrameHeader mHeader;
   
   /**
    * Creates a new blank MpegAudioFrame.
    */
   public MpegAudioFrame()
   {
      // create a blank header
      mHeader = new MpegAudioFrameHeader();
   }
   
   /**
    * Converts this frame from a byte array.
    * 
    * @param bytes the byte array to convert this frame from.
    * @param offset the offset to start converting from in the passed array.
    * @param length the number of valid bytes in the passed array.
    * 
    * @return true if this frame is valid after converteing from the passed
    *         array.
    */
   public boolean convertFromBytes(byte[] bytes, int offset, int length)
   {
      boolean rval = false;
      
      // FIXME:
      
      return rval;
   }
   
   /**
    * Gets the header for this frame.
    * 
    * @return the header for this frame.
    */
   public MpegAudioFrameHeader getHeader()
   {
      return mHeader;
   }
   
   /**
    * Gets the bytes for this frame.
    * 
    * @return the bytes for this frame.
    */
   public byte[] getBytes()
   {
      byte[] rval = new byte[0];
      
      // FIXME:
      
      return rval;
   }
}
