/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

import java.util.zip.CRC32;

/**
 * An MpegAudioFrame is a single frame with MPEG Audio data.
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
    * The CRC-16 for the frame, if the header's CRC protection bit is set.
    * Otherwise set to 0.
    */
   protected int mCrc16;
   
   /**
    * Stores the audio data for this frame. This is all data that follows the
    * header -- or the CRC-16, if a CRC-16 is used, as the CRC-16 follows
    * the header.
    */
   protected byte[] mAudioData;
   
   /**
    * Creates a new blank MpegAudioFrame.
    */
   public MpegAudioFrame()
   {
      // create a blank header
      mHeader = new MpegAudioFrameHeader();
      
      // no CRC set for this frame yet
      mCrc16 = 0;
      
      // no audio data for this frame yet
      mAudioData = new byte[0];
   }
   
   /**
    * Gets the CRC-16 for this frame based on its current data.
    * 
    * The CRC-16 for a frame is calculated by using the last 2 bytes of
    * the frames header along with a number of bits from the audio data
    * that follows the checksum. The checksum itself must not be included
    * in the calculation.
    * 
    * The number of bits used from the audio data is determined by the
    * layer type.
    * 
    * For Layer I:
    * 
    * The number of bits used is 4 * the number of channels (stereo = 2,
    * mono = 1) * the lowest subband + the number of subbands in use which is
    * just 32 - the lowest subband.
    * 
    * For Layer II:
    * 
    * Unimplemented.
    * 
    * For Layer III:
    * 
    * The number of bits used is the same as the length of the side information.
    * This is 32 bytes for MPEG 1/Stereo, 17 bytes for MPEG 1/Mono,
    * 17 bytes for MPEG 2/2.5/Stereo, and 9 bytes for MPEG 2/2.5/Mono.
    * 
    * @return the calculated CRC.
    */
   protected int calculateCrc()
   {
      int rval = 0;
      
      if(getHeader().isCrcEnabled())
      {
         // CRC is enabled
         
         // determine the number of bits of audio data to use in the CRC
         int audioDataBits = 0;
         
         // get the channel count and lower band
         int channels = getHeader().getChannelCount();
         int lowerBand = getHeader().getJointStereoLowerBand();
         
         // how many bits to use depends on the layer type
         switch(getHeader().getLayer())
         {
            case Layer1: 
               // layer I uses channels and band range
               audioDataBits = 4 * (channels * lowerBand) + (32 - lowerBand);
               break;
            case Layer2:
               // assume CRC is correct, not implemented
               break;
            case Layer3:
               // layer III uses side information length (x8 to get bits)
               audioDataBits = getHeader().getSideInformationLength() * 8;
               break;
         }
         
         if(audioDataBits > 0)
         {
            // determine the number of audio data bytes (round up)
            int audioDataBytes = (int)Math.round(((double)audioDataBits / 8));
            
            // create a CRC-32
            CRC32 crc = new CRC32();
            
            // update the data with the last 2 bytes of the header
            crc.update(getHeader().getBytes(), 2, 2);
            
            // update the data with the audio data bytes
            crc.update(getAudioData(), 0, audioDataBytes);
            
            // AND the CRC-32 value to get the CRC-16
            rval = (int)(crc.getValue() & 0xffff);
         }
      }
      
      return rval;
   }
   
   /**
    * Updates the CRC-16 for this frame based on its current data.
    */
   protected void updateCrc()
   {
      // calculate the CRC
      mCrc16 = calculateCrc();
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
    * Converts this frame, excluding its header, from a byte array. The
    * passed header will be used as the header for this frame. 
    * 
    * @param header the header to use for this frame.
    * @param bytes the byte array to convert this frame (excluding its header)
    *              from.
    * @param offset the offset to start converting from in the passed array.
    * @param length the number of valid bytes in the passed array.
    * 
    * @return true if this frame is valid after converting from the passed
    *         array.
    */
   public boolean convertFromBytes(
      MpegAudioFrameHeader header, byte[] bytes, int offset, int length)
   {
      boolean rval = false;
      
      // set the header for this frame
      mHeader = header;
      
      // if the header is valid, then convert the rest of the frame
      if(getHeader().isValid())
      {
         // determine if a CRC-16 must be read
         if(getHeader().isCrcEnabled())
         {
            // read in the CRC-16, it should be the first 2 bytes
            // of the frame data
            byte b0 = bytes[offset++];
            byte b1 = bytes[offset++];
            
            // the CRC is stored in Big Endian, so most significant byte first
            mCrc16 = (b0 << 8) | b1;
            
            // decrement remaining length
            length -= 2;
         }
         
         // read in the audio data, if there is enough data available
         int audioDataLength = getHeader().getAudioDataLength();
         if(length >= audioDataLength) 
         {
            // allocate enough room for the data
            if(mAudioData.length != audioDataLength) 
            {
               mAudioData = new byte[audioDataLength];
            }
            
            // copy the audio data
            System.arraycopy(bytes, offset, mAudioData, 0, audioDataLength);
         }
      }
      
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
    * Returns true if the CRC-16 for this frame is valid, false if not.
    * 
    * @return true if the CRC-16 for this frame is valid, false if not.
    */
   public boolean checkCrc()
   {
      boolean rval = false;
      
      // calculate the CRC for the current data
      int crc16 = calculateCrc();
      
      // check it against the current CRC
      if(crc16 == getCrc())
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the current CRC-16 value for this frame.
    * 
    * @return the current CRC-16 value for this frame.
    */
   public int getCrc()
   {
      return mCrc16;
   }
   
   /**
    * Gets the audio data for this frame.
    * 
    * @return the audio data for this frame.
    */
   public byte[] getAudioData()
   {
      return mAudioData;
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
