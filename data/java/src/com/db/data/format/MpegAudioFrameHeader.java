/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

import java.util.EnumSet;
import java.util.HashMap;

/**
 * An MpegAudioFrameHeader is a header for an MpegAudioFrame.
 * 
 * MPEG Audio data is made up of frames. An MpegAudioFrame represents a single
 * frame. Each frame uses a *fixed* number of samples. Each frame contains a
 * header that is 4 bytes in length. This class represents that header.
 * 
 * The header contains information about the frame including, but not limited
 * to, its version, its layer type, whether or not it is protected by a CRC,
 * its bitrate (which INCLUDES the header!), its sampling rate, and whether
 * or not the frame uses padding.
 * 
 * Each frame header always begins with a series of set bits (set to '1')
 * that are used for "synchronization." They are referred to as "frame sync" as
 * they are meant to help determine the beginning of an MPEG Audio frame.
 * 
 * For MPEG versions 1 and 2, the first 12 bits of the frame are always set.
 * 
 * MPEG version 2.5 has only the first 11 bits of a frame set. This header
 * implementation supports MPEG versions 1, 2, and 2.5, therefore it "syncs"
 * frames by checking only the first 11 bits.
 * 
 * The MPEG version follows the frame sync bits as the next bit (bits 12 and
 * 13 counting from the left, bits 20 and 19 counting from the right). After
 * the version the MPEG Layer is set.
 * 
 * All 3 versions of MPEG frames can use either Layer I, Layer II, or Layer III.
 * 
 * The difference between the layers is in how the audio data is encoded and
 * which combinations of bitrate and mode are permitted.
 * 
 * The data in MPEG Audio frames is broken up into "slots." Layer I slots
 * are always 4 bytes in length. Layers II & III slots are always 1 byte in
 * length.
 * 
 * Layer I always uses 384 samples per frame. Layer II always uses 1152 samples
 * per frame. Layer III uses 1152 samples per frame with MPEG Version 1 and
 * 576 (a Lower Sampling Frequency LSF) with MPEG Version 2 and 2.5.
 * 
 * In order to calculate the length of a frame in bytes, one must know the
 * bitrate (bits/second), the sampling rate (samples/second), the number of
 * samples in the frame (specified by the frame's Version and Layer type),
 * the size of the slot (also specified by the Layer type), and whether or not
 * the frame uses padding.
 * 
 * If the frame uses padding, then an one extra slot is added to the frame to
 * ensure that the bitrate is accurate (more data must be present such that
 * when playing the audio, the number of bits specified pass in the appropriate
 * period of time so that the audio doesn't play too slowly or too quickly
 * with respect to the actual audio data).
 * 
 * Therefore, to calculate the length of a frame in bytes, the following
 * algorithm is used:
 * 
 * bits/second * samples/second = bits/sample
 * bits/sample / 8 = bytes/sample
 * bytes/sample * number of samples = number of bytes for samples
 * number of bytes for samples + padding * slot size = total bytes in the frame
 * 
 * The slot size for Layer I is always 4 bytes.
 * 
 * The slot size for Layers II & III is always 1 byte.
 * 
 * Now, the total number of bytes in the frame may not be a whole number.
 * According to the ISO standards, the number should be truncated (that is,
 * rounded DOWN) to the nearest whole number. So, the actual total number
 * of bytes in a frame would be:
 * 
 * floor(total bytes in the frame)
 * 
 * Keep in mind that the bitrate INCLUDES THE HEADER so so does the above
 * calculation. 
 * 
 * All of the above components can be found in the frame header. The header
 * provides the bitrate (bits/second), sampling rate (samples/second), samples
 * per frame, the slot size, and whether or not padding is used.
 * 
 * Therefore, the total frame length for a frame can be calculated from just
 * the information provided in the header.
 * 
 * Note: The sampling rate is provided in Hz. One Hertz (Hz) is one complete
 * cycle of a sound wave per second. A sample is one cycle, so Hz is the same
 * as samples per second.
 * 
 * The formula, as produced from the above information, for determining the
 * frame length is as follows:
 * 
 * Layer I frame length in bytes:
 * 
 * 384 * (bitrate / samplingRate / 8) + padding * 4
 * 
 * This can be simplified ((by dividing by 8, then factoring out 4) to:
 * 
 * (12 * (bitrate / samplingRate) + padding) * 4
 * 
 * Layer II frame length in bytes:
 * 
 * 1152 * (bitrate / samplingRate / 8) + padding * 1
 * 
 * This can be simplified to (by dividing by 8):
 * 
 * 144 * (bitrate / samplingRate) + padding
 * 
 * Layer III, MPEG Version 1 frame length in bytes:
 * 
 * 144 * (bitrate / samplingRate) + padding
 * 
 * Layer III, MPEG Version 2 and 2.5 frame length in bytes
 * (using 576 samples/second):
 * 
 * 72 * (bitrate / samplingRate) + padding
 * 
 * --------------------------------------------------------------------------
 * The format for an MPEG Audio frame header is as follows:
 * 
 * | byte 0 | byte 1 | byte 2 | byte 3 |
 * 
 *  AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM
 * 
 * A: Frame sync
 * 1 - Always. There are 11 bits.
 * 
 * BB: MPEG Version
 * 00 - MPEG 2.5
 * 01 - reserved
 * 10 - MPEG 2
 * 11 - MPEG 1
 * 
 * CC: MPEG Layer Type
 * 00 - reserved
 * 01 - Layer III
 * 10 - Layer II
 * 11 - Layer I
 * 
 * D: CRC-16 Protection bit
 * 0 - The frame data (some portion of it is used as explained in
 *     MpegAudioFrame) is protected by a 16 bit CRC. The location of the CRC
 *     in the frame is outlined in MpegAudioFrame.
 * 1 - Not Protected. How this affects the frame format is outlined in
 *     MpegAudioFrame.
 * 
 * Note: This is not a mistake (at least not by me :P) -- CRC is ON when the
 * bit is NOT set (value of 0) and CRC is OFF when the bit is set. Why this
 * was done this way is not known to me (Dave Longley) at the time of this
 * writing, as it seems a tad bit backwards. I can only assume there was a
 * good reason for it.
 * 
 * EEEE: Bitrate Index - an index into a Bitrate Table.
 * See the BitrateTable subclass.
 * 
 * Note: If MPEG Audio data has a variable bitrate (VBR) then the bitrate may
 * vary from frame to frame.
 * 
 * Note: Layer II only permits some combinations of bitrate and channel mode.
 * The combinations it permits are listed below:
 * 
 * bitrate  allowed modes
 * free     all
 * 32       single channel
 * 48       single channel
 * 56       single channel
 * 64       all
 * 80       single channel
 * 96       all
 * 112      all
 * 128      all
 * 160      all
 * 192      all
 * 224      stereo, intensity stereo, dual channel
 * 256      stereo, intensity stereo, dual channel
 * 320      stereo, intensity stereo, dual channel
 * 384      stereo, intensity stereo, dual channel
 * 
 * FF: Sampling Rate Index - an index into a Sampling Rate Table.
 * See the SamplingRateTable subclass.
 * 
 * G: Padding bit
 * 0 - Frame is not padded. There is no extra slot for this frame.
 * 1 - Frame is padded. There is one extra slot for this frame. This padding
 *     is used to ensure bitrates are matched properly.
 * 
 * H: Private bit. This bit can be used by an application for its own purposes.
 * 0 - Application specific.
 * 1 - Application specific.
 * 
 * II: The Channel Mode.
 * 00 - Stereo (Stereo)
 * 01 - Joint Stereo (Stereo)
 * 10 - Dual Channel (Stereo)
 * 11 - Single Channel (Mono)
 * 
 * JJ: Channel Mode Extension (used only for Joint Stereo Channel Mode).
 * 
 * These bits are dynamically determined by an encoder using Joint Stereo
 * Channel Mode.
 * 
 * For Layers I & II the frequency range of the MPEG data is divided into 32
 * subbands. These two bits (JJ) determine where intensity stereo is applied
 * like so:
 * 
 * Layers I & II
 * bits  band range
 * 00    4 - 31
 * 01    8 - 31
 * 10    12 - 31
 * 11    16 - 31
 * 
 * For Layer III these two bits (JJ) determine which type of joint stereo
 * is used (intensity stereo or m/s stereo). The frequency range is determined
 * by the decompression algorithm.
 * 
 * Layer III
 * bits Intensity Stereo  MS Stereo
 * 00   off               off
 * 01   on                off
 * 10   off               on
 * 11   on                on
 * 
 * K: Copyright bit - Informational only.
 * 0 - Audio is not copyrighted.
 * 1 - Audio is copyrighted.
 * 
 * L: Original bit - Informational only.
 * 0 - Audio is a copy of the original media.
 * 1 - Audio is the original media.
 * 
 * MM: Emphasis. Used to tell the decoder to "re-equalize" sound after
 *     sound suppression. This is rarely used.
 * 00 - none
 * 01 - 50/15 ms
 * 10 - reserved
 * 11 - CCIT J.17
 * 
 * Some MPEG Audio information obtained from:
 * http://www.codeproject.com/audio/MPEGAudioInfo.asp
 * 
 * @author Dave Longley
 */
