/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media.audio;

import com.db.util.Crc16;

/**
 * This class is used to *correctly* calculate the CRC-16 for MPEG Audio.
 * 
 * The polynomial key used is 0x8005. The starting CRC value is 0xffff.
 * 
 * @author Dave Longley
 */
public class MpegAudioCrc16 extends Crc16
{
   /**
    * Creates a new MpegAudioCrc16.
    */
   public MpegAudioCrc16()
   {
      super(0x8005);
      
      // reset
      reset();
   }
   
   /**
    * Resets the CRC value to 0xffff.
    */
   @Override
   public void reset()
   {
      super.resetValueToMax();
   }
   
   /**
    * Gets the number of audio data bytes required to determine the CRC-16.
    * 
    * The CRC-16 for an MPEG audio frame is calculated by using the last 2
    * bytes of the frames header along with a number of bits from the audio
    * data that follows the checksum. The checksum itself must not be included
    * in the calculation.
    * 
    * The number of bits used from the audio data is determined by the
    * layer type.
    * 
    * For Layer I:
    * 
    * The number of bits used is two times (because stereo is 2 channels) the
    * number of stereo subbands plus the number of mono subbands -- all times 4.
    * 
    * For Layer II:
    * 
    * Unimplemented.
    * 
    * For Layer III:
    * 
    * The bits used are the side information. This is 32 bytes for
    * MPEG 1/Stereo, 17 bytes for MPEG 1/Mono, 17 bytes for MPEG 2/2.5/Stereo,
    * and 9 bytes for MPEG 2/2.5/Mono.
    * 
    * @param header the MpegAudioFrameHeader header to use.
    * 
    * @return the number of audio data bytes required to determine the CRC-16.
    */
   public int getAudioDataAmount(MpegAudioFrameHeader header)
   {
      int rval = 0;
      
      // determine the number of bits of audio data to use in the CRC
      int audioDataBits = 0;
      
      // get the channel count and stereo subband bound
      int channels = header.getChannelCount();
      int bound = header.getJointStereoBound();
      
      // how many bits to use depends on the layer type
      switch(header.getLayer())
      {
         case Layer1: 
            // layer I determines the amount of data to pass through the
            // CRC algorithm by multiplying stereo bands by 2 channels
            // and adding them to mono bands -- then multiplying by 4
            // this algorithm simplifies that calculation ... when
            // no stereo is used the bound is set to 32
            audioDataBits = 4 * (channels * bound + (32 - bound));
            break;
         case Layer2:
            // assume CRC is correct, not implemented
            break;
         case Layer3:
            // layer III uses side information for the CRC (x8 to get bits)
            audioDataBits = header.getSideInformationLength() * 8;
            break;
      }
      
      if(audioDataBits > 0)
      {
         // determine the number of audio data bytes (round up)
         rval = (int)Math.round(((double)audioDataBits / 8));
      }
      
      return rval;      
   }
   
   /**
    * Calculates the CRC-16 for this frame given the passed audio data bytes.
    * The appropriate number of audio data bytes can be determined by calling
    * getAudioDataAmount(header).
    * 
    * The given MpegAudioFrameHeader data will be used in the CRC calculation.
    * 
    * @param header the MpegAudioFrameHeader to use.
    * @param audioData the buffer with audio data.
    * @param offset the offset at which the audio data begins.
    * 
    * @return the calculated CRC.
    */
   public int calculateCrc(
      MpegAudioFrameHeader header, byte[] audioData, int offset)
   {
      // reset the CRC value
      reset();
      
      // update the CRC with the last 2 header bytes 
      update(header.getBytes(), 2, 2);
      
      // update the CRC with the audio data amount
      update(audioData, offset, getAudioDataAmount(header));
      
      // return the CRC value
      return getValue();
   }
}
