/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_mpeg_FrameHeader_H
#define db_data_mpeg_FrameHeader_H

#include "db/io/ByteBuffer.h"
#include "db/data/mpeg/AudioCharacteristics.h"
#include "db/data/mpeg/AudioBitrateTable.h"
#include "db/data/mpeg/AudioSamplingRateTable.h"

namespace db
{
namespace data
{
namespace mpeg
{

/**
 * An AudioFrameHeader is a header for an MPEG AudioFrame.
 * 
 * MPEG Audio data is made up of frames. An AudioFrame represents a single
 * frame. Each frame uses a *fixed* number of samples. A sample is one complete
 * cycle of a sound wave. Each frame contains a header that is 4 bytes in
 * length. This class represents that header.
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
 *     AudioFrame) is protected by a 16 bit CRC. The location of the CRC
 *     in the frame is outlined in AudioFrame.
 * 1 - Not Protected. How this affects the frame format is outlined in
 *     AudioFrame.
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
 * See the ChannelModeExtension enumeration for details.
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
class AudioFrameHeader
{
protected:
   /**
    * The header data as a byte array.
    */
   db::io::ByteBuffer* mData;
   
   /**
    * A helper function for interpreting the header data as unsigned chars.
    */
   virtual unsigned char* getDataBytes();
   
   /**
    * A static bitrate table used for all frame headers.
    */
   static AudioBitrateTable sBitrateTable;
   
   /**
    * A static sampling rate table used for all frame headers.
    */
   static AudioSamplingRateTable sSamplingRateTable;
   
public:
   /**
    * Creates a new AudioFrameHeader.
    */
   AudioFrameHeader();
   
   /**
    * Destructs this AudioFrameHeader.
    */
   virtual ~AudioFrameHeader();
   
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
   virtual bool convertFromBytes(const char* bytes, int length);
   
   /**
    * Determines if this header is valid or not.
    * 
    * @return true if this header is valid, false if not.
    */
   virtual bool isValid();
   
   /**
    * Determines if this header has frame sync. This means that its first
    * 11 bits are set.
    * 
    * @return true if this header has its first 11 bits set, false if not.
    */
   virtual bool hasFrameSync();
   
   /**
    * Gets the version for the frame.
    * 
    * @param version the Version to populate.
    */
   virtual void getVersion(AudioVersion& version);
   
   /**
    * Determines if this header has a valid MPEG version or not.
    * 
    * @return true if this header has a valid MPEG version, false if not.
    */
   virtual bool isVersionValid();
   
   /**
    * Gets the layer for the frame.
    * 
    * @param layer the layer to populate.
    */
   virtual void getLayer(AudioLayer& layer);
   
   /**
    * Determines if this header has a valid MPEG layer type or not.
    * 
    * @return true if this header has a valid MPEG layer type, false if not.
    */
   virtual bool isLayerValid();
   
   /**
    * Sets whether or not CRC-16 protection is enabled for the frame.
    * 
    * @param enabled true to enable CRC-16 protection for the frame, false
    *                to disable it.
    */
   virtual void setCrcEnabled(bool enabled);
   
   /**
    * Determines whether or not CRC-16 protection is enabled for the frame.
    * 
    * @return true if CRC-16 protection is enabled for the frame, false if not.
    */
   virtual bool isCrcEnabled();
   
   /**
    * Gets the bitrate for this frame in bits/second.
    * 
    * @return the bitrate for this frame in bits/second with 0 indicating
    *         a free format bitrate and -1 indicating an invalid bitrate.
    */
   virtual int getBitrate();
   
   /**
    * Determines if this header has a valid bitrate or not.
    * 
    * @return true if this header has a valid bitrate, false if not.
    */
   virtual bool isBitrateValid();
   
   /**
    * Gets the sampling rate for this frame in samples/second.
    * 
    * @return the sampling rate for this frame in samples/second with -1
    *         indicating an invalid sampling rate.
    */
   virtual int getSamplingRate();
   
   /**
    * Determines if this header has a valid sampling rate or not.
    * 
    * @return true if this header has a valid sampling rate, false if not.
    */
   virtual bool isSamplingRateValid();
   
   /**
    * Sets whether or not the frame is padded.
    * 
    * @param padded true to mark the frame as padded, false to mark it as
    *               unpadded.
    */
   virtual void setPadded(bool padded);
   
   /**
    * Determines whether or not the frame is padded.
    * 
    * @return true if the frame is padded, false if not.
    */
   virtual bool isPadded();
   
   /**
    * Sets whether or not the private bit is set for the frame.
    * 
    * @param set true to set the private bit for the frame, false to clear it.
    */
   virtual void setPrivateBit(bool set);
   
   /**
    * Determines whether or not the private bit is set for the frame.
    * 
    * @return true if the private bit is set for the frame, false if not.
    */
   virtual bool isPrivateBitSet();
   