public class MpegAudioFrameHeader
{
   /**
    * The header data as a byte array.
    */
   protected byte[] mData;
   
   /**
    * A static bitrate table used for all frame headers.
    */
   protected static BitrateTable smBitrateTable =
      new BitrateTable();
   
   /**
    * A static sampling rate table used for all frame headers.
    */
   protected static SamplingRateTable smSamplingRateTable =
      new SamplingRateTable();
   
   /**
    * Creates a new blank MpegAudioFrameHeader.
    */
   public MpegAudioFrameHeader()
   {
   }
   
   /**
    * Converts this header from an array of bytes. The passed length must be
    * at least 4 bytes, which is the required size of an MpegAudioFrameHeader.
    * 
    * @param bytes the array of bytes to convert this header from.
    * @param offset the offset in the passed array to start converting from.
    * @param length the number of valid bytes in the passed array.
    * 
    * @return true if this header is valid after converting from the
    *         passed array.
    * 
    * @exception IllegalArgumentException thrown if the passed length is less
    * than 4 (the required header size).
    */
   public boolean convertFromBytes(byte[] bytes, int offset, int length)
   {
      boolean rval = false;
      
      if(length < 4)
      {
         throw new IllegalArgumentException("Length must be >= 4");
      }
      
      // get the header bytes
      byte[] data = getBytes();
      
      // copy the first 4 bytes from the array
      System.arraycopy(bytes, offset, data, 0, 4);
      
      // determine if this header is valid
      rval = isValid();
      
      return rval;
   }
   
   /**
    * Determines if this header is valid or not.
    * 
    * @return true if this header is valid, false if not.
    */
   public boolean isValid() 
   {
      boolean rval = false;
      
      // this header is valid if it has frame sync and its version,
      // layer, bitrate, and sampling rate are valid
      if(hasFrameSync() && isVersionValid() && isLayerValid() &&
         isBitrateValid() && isSamplingRateValid())
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Determines if this header has frame sync. This means that its first
    * 11 bits are set.
    * 
    * @return true if this header has its first 11 bits set, false if not.
    */
   public boolean hasFrameSync()
   {
      boolean rval = false;
      
      // get the header bytes
      byte[] data = getBytes();
      
      // the first 11 bits must be set to 1 for "frame sync":
      // 1111111111xxxx
      // 1111111 = 255
      // 111xxxx = at least 224 (greater than 223)
      // 255 = 0xFF, 224 = 0xe0
      if(data[0] == (byte)255 && data[1] > (byte)223)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the version for the frame.
    * 
    * @return the version for the frame.
    */
   public Version getVersion()
   {
      // get the header bytes
      byte[] data = getBytes();
      
      // the version is located in bits 4 and 3 for byte 1, so shift
      // to the right 3 and then AND with 3
      // with 00011000 >> 3 = 00000011 & 0x03 = 11
      byte version = (byte)((data[1] >> 3) & 0x03);
      
      // return the appropriate version
      return Version.getVersion(version);
   }
   
   /**
    * Determines if this header has a valid MPEG version or not.
    * 
    * @return true if this header has a valid MPEG version, false if not.
    */
   public boolean isVersionValid()
   {
      boolean rval = false;
      
      // any version other than reserved is valid
      if(getVersion() != Version.Reserved)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the layer for the frame.
    * 
    * @return the layer for the frame.
    */
   public Layer getLayer()
   {
      // get the header bytes
      byte[] data = getBytes();
      
      // the layer is located in bits 2 and 1 for byte 1, so shift
      // to the right 1 and then AND with 1
      // with 00000110 >> 1 = 00000011 & 0x03 = 11
      byte layer = (byte)((data[1] >> 1) & 0x03);
      
      // return the appropriate layer
      return Layer.getLayer(layer);
   }
   
   /**
    * Determines if this header has a valid MPEG layer type or not.
    * 
    * @return true if this header has a valid MPEG layer type, false if not.
    */
   public boolean isLayerValid()
   {
      boolean rval = false;
      
      // any layer other than reserved is valid
      if(getLayer() != Layer.Reserved)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Sets whether or not CRC-16 protection is enabled for the frame.
    * 
    * @param enabled true to enable CRC-16 protection for the frame, false
    *                to disable it.
    */
   public void setCrcEnabled(boolean enabled)
   {
      // get the header bytes
      byte[] data = getBytes();
      
      // the crc protection bit is located in bit 0 for byte 1,
      // if enabling, then CLEAR bit 0 by ANDing with 0xFE,
      // if disabling, then SET bit 0 by ORing with 0x01
      if(enabled)
      {
         data[1] &= 0xFE;
      }
      else
      {
         data[1] |= 0x01;
      }
   }
   
   /**
    * Determines whether or not CRC-16 protection is enabled for the frame.
    * 
    * @return true if CRC-16 protection is enabled for the frame, false if not.
    */
   public boolean isCrcEnabled()
   {
      boolean rval = false;
      
      // get the header bytes
      byte[] data = getBytes();
      
      // the crc protection bit is located in bit 0 for byte 1, so
      // AND against 1
      byte protection = (byte)(data[1] & 0x01);
      
      // protection is enabled if the bit is NOT set (it IS cleared)
      rval = (protection == 0);
      
      return rval;
   }
   
   /**
    * Gets the bitrate for this frame in bits/second.
    * 
    * @return the bitrate for this frame in bits/second with 0 indicating
    *         a free format bitrate and -1 indicating an invalid bitrate.
    */
   public int getBitrate()
   {
      // get the header bytes
      byte[] data = getBytes();
      
      // the bitrate index is located in bits 7-4 for byte 2, so shift
      // to the right 4 and then AND with 0x0F
      // with 11110000 >> 4 = 00001111 & 0x0F = 1111
      byte bitrateIndex = (byte)((data[2] >> 4) & 0x0F);
      
      // get the bitrate from the bitrate table
      return smBitrateTable.getBitrate(bitrateIndex, getVersion(), getLayer());
   }
   
   /**
    * Determines if this header has a valid bitrate or not.
    * 
    * @return true if this header has a valid bitrate, false if not.
    */
   public boolean isBitrateValid()
   {
      boolean rval = false;
      
      // bitrate is valid if it is not -1
      if(getBitrate() != -1)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Gets the sampling rate for this frame in samples/second.
    * 
    * @return the sampling rate for this frame in samples/second with -1
    *         indicating an invalid sampling rate.
    */
   public int getSamplingRate()
   {
      // get the header bytes
      byte[] data = getBytes();
      
      // the sampling rate index is located in bits 3-2 for byte 2, so shift
      // to the right 2 and then AND with 0x03
      // with 00001100 >> 2 = 00000011 & 0x03 = 11
      byte samplingRateIndex = (byte)((data[2] >> 2) & 0x03);
      
      // get the sampling rate from the sampling rate table
      return smSamplingRateTable.getSamplingRate(
         samplingRateIndex, getVersion());
   }
   
   /**
    * Determines if this header has a valid sampling rate or not.
    * 
    * @return true if this header has a valid sampling rate, false if not.
    */
   public boolean isSamplingRateValid()
   {
      boolean rval = false;
      
      // sampling rate is valid if it is not -1
      if(getSamplingRate() != -1)
      {
         rval = true;
      }
      
      return rval;
   }
   
   /**
    * Sets whether or not the frame is padded.
    * 
    * @param padded true to mark the frame as padded, false to mark it as
    *               unpadded.
    */
   public void setPadded(boolean padded)
   {
      // get the header bytes
      byte[] data = getBytes();
      
      // the padding bit is located in bit 1 for byte 2,
      // if enabling, then SET bit 1 by ORing with 0x02,
      // if disabling, then CLEAR bit 1 by ANDing with 0xFD 
      if(padded)
      {
         data[2] |= 0x02;
      }
      else
      {
         data[2] &= 0xFD;
      }
   }
   
   /**
    * Determines whether or not the frame is padded.
    * 
    * @return true if the frame is padded, false if not.
    */
   public boolean isPadded()
   {
      boolean rval = false;
      
      // get the header bytes
      byte[] data = getBytes();
      
      // the padding bit is located in bit 1 for byte 2, so
      // AND against 2
      byte padding = (byte)(data[2] & 0x02);
      
      // padding is enabled if the bit is NOT cleared
      rval = (padding != 0);
      
      return rval;
   }
   
   /**
    * Sets whether or not the private bit is set for the frame.
    * 
    * @param set true to set the private bit for the frame, false to clear it.
    */
   public void setPrivateBit(boolean set)
   {
      // get the header bytes
      byte[] data = getBytes();
      
      // the private bit is located in bit 0 for byte 2,
      // if enabling, then SET bit 1 by ORing with 0x01,
      // if disabling, then CLEAR bit 1 by ANDing with 0xFE 
      if(set)
      {
         data[2] |= 0x01;
      }
      else
      {
         data[2] &= 0xFE;
      }
   }
   
   /**
    * Determines whether or not the private bit is set for the frame.
    * 
    * @return true if the private bit is set for the frame, false if not.
    */
   public boolean isPrivateBitSet()
   {
      boolean rval = false;
      
      // get the header bytes
      byte[] data = getBytes();
      
      // the private bit is located in bit 0 for byte 2, so
      // AND against 1
      byte privateBit = (byte)(data[2] & 0x01);
      
      // privateBit is enabled if the bit is NOT cleared
      rval = (privateBit != 0);
      
      return rval;
   }   
   
   /**
    * Gets the channel mode for the frame.
    * 
    * @return the channel mode for the frame.
    */
   public ChannelMode getChannelMode()
   {
      // get the header bytes
      byte[] data = getBytes();
      
      // the channel mode is located in bits 7 and 6 for byte 3, so shift
      // to the right 6 and then AND with 0x03
      // with 11000000 >> 6 = 00000011 & 0x03 = 11
      byte cm = (byte)((data[3] >> 6) & 0x03);
      
      // return the appropriate channel mode
      return ChannelMode.getChannelMode(cm);
   }
   
   /**
    * Gets the number of channels used in the frame.
    * 
    * @return the number of channels used in the frame.
    */
   public int getChannelCount()
   {
      // get the channel mode
      ChannelMode cm = getChannelMode();
      
      // return the number of channels
      return cm.channelCount();
   }
   
   /**
    * Gets the channel mode extension for the frame.
    * 
    * @return the channel mode extension for the frame.
    */
   public ChannelModeExtension getChannelModeExtension()
   {
      // get the header bytes
      byte[] data = getBytes();
      
      // the channel mode extension is located in bits 5 and 4 for byte 3, so
      // shift to the right 4 and then AND with 0x03
      // with 00110000 >> 4 = 00000011 & 0x03 = 11
      byte extension = (byte)((data[3] >> 4) & 0x03);
      
      // return the appropriate channel mode extension
      return ChannelModeExtension.getChannelModeExtension(
         extension, getLayer());      
   }
   
   /**
    * Gets the lowest subband the joint stereo is bound to. This value is
    * required for CRC-16 calculation for Layer I frames.
    * 
    * @return the Joint Stereo subband lower bound.
    */
   public int getJointStereoLowerBand()
   {
      // get the channel mode extension
      ChannelModeExtension extension = getChannelModeExtension();
      
      // return the lower band
      return extension.lowerBand();
   }
   
   /**
    * Sets whether or not the frame is marked as copyrighted.
    * 
    * @param copyrighted true to mark this frame as copyrighted, false to mark
    *                    it as not copyrighted.
    */
   public void setCopyrighted(boolean copyrighted)
   {
      // get the header bytes
      byte[] data = getBytes();
      
      // the copyright bit is located in bit 3 for byte 3,
      // if enabling, then SET bit 3 by ORing with 0x08,
      // if disabling, then CLEAR bit 3 by ANDing with 0xF7
      if(copyrighted)
      {
         data[3] |= 0x08;
      }
      else
      {
         data[3] &= 0xF7;
      }
   }
   
   /**
    * Determines whether or not the frame is marked as copyrighted.
    * 
    * @return true if the frame is marked as copyrighted, false if not.
    */
   public boolean isCopyrighted()
   {
      boolean rval = false;
      
      // get the header bytes
      byte[] data = getBytes();
      
      // the copyright bit is located in bit 3 for byte 3, so
      // AND against 8
      byte copyrighted = (byte)(data[3] & 0x08);
      
      // is copyrighted if the bit is NOT cleared
      rval = (copyrighted != 0);
      
      return rval;
   }
   
   /**
    * Sets whether or not the frame is marked as an original.
    *  
    * @param original true to mark this frame as original, false to mark
    *                 it as not original.
    */
   public void setOriginal(boolean original)
   {
      // get the header bytes
      byte[] data = getBytes();
      
      // the original bit is located in bit 2 for byte 3,
      // if enabling, then SET bit 2 by ORing with 0x04,
      // if disabling, then CLEAR bit 2 by ANDing with 0xFB
      if(original)
      {
         data[3] |= 0x04;
      }
      else
      {
         data[3] &= 0xFB;
      }
   }
   
   /**
    * Determines whether or not the frame is marked as an original.
    * 
    * @return true if the frame is marked as original, false if not.
    */
   public boolean isOriginal()
   {
      boolean rval = false;
      
      // get the header bytes
      byte[] data = getBytes();
      
      // the original bit is located in bit 2 for byte 3, so
      // AND against 4
      byte original = (byte)(data[3] & 0x04);
      
      // is original if the bit is NOT cleared
      rval = (original != 0);
      
      return rval;
   }
   
   /**
    * Gets the emphasis for the frame.
    * 
    * @return the emphasis for the frame.
    */
   public Emphasis getEmphasis()
   {
      // get the header bytes
      byte[] data = getBytes();
      
      // the emphasis is located in bits 1 and 0 for byte 3, so shift
      // AND with 0x03
      // with 00000011 & 0x03 = 11
      byte emphasis = (byte)(data[3] & 0x03);
      
      // return the appropriate emphasis
      return Emphasis.getEmphasis(emphasis);
   }
   
   /**
    * Gets the side information size for this header in bytes. The side
    * information is either 32, 17, or 9 bytes in length. This information
    * is used to help decoders and is used to calculate the CRC-16 for
    * Layer III MPEG Audio frames.
    * 
    * @return the side information length for this header in bytes.
    */
   public int getSideInformationLength()
   {
      int rval = 0;
      
      // get version
      Version version = getVersion();
      
      // get channel count
      int channels = getChannelCount();
      
      if(version == Version.Mpeg1)
      {
         if(channels == 2)
         {
            // MPEG 1, stereo = 32 bytes
            rval = 32;
         }
         else if(channels == 1)
         {
            // MPEG 1, mono = 17 bytes
            rval = 17;
         }
      }
      else
      {
         if(channels == 2)
         {
            // MPEG 2/2.5, stereo = 17 bytes
            rval = 17;
         }
         else
         {
            // MPEG 2/2.5, mono = 9 bytes
            rval = 9;
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the length of the audio data in the frame (excluding the header and
    * any CRC-16) in bytes.
    * 
    * @return the length of the audio data in the frame in bytes.
    */
   public int getAudioDataLength()
   {
      int rval = 0;
      
      // subtract the header length from the frame length
      rval = getFrameLength() - 4;
      
      // subtract the CRC-16, if applicable
      if(isCrcEnabled())
      {
         rval -= 2;
      }
      
      return rval;
   }
   
   /**
    * Gets the length of the frame in bytes. This includes the header of 4
    * bytes, an option CRC-16 of 2 bytes, and the audio data. 
    * 
    * @return the length of the frame in bytes.
    */
   public int getFrameLength()
   {
      int rval = 0;
      
      // get the version and layer for the frame
      Version version = getVersion();
      Layer layer = getLayer();
      
      // get the bitrate and sampling rate for the frame
      double bitrate = getBitrate();
      double samplingRate = getSamplingRate();
      
      // get the padding for the frame
      int padding = (isPadded()) ? 1 : 0;
      
      // calculate the frame length based on the version and layer
      if(layer == Layer.Layer1)
      {
         rval = (int)Math.floor((12 * (bitrate / samplingRate) + padding) * 4);
      }
      else if(layer == Layer.Layer2)
      {
         rval = (int)Math.floor(144 * (bitrate / samplingRate) + padding);
      }
      else if(layer == Layer.Layer3)
      {
         if(version == Version.Mpeg1)
         {
            rval = (int)Math.floor(144 * (bitrate / samplingRate) + padding);
         }
         else
         {
            rval = (int)Math.floor(72 * (bitrate / samplingRate) + padding);
         }
      }
      
      return rval;
   }
   
   /**
    * Gets the length of the frame in seconds. This is the amount of time
    * it takes to play the audio in the frame.
    * 
    * @return the length of the frame in seconds.
    */
   public double getAudioLength()
   {
      double rval = 0.0D;
      
      // get the version and layer
      Version version = getVersion();
      Layer layer = getLayer();
      
      // calculate the audio length based on the version and layer
      if(layer == Layer.Layer1)
      {
         rval = 384D / getSamplingRate();
      }
      else if(layer == Layer.Layer2)
      {
         rval = 1152D / getSamplingRate();
      }
      else if(layer == Layer.Layer3)
      {
         if(version == Version.Mpeg1)
         {
            rval = 1152D / getSamplingRate();
         }
         else
         {
            rval = 576D / getSamplingRate();
         }
      }
      
      return rval;  
   }
   
   /**
    * Gets the header data as a byte array. This byte array will be allocated
    * on demand.
    * 
    * @return the header data as a byte array.
    */
   public byte[] getBytes()
   {
      if(mData == null)
      {
         mData = new byte[4];
         
         // set the first and second bytes to frame sync values
         mData[0] = (byte)255;
         mData[1] = (byte)240;
      }
      
      return mData;
   }
   
   /**
    * Converts this frame header to a string representation.
    * 
    * @return a human-readable string representation for this header.
    */
   public String toString()
   {
      StringBuffer sb = new StringBuffer();
      
      sb.append("[MpegAudioFrameHeader]");
      sb.append("\n" + getVersion());
      sb.append("\n" + getLayer());
      
      int bitrate = getBitrate();
      if(bitrate > 0)
      {
         sb.append("\nBitrate: " + bitrate + " bps");
      }
      else if(bitrate == 0)
      {
         sb.append("\nBitrate: Free Format");
      }
      else
      {
         sb.append("\nBitrate: Invalid");
      }
      
      int samplingRate = getSamplingRate();
      if(samplingRate != -1)
      {
         sb.append("\nSampling Rate: " + samplingRate + " Hz");
      }
      else
      {
         sb.append("\nSampling Rate: Invalid");
      }
      
      sb.append("\nChannel Mode: " + getChannelMode());
      sb.append("\nChannel Mode Extension: " + getChannelModeExtension());
      sb.append("\nEmphasis: " + getEmphasis());
      
      sb.append("\nCRC-16 Protected: " + isCrcEnabled());
      sb.append("\nPadded: " + isPadded());
      sb.append("\nPrivate Bit Set: " + isPrivateBitSet());
      sb.append("\nCopyrighted: " + isCopyrighted());
      sb.append("\nOriginal: " + isOriginal());
      
      sb.append("\nFrame Length: " + getFrameLength() + " bytes");
      sb.append("\nAudio Length: " + getAudioLength() + " seconds");
      
      return sb.toString();
   }
   
   /**
    * A Version enumerates the possible versions for MPEG Audio and their
    * bit values in this header.
    * 
    * @author Dave Longley
    */
   public enum Version
   {
      /**
       * Reserved version. Invalid.
       */
      Reserved("Reserved", (byte)0x01),

      /**
       * MPEG Version 1.
       */
      Mpeg1("Version 1", (byte)0x03),
      
      /**
       * MPEG Version 2.
       */
      Mpeg2("Version 2", (byte)0x02),
      
      /**
       * MPEG Version 2.5.
       */
      Mpeg25("Version 2.5", (byte)0x00);
      
      /**
       * The name for this version.
       */
      protected String mName;
      
      /**
       * The bit values for this version. These values are using in an
       * MpegAudioFrameHeader.
       */
      protected byte mBitValues;
      
      /**
       * Creates a new Version with the specified bit values.
       * 
       * @param name the name for this version.
       * @param bitValues the bit values for this version.
       */
      Version(String name, byte bitValues)
      {
         mName = name;
         mBitValues = bitValues;
      }
      
      /**
       * Returns the bit values for this version as a byte.
       *
       * @return the bit values for this version as a byte.
       */
      public byte bitValues()
      {
         return mBitValues;
      }
      
      /**
       * Returns the name for this version.
       * 
       * @return the name for this version.
       */
      public String toString()
      {
         return mName;
      }      
      
      /**
       * Gets a version from the given bit values.
       * 
       * @param bitValues the bit values for the version as a byte.
       * 
       * @return the version from the given bit values.
       */
      public static Version getVersion(byte bitValues)
      {
         Version rval = Version.Reserved;
         
         // set the appropriate version
         for(Version v: Version.values()) 
         {
            if(v.bitValues() == bitValues)
            {
               rval = v;
               break;
            }
         }
         
         return rval;
      }
   }
   
   /**
    * A Layer enumerates the possible layer for MPEG Audio and their
    * bit values in this header.
    * 
    * @author Dave Longley
    */
   public enum Layer
   {
      /**
       * Reserved layer. Invalid.
       */
      Reserved("Reserved", (byte)0x00),

      /**
       * Layer I.
       */
      Layer1("Layer I", (byte)0x03),
      
      /**
       * Layer II.
       */
      Layer2("Layer II", (byte)0x02),
      
      /**
       * Layer III.
       */
      Layer3("Layer III", (byte)0x01);
      
      /**
       * The name for this layer.
       */
      protected String mName;
      
      /**
       * The bit values for this layer. These values are using in an
       * MpegAudioFrameHeader.
       */
      protected byte mBitValues;
      
      /**
       * Creates a new Layer with the specified bit values.
       * 
       * @param name the name for this layer.
       * @param bitValues the bit values for this layer.
       */
      Layer(String name, byte bitValues)
      {
         mName = name;
         mBitValues = bitValues;
      }
      
      /**
       * Returns the bit values for this layer as a byte.
       *
       * @return the bit values for this layer as a byte.
       */
      public byte bitValues()
      {
         return mBitValues;
      }
      
      /**
       * Returns the name for this layer.
       * 
       * @return the name for this layer.
       */
      public String toString()
      {
         return mName;
      }
      
      /**
       * Gets a layer from the given bit values.
       * 
       * @param bitValues the bit values for the layer as a byte.
       * 
       * @return the layer from the given bit values.
       */
      public static Layer getLayer(byte bitValues)
      {
         Layer rval = Layer.Reserved;
         
         // set the appropriate layer
         for(Layer l: Layer.values()) 
         {
            if(l.bitValues() == bitValues)
            {
               rval = l;
               break;
            }
         }
         
         return rval;
      }
   }
   
   /**
    * A ChannelMode enumerates the possible channel modes for MPEG Audio and
    * their bit values in this header.
    * 
    * @author Dave Longley
    */
   public enum ChannelMode
   {
      /**
       * Stereo. 2 Channels.
       */
      Stereo("Stereo", (byte)0x00, 2),

      /**
       * Joint Stereo. 2 Channels.
       */
      JointStereo("Joint Stereo", (byte)0x01, 2),
      
      /**
       * Dual Channel. 2 Channels.
       */
      DualChannel("Dual Channel", (byte)0x02, 2),
      
      /**
       * Single Channel. 1 Channel.
       */
      SingleChannel("Single Channel", (byte)0x03, 1);
      
      /**
       * The name for this channel mode.
       */
      protected String mName;
      
      /**
       * The bit values for this channel mode. These values are using in an
       * MpegAudioFrameHeader.
       */
      protected byte mBitValues;
      
      /**
       * The number of channels used by this mode.
       */
      protected int mChannelCount;
      
      /**
       * Creates a new ChannelMode with the specified bit values.
       * 
       * @param name the name for this channel mode.
       * @param bitValues the bit values for this channel mode.
       * @param channels the number of channels used by this mode.
       */
      ChannelMode(String name, byte bitValues, int channels)
      {
         mName = name;
         mBitValues = bitValues;
         mChannelCount = channels;
      }
      
      /**
       * Returns the bit values for this channel mode as a byte.
       *
       * @return the bit values for this channel mode as a byte.
       */
      public byte bitValues()
      {
         return mBitValues;
      }
      
      /**
       * Gets the number of channels used by this mode.
       * 
       * @return the number of channels used by this mode.
       */
      public int channelCount()
      {
         return mChannelCount;
      }
      
      /**
       * Returns the name for this channel mode.
       * 
       * @return the name for this channel mode.
       */
      public String toString()
      {
         return mName;
      }
      
      /**
       * Gets a channel mode from the given bit values.
       * 
       * @param bitValues the bit values for the channel mode as a byte.
       * 
       * @return the channel mode from the given bit values.
       */
      public static ChannelMode getChannelMode(byte bitValues)
      {
         ChannelMode rval = ChannelMode.Stereo;
         
         // set the appropriate channel mode
         for(ChannelMode cm: ChannelMode.values()) 
         {
            if(cm.bitValues() == bitValues)
            {
               rval = cm;
               break;
            }
         }
         
         return rval;
      }
   }
   
   /**
    * A ChannelModeExtension enumerates the possible channel mode extensions
    * for MPEG Audio and their bit values in this header.
    * 
    * JJ: Channel Mode Extension (used only for Joint Stereo Channel Mode).
    * 
    * These bits are dynamically determined by an encoder using Joint Stereo
    * Channel Mode.
    * 
    * For Layers I & II the frequency range of the MPEG data is divided into 32
    * subbands. These two bits (JJ) determine where intensity stereo is applied
    * like so:
    * 
    * Layers I & II
    * bits  band range
    * 00    4 - 31
    * 01    8 - 31
    * 10    12 - 31
    * 11    16 - 31
    * 
    * For Layer III these two bits (JJ) determine which type of joint stereo
    * is used (intensity stereo or m/s stereo). The frequency range is
    * determined by the decompression algorithm.
    * 
    * Layer III
    * bits Intensity Stereo  MS Stereo
    * 00   off               off
    * 01   on                off
    * 10   off               on
    * 11   on                on
    * 
    * @author Dave Longley
    */
   public enum ChannelModeExtension
   {
      /**
       * Band range 4-31. Used with Layers I & II.
       */
      BandRange4("Band Range 4-31", (byte)0x00, 4),

      /**
       * Band range 8-31. Used with Layers I & II.
       */
      BandRange8("Band Range 8-31", (byte)0x01, 8),
      
      /**
       * Band range 12-31. Used with Layers I & II.
       */
      BandRange12("Band Range 12-31", (byte)0x02, 12),
      
      /**
       * Band range 16-31. Used with Layers I & II.
       */
      BandRange16("Band Range 16-31", (byte)0x03, 16),
      
      /**
       * Intensity Stereo Off, MS Stereo Off. Used with Layer III.
       */
      IntensityOffMSOff("Intensity Stereo Off/MS Stereo Off", (byte)0x00, 32),

      /**
       * Intensity Stereo On, MS Stereo Off. Used with Layer III.
       */
      IntensityOnMSOff("Intensity Stereo On/MS Stereo Off", (byte)0x01, 32),
      
      /**
       * Intensity Stereo Off, MS Stereo On. Used with Layer III.
       */
      IntensityOffMSOn("Intensity Stereo Off/MS Stereo On", (byte)0x02, 32),
      
      /**
       * Intensity Stereo On, MS Stereo On. Used with Layer III.
       */
      IntensityOnMSOn("Intensity Stereo On/MS Stereo On", (byte)0x03, 32);
      
      /**
       * The name for this channel mode.
       */
      protected String mName;
      
      /**
       * The bit values for this channel mode. These values are using in an
       * MpegAudioFrameHeader.
       */
      protected byte mBitValues;
      
      /**
       * The lower band bound for this extension. This is only valid for
       * Layer I & II extensions.
       */
      protected int mLowerBand;
      
      /**
       * Creates a new ChannelModeExtension with the specified bit values.
       * 
       * @param name the name for this channel mode extension.
       * @param bitValues the bit values for this channel mode extension.
       * @param lowerBand the lowest subband for this extension.
       */
      ChannelModeExtension(String name, byte bitValues, int lowerBand)
      {
         mName = name;
         mBitValues = bitValues;
         mLowerBand = lowerBand;
      }
      
      /**
       * Returns the bit values for this channel mode extension as a byte.
       *
       * @return the bit values for this channel mode extension as a byte.
       */
      public byte bitValues()
      {
         return mBitValues;
      }
      
      /**
       * Returns the lower band bound for this extension. This is only
       * valid for Layer I && II extensions. Always returns 32 (maximum
       * subband) for Layer III.
       * 
       * @return the lower band bound.
       */
      public int lowerBand()
      {
         return mLowerBand;
      }
      
      /**
       * Returns the name for this channel mode extension.
       * 
       * @return the name for this channel mode extension.
       */
      public String toString()
      {
         return mName;
      }
      
      /**
       * Gets a channel mode extension from the given bit values.
       * 
       * @param bitValues the bit values for the channel mode extension 
       *                  as a byte.
       * @param layer the layer the extension is for.
       * 
       * @return the channel mode extension from the given bit values.
       */
      public static ChannelModeExtension getChannelModeExtension(
         byte bitValues, Layer layer)
      {
         ChannelModeExtension rval = null;
         
         if(layer == Layer.Layer3)
         {
            // set the appropriate channel mode extension
            for(ChannelModeExtension cme: EnumSet.range(
                   IntensityOffMSOff, IntensityOnMSOn)) 
            {
               if(cme.bitValues() == bitValues)
               {
                  rval = cme;
                  break;
               }
            }            
         }
         else
         {
            // set the appropriate channel mode extension
            for(ChannelModeExtension cme: EnumSet.range(
                   BandRange4, BandRange16)) 
            {
               if(cme.bitValues() == bitValues)
               {
                  rval = cme;
                  break;
               }
            }
         }
         
         return rval;
      }
   }
   
   /**
    * An Emphasis tells a decoder to "re-equalize" sound after a sound
    * suppression.
    * 
    * @author Dave Longley
    */
   public enum Emphasis
   {
      /**
       * None.
       */
      None("None", (byte)0x00),

      /**
       * 50/15 ms.
       */
      FiftyFifteenMilliseconds("50/15 ms", (byte)0x01),
      
      /**
       * Reserved.
       */
      Reserved("Reserved", (byte)0x02),
      
      /**
       * CCIT J.17.
       */
      CCITJ17("CCIT J.17", (byte)0x03);
      
      /**
       * The name for this emphasis.
       */
      protected String mName;
      
      /**
       * The bit values for this emphasis. These values are using in an
       * MpegAudioFrameHeader.
       */
      protected byte mBitValues;
      
      /**
       * Creates a new Emphasis with the specified bit values.
       * 
       * @param name the name for this emphasis.
       * @param bitValues the bit values for this emphasis.
       */
      Emphasis(String name, byte bitValues)
      {
         mName = name;
         mBitValues = bitValues;
      }
      
      /**
       * Returns the bit values for this emphasis as a byte.
       *
       * @return the bit values for this emphasis as a byte.
       */
      public byte bitValues()
      {
         return mBitValues;
      }
      
      /**
       * Returns the name for this emphasis.
       * 
       * @return the name for this emphasis.
       */
      public String toString()
      {
         return mName;
      }
      
      /**
       * Gets a emphasis from the given bit values.
       * 
       * @param bitValues the bit values for the emphasis as a byte.
       * 
       * @return the emphasis from the given bit values.
       */
      public static Emphasis getEmphasis(byte bitValues)
      {
         Emphasis rval = Emphasis.None;
         
         // set the appropriate emphasis
         for(Emphasis e: Emphasis.values()) 
         {
            if(e.bitValues() == bitValues)
            {
               rval = e;
               break;
            }
         }
         
         return rval;
      }
   }   
   
   /**
    * A BitrateTable maps Bitrate Indices (as bit values in this header) to
    * particular bitrates.
    * 
    *    EEEE: Bitrate Index - an index into a Bitrate Table:
    *    bits  V1,L1 V1,L2 V1,L3   V2,L1 V2,L2&L3
    * 0  0000  free  free  free    free  free
    * 1  0001  32    32    32      32    8
    * 2  0010  64    48    40      48    16
    * 3  0011  96    56    48      56    24
    * 4  0100  128   64    56      64    32
    * 5  0101  160   80    64      80    40
    * 6  0110  192   96    80      96    48
    * 7  0111  224   112   96      112   56
    * 8  1000  256   128   112     128   64
    * 9  1001  288   160   128     144   80
    * 10 1010  320   192   160     160   96
    * 11 1011  352   224   192     176   112
    * 12 1100  384   256   224     192   128
    * 13 1101  416   320   256     224   144
    * 14 1110  448   384   320     256   160
    * 15 1111  res   res   res     res   res
    * 
    * Key:
    * All values above are in kbps (1000 bits per second NOT 1024 bits per
    * second)
    * V1 = MPEG Version 1
    * V2 = MPEG Version 2 and 2.5
    * L1 = Layer I
    * L2 = Layer II
    * L3 = Layer III
    * free = Free format. Application determined.
    * res = Reserved. This is invalid should never be used.
    * 
    * @author Dave Longley
    */
   public static class BitrateTable
   {
      /**
       * The underlying hash map used to map indices, versions, and layers
       * to bitrates.
       */
      protected HashMap<String, Integer> mTable =
         new HashMap<String, Integer>();
      
      /**
       * Creates a new BitrateTable.
       */
      public BitrateTable()
      {
         // add the bitrate table entries:
         
         // add bitrates for Version 1 Layer I
         for(byte i = 0x00; i < 0x0F; i++)
         {
            addBitrate(i, Version.Mpeg1, Layer.Layer1, i * 32);
         }
         
         // add bitrates for Version 1 Layer II
         addBitrate((byte)0x00, Version.Mpeg1, Layer.Layer2, 0);
         addBitrate((byte)0x01, Version.Mpeg1, Layer.Layer2, 32);
         addBitrate((byte)0x02, Version.Mpeg1, Layer.Layer2, 48);
         addBitrate((byte)0x03, Version.Mpeg1, Layer.Layer2, 56);
         addBitrate((byte)0x04, Version.Mpeg1, Layer.Layer2, 64);
         addBitrate((byte)0x05, Version.Mpeg1, Layer.Layer2, 80);
         addBitrate((byte)0x06, Version.Mpeg1, Layer.Layer2, 96);
         addBitrate((byte)0x07, Version.Mpeg1, Layer.Layer2, 112);
         addBitrate((byte)0x08, Version.Mpeg1, Layer.Layer2, 128);
         addBitrate((byte)0x09, Version.Mpeg1, Layer.Layer2, 160);
         addBitrate((byte)0x0A, Version.Mpeg1, Layer.Layer2, 192);
         addBitrate((byte)0x0B, Version.Mpeg1, Layer.Layer2, 224);
         addBitrate((byte)0x0C, Version.Mpeg1, Layer.Layer2, 256);
         addBitrate((byte)0x0D, Version.Mpeg1, Layer.Layer2, 320);
         addBitrate((byte)0x0E, Version.Mpeg1, Layer.Layer2, 384);
         
         // add bitrates for Version 1 Layer III
         addBitrate((byte)0x00, Version.Mpeg1, Layer.Layer3, 0);
         addBitrate((byte)0x01, Version.Mpeg1, Layer.Layer3, 32);
         addBitrate((byte)0x02, Version.Mpeg1, Layer.Layer3, 40);
         addBitrate((byte)0x03, Version.Mpeg1, Layer.Layer3, 48);
         addBitrate((byte)0x04, Version.Mpeg1, Layer.Layer3, 56);
         addBitrate((byte)0x05, Version.Mpeg1, Layer.Layer3, 64);
         addBitrate((byte)0x06, Version.Mpeg1, Layer.Layer3, 80);
         addBitrate((byte)0x07, Version.Mpeg1, Layer.Layer3, 96);
         addBitrate((byte)0x08, Version.Mpeg1, Layer.Layer3, 112);
         addBitrate((byte)0x09, Version.Mpeg1, Layer.Layer3, 128);
         addBitrate((byte)0x0A, Version.Mpeg1, Layer.Layer3, 160);
         addBitrate((byte)0x0B, Version.Mpeg1, Layer.Layer3, 192);
         addBitrate((byte)0x0C, Version.Mpeg1, Layer.Layer3, 224);
         addBitrate((byte)0x0D, Version.Mpeg1, Layer.Layer3, 256);
         addBitrate((byte)0x0E, Version.Mpeg1, Layer.Layer3, 320);
         
         // add bitrates for Version 2 Layer I
         addBitrate((byte)0x00, Version.Mpeg2, Layer.Layer1, 0);
         addBitrate((byte)0x01, Version.Mpeg2, Layer.Layer1, 32);
         addBitrate((byte)0x02, Version.Mpeg2, Layer.Layer1, 48);
         addBitrate((byte)0x03, Version.Mpeg2, Layer.Layer1, 56);
         addBitrate((byte)0x04, Version.Mpeg2, Layer.Layer1, 64);
         addBitrate((byte)0x05, Version.Mpeg2, Layer.Layer1, 80);
         addBitrate((byte)0x06, Version.Mpeg2, Layer.Layer1, 96);
         addBitrate((byte)0x07, Version.Mpeg2, Layer.Layer1, 112);
         addBitrate((byte)0x08, Version.Mpeg2, Layer.Layer1, 128);
         addBitrate((byte)0x09, Version.Mpeg2, Layer.Layer1, 144);
         addBitrate((byte)0x0A, Version.Mpeg2, Layer.Layer1, 160);
         addBitrate((byte)0x0B, Version.Mpeg2, Layer.Layer1, 176);
         addBitrate((byte)0x0C, Version.Mpeg2, Layer.Layer1, 192);
         addBitrate((byte)0x0D, Version.Mpeg2, Layer.Layer1, 224);
         addBitrate((byte)0x0E, Version.Mpeg2, Layer.Layer1, 256);
         
         // add bitrates for Version 2 Layer II
         addBitrate((byte)0x00, Version.Mpeg2, Layer.Layer2, 0);
         addBitrate((byte)0x01, Version.Mpeg2, Layer.Layer2, 8);
         addBitrate((byte)0x02, Version.Mpeg2, Layer.Layer2, 16);
         addBitrate((byte)0x03, Version.Mpeg2, Layer.Layer2, 24);
         addBitrate((byte)0x04, Version.Mpeg2, Layer.Layer2, 32);
         addBitrate((byte)0x05, Version.Mpeg2, Layer.Layer2, 40);
         addBitrate((byte)0x06, Version.Mpeg2, Layer.Layer2, 48);
         addBitrate((byte)0x07, Version.Mpeg2, Layer.Layer2, 56);
         addBitrate((byte)0x08, Version.Mpeg2, Layer.Layer2, 64);
         addBitrate((byte)0x09, Version.Mpeg2, Layer.Layer2, 80);
         addBitrate((byte)0x0A, Version.Mpeg2, Layer.Layer2, 96);
         addBitrate((byte)0x0B, Version.Mpeg2, Layer.Layer2, 112);
         addBitrate((byte)0x0C, Version.Mpeg2, Layer.Layer2, 128);
         addBitrate((byte)0x0D, Version.Mpeg2, Layer.Layer2, 144);
         addBitrate((byte)0x0E, Version.Mpeg2, Layer.Layer2, 160);
         
         // add bitrates for Version 2 Layer III
         addBitrate((byte)0x00, Version.Mpeg2, Layer.Layer3, 0);
         addBitrate((byte)0x01, Version.Mpeg2, Layer.Layer3, 8);
         addBitrate((byte)0x02, Version.Mpeg2, Layer.Layer3, 16);
         addBitrate((byte)0x03, Version.Mpeg2, Layer.Layer3, 24);
         addBitrate((byte)0x04, Version.Mpeg2, Layer.Layer3, 32);
         addBitrate((byte)0x05, Version.Mpeg2, Layer.Layer3, 40);
         addBitrate((byte)0x06, Version.Mpeg2, Layer.Layer3, 48);
         addBitrate((byte)0x07, Version.Mpeg2, Layer.Layer3, 56);
         addBitrate((byte)0x08, Version.Mpeg2, Layer.Layer3, 64);
         addBitrate((byte)0x09, Version.Mpeg2, Layer.Layer3, 80);
         addBitrate((byte)0x0A, Version.Mpeg2, Layer.Layer3, 96);
         addBitrate((byte)0x0B, Version.Mpeg2, Layer.Layer3, 112);
         addBitrate((byte)0x0C, Version.Mpeg2, Layer.Layer3, 128);
         addBitrate((byte)0x0D, Version.Mpeg2, Layer.Layer3, 144);
         addBitrate((byte)0x0E, Version.Mpeg2, Layer.Layer3, 160);
      }
      
      /**
       * Adds a bitrate entry for the given index, version, and layer.
       * 
       * @param index the bit values for the bitrate index.
       * @param version the MPEG version.
       * @param layer the MPEG layer.
       * @param bitrate the bitrate with 0 indicating a free format bitrate.
       */
      protected void addBitrate(
         byte index, Version version, Layer layer, int bitrate)
      {
         mTable.put("" + index + version + layer, bitrate);
      }
      
      /**
       * Gets the bitrate for the given index (bit values as a byte), version,
       * and layer.
       * 
       * @param index the bit values for the bitrate index.
       * @param version the MPEG version.
       * @param layer the MPEG layer type.
       * 
       * @return the bitrate (in bits/second) with 0 indicating a free format
       *         bitrate and -1 indicating an invalid bitrate index.
       */
      public int getBitrate(byte index, Version version, Layer layer)
      {
         int rval = -1;
         
         // MPEG version 2.5 uses the same bitrate table as MPEG version 2
         if(version == Version.Mpeg25)
         {
            version = Version.Mpeg2;
         }
         
         Integer i = mTable.get("" + index + version + layer);
         if(i != null)
         {
            // convert from kilobits to bits
            rval = i.intValue() * 1000;
         }
         
         return rval;
      }
   }
   
   /**
    * A SamplingRateTable maps Sampling Rate Indices (as bit values in this
    * header) to particular sampling rates.
    * 
    * FF: Sampling Rate Index - an index into a Sampling Rate Table:
    * bits MPEG1  MPEG2  MPEG2.5
    * 00   44100  22050  11025
    * 01   48000  24000  12000
    * 10   32000  16000  8000
    * 11   res    res    res
    * 
    * Key:
    * All values are in Hz (Hertz, one complete cycle of a sound wave per
    * second)
    * res = Reserved. This is invalid and should not be used.
    * MPEG1 = MPEG Version 1
    * MPEG2 = MPEG Version 2
    * MPEG2.5 = MPEG Version 2.5
    *
    * @author Dave Longley
    */
   public static class SamplingRateTable
   {
      /**
       * The underlying hash map used to map indices and versions to sampling
       * rates.
       */
      protected HashMap<String, Integer> mTable =
         new HashMap<String, Integer>();
      
      /**
       * Creates a new SamplingRateTable.
       */
      public SamplingRateTable()
      {
         // add the sampling rate table entries:
         
         // add sampling rates for Version 1
         addSamplingRate((byte)0x00, Version.Mpeg1, 44100);
         addSamplingRate((byte)0x01, Version.Mpeg1, 48000);
         addSamplingRate((byte)0x02, Version.Mpeg1, 32000);
         
         // add sampling rates for Version 2
         addSamplingRate((byte)0x00, Version.Mpeg2, 22050);
         addSamplingRate((byte)0x01, Version.Mpeg2, 24000);
         addSamplingRate((byte)0x02, Version.Mpeg2, 16000);
         
         // add sampling rates for Version 2.5
         addSamplingRate((byte)0x00, Version.Mpeg25, 11025);
         addSamplingRate((byte)0x01, Version.Mpeg25, 12000);
         addSamplingRate((byte)0x02, Version.Mpeg25, 8000);
      }
      
      /**
       * Adds a sampling rate entry for the given index and version.
       * 
       * @param index the bit values for the bitrate index.
       * @param version the MPEG version.
       * @param samplingRate the sampling rate (in Hz).
       */
      protected void addSamplingRate(
         byte index, Version version, int samplingRate)
      {
         mTable.put("" + index + version, samplingRate);
      }
      
      /**
       * Gets the sampling rate for the given index (bit values as a byte) and
       * version.
       * 
       * @param index the bit values for the sampling rate index.
       * @param version the MPEG version.
       * 
       * @return the sampling rate (in samples/second) with -1 indicating an
       *         invalid sampling rate index.
       */
      public int getSamplingRate(byte index, Version version)
      {
         int rval = -1;
         
         Integer i = mTable.get("" + index + version);
         if(i != null)
         {
            rval = i.intValue();
         }
         
         return rval;
      }      
   }
}