   /**
    * Gets the channel mode for the frame.
    * 
    * @param cm the ChannelMode to populate.
    */
   virtual void getChannelMode(AudioChannelMode& cm);
   
   /**
    * Gets the number of channels used in the frame.
    * 
    * @return the number of channels used in the frame.
    */
   virtual int getChannelCount();
   
   /**
    * Determines if the bitrate and channel mode are valid combinations for
    * the frame. This will always return true for Layers I & III, but false
    * for Layers II in some select cases.
    * 
    * @return true if the bitrate and channel mode are valid combinations
    *         for the frame, false if not.
    */
   virtual bool isBitrateChannelModeCombinationValid();
   
   /**
    * Gets the channel mode extension for the frame.
    * 
    * @param cme the ChannelModeExtension to populate.
    */
   virtual void getChannelModeExtension(AudioChannelModeExtension& cme);
   
   /**
    * Gets the highest subband the joint stereo is bound to. This means that
    * there are 2 channels (stereo) of information for subbands up until this
    * number -- the rest of the subbands use only 1 channel (mono). This value
    * is required for CRC-16 calculation for Layer I frames.
    * 
    * @return the Joint Stereo Bound.
    */
   virtual int getJointStereoBound();
   
   /**
    * Sets whether or not the frame is marked as copyrighted.
    * 
    * @param copyrighted true to mark this frame as copyrighted, false to mark
    *                    it as not copyrighted.
    */
   virtual void setCopyrighted(bool copyrighted);
   
   /**
    * Determines whether or not the frame is marked as copyrighted.
    * 
    * @return true if the frame is marked as copyrighted, false if not.
    */
   virtual bool isCopyrighted();
   
   /**
    * Sets whether or not the frame is marked as an original.
    *  
    * @param original true to mark this frame as original, false to mark
    *                 it as not original.
    */
   virtual void setOriginal(bool original);
      
   /**
    * Determines whether or not the frame is marked as an original.
    * 
    * @return true if the frame is marked as original, false if not.
    */
   virtual bool isOriginal();
   
   /**
    * Gets the emphasis for the frame.
    * 
    * @param emphasis the AudioEmphasis to populate.
    */
   virtual void getEmphasis(AudioEmphasis& emphasis);
   
   /**
    * Gets the side information size for this header in bytes. The side
    * information is either 32, 17, or 9 bytes in length. This information
    * is used to help decoders and is used to calculate the CRC-16 for
    * Layer III MPEG Audio frames.
    * 
    * @return the side information length for this header in bytes.
    */
   virtual int getSideInformationLength();
   
   /**
    * Gets the length of the audio data in the frame (excluding the header and
    * any CRC-16) in bytes.
    * 
    * @return the length of the audio data in the frame in bytes.
    */
   virtual int getAudioDataLength();
   
   /**
    * Gets the length of the frame in bytes. This includes the header of 4
    * bytes, an optional CRC-16 of 2 bytes, and the audio data. 
    * 
    * @return the length of the frame in bytes.
    */
   virtual int getFrameLength();
   
   /**
    * Gets the length of the frame in seconds. This is the amount of time
    * it takes to play the audio in the frame.
    * 
    * @return the length of the frame in seconds.
    */
   virtual double getAudioLength();
   
   /**
    * Gets the header data as a byte array. This byte array will be allocated
    * on demand.
    * 
    * @return the header data as a byte array.
    */
   virtual db::io::ByteBuffer* getBytes();
   
   /**
    * Converts this frame header to a string representation.
    * 
    * @param str the string to fill with a human-readable string representation
    *            for this header.
    */
   virtual std::string& toString(std::string& str);
   
   /**
    * Calculates frame length in bytes based on version, layer, bitrate,
    * sampling rate, and padding.
    * 
    * @param version the MPEG version.
    * @param layer the MPEG layer type.
    * @param bitrate the bitrate (in bits/second).
    * @param samplingRate the sampling rate (in samples/second).
    * @param paddingEnabled true if a padding slot is used, false if not.
    * 
    * @return the frame length in bytes.
    */
   static int calculateFrameLength(
      const AudioVersion& version, const AudioLayer& layer,
      double bitrate, double samplingRate, bool paddingEnabled);
   
   /**
    * Calculates the minimum frame length, in bytes, for the given version
    * and layer.
    * 
    * @param version the MPEG version.
    * @param layer the MPEG layer type.
    * 
    * @return the minimum frame length, in bytes, for the given version and
    *         layer.
    */
   static int calculateMinFrameLength(
      const AudioVersion& version, const AudioLayer& layer);   
   
   /**
    * Calculates the maximum frame length, in bytes, for the given version
    * and layer.
    * 
    * @param version the MPEG version.
    * @param layer the MPEG layer type.
    * 
    * @return the maximum frame length, in bytes, for the given version and
    *         layer.
    */
   static int calculateMaxFrameLength(
      const AudioVersion& version, const AudioLayer& layer);   
};

} // end namespace mpeg
} // end namespace data
} // end namespace db
#endif
