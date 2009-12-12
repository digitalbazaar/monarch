/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/mpeg/AudioFrameHeader.h"

#include "monarch/rt/Exception.h"
#include <math.h>
#include <cstdio>

using namespace std;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::data::mpeg;

// initialize tables
AudioBitrateTable AudioFrameHeader::sBitrateTable;
AudioSamplingRateTable AudioFrameHeader::sSamplingRateTable;

AudioFrameHeader::AudioFrameHeader()
{
   mData = NULL;
}

AudioFrameHeader::~AudioFrameHeader()
{
   if(mData != NULL)
   {
      delete mData;
   }
}

inline unsigned char* AudioFrameHeader::getDataBytes()
{
   return (unsigned char*)getBytes()->bytes();
}

bool AudioFrameHeader::convertFromBytes(const char* bytes, int length)
{
   bool rval = false;

   if(length < 4)
   {
      ExceptionRef e = new Exception(
         "AudioFrameHeader::convertFromBytes() length must be >= 4",
         "db.data.mpeg.InvalidLength");
      Exception::set(e);
   }
   else
   {
      if(mData == NULL)
      {
         mData = new ByteBuffer(4);
      }

      // put bytes into buffer
      mData->clear();
      mData->put(bytes, 4, false);

      // determine if this header is valid
      rval = isValid();
   }

   return rval;
}

bool AudioFrameHeader::isValid()
{
   bool rval = false;

   // this header is valid if it has frame sync and its version,
   // layer, bitrate, sampling rate are valid, and if its bitrate
   // and channel mode combination are valid
   if(hasFrameSync() && isVersionValid() && isLayerValid() &&
      isBitrateValid() && isSamplingRateValid() &&
      isBitrateChannelModeCombinationValid())
   {
      rval = true;
   }

   return rval;
}

bool AudioFrameHeader::hasFrameSync()
{
   bool rval = false;

   // the first 11 bits must be set to 1 for "frame sync":
   // 1111111111xxxx
   // 1111111 = 255
   // 111xxxx = at least 224 (greater than 223)
   // 255 = 0xff, 224 = 0xe0, 223 = 0xdf
   if(getDataBytes()[0] == 0xff && getDataBytes()[1] > 0xdf)
   {
      rval = true;
   }

   return rval;
}

void AudioFrameHeader::getVersion(AudioVersion& version)
{
   // the version is located in bits 4 and 3 for byte 1, so shift
   // to the right 3 and then AND with 3
   // with 00011000 >> 3 = 00000011 & 0x03 = 11
   version.setBitValues((getDataBytes()[1] >> 3) & 0x03);
}

bool AudioFrameHeader::isVersionValid()
{
   // any version other than reserved (0x01) is valid
   return ((getDataBytes()[1] >> 3) & 0x03) != 0x01;
}

void AudioFrameHeader::getLayer(AudioLayer& layer)
{
   // the layer is located in bits 2 and 1 for byte 1, so shift
   // to the right 1 and then AND with 1
   // with 00000110 >> 1 = 00000011 & 0x03 = 11
   layer.setBitValues((getDataBytes()[1] >> 1) & 0x03);
}

bool AudioFrameHeader::isLayerValid()
{
   // any layer other than reserved (0x00) is valid
   return ((getDataBytes()[1] >> 1) & 0x03) != 0x00;
}

void AudioFrameHeader::setCrcEnabled(bool enabled)
{
   // the crc protection bit is located in bit 0 for byte 1,
   // if enabling, then CLEAR bit 0 by ANDing with 0xFE,
   // if disabling, then SET bit 0 by ORing with 0x01
   if(enabled)
   {
      getDataBytes()[1] &= 0xfe;
   }
   else
   {
      getDataBytes()[1] |= 0x01;
   }
}

bool AudioFrameHeader::isCrcEnabled()
{
   // the crc protection bit is located in bit 0 for byte 1, so
   // AND against 1
   // protection is enabled if the bit is NOT set (it IS cleared)
   return (getDataBytes()[1] & 0x01) == 0;
}

int AudioFrameHeader::getBitrate()
{
   // the bitrate index is located in bits 7-4 for byte 2, so shift
   // to the right 4 and then AND with 0x0F
   // with 11110000 >> 4 = 00001111 & 0x0F = 1111
   unsigned char bitrateIndex = ((getDataBytes()[2] >> 4) & 0x0f);

   // get the bitrate from the bitrate table
   AudioVersion v;
   getVersion(v);
   AudioLayer l;
   getLayer(l);
   return sBitrateTable.getBitrate(bitrateIndex, v, l);
}

bool AudioFrameHeader::isBitrateValid()
{
   // bitrate is valid if it is not -1
   return getBitrate() != -1;
}

int AudioFrameHeader::getSamplingRate()
{
   // the sampling rate index is located in bits 3-2 for byte 2, so shift
   // to the right 2 and then AND with 0x03
   // with 00001100 >> 2 = 00000011 & 0x03 = 11
   unsigned char samplingRateIndex = ((getDataBytes()[2] >> 2) & 0x03);

   // get the sampling rate from the sampling rate table
   AudioVersion v;
   getVersion(v);
   return sSamplingRateTable.getSamplingRate(samplingRateIndex, v);
}

bool AudioFrameHeader::isSamplingRateValid()
{
   // sampling rate is valid if it is not -1
   return getSamplingRate() != -1;
}

void AudioFrameHeader::setPadded(bool padded)
{
   // the padding bit is located in bit 1 for byte 2,
   // if enabling, then SET bit 1 by ORing with 0x02,
   // if disabling, then CLEAR bit 1 by ANDing with 0xFD
   if(padded)
   {
      getDataBytes()[2] |= 0x02;
   }
   else
   {
      getDataBytes()[2] &= 0xfd;
   }
}

bool AudioFrameHeader::isPadded()
{
   // the padding bit is located in bit 1 for byte 2, so
   // AND against 2
   // padding is enabled if the bit is NOT cleared
   return (getDataBytes()[2] & 0x02) != 0;
}

void AudioFrameHeader::setPrivateBit(bool set)
{
   // the private bit is located in bit 0 for byte 2,
   // if enabling, then SET bit 1 by ORing with 0x01,
   // if disabling, then CLEAR bit 1 by ANDing with 0xFE
   if(set)
   {
      getDataBytes()[2] |= 0x01;
   }
   else
   {
      getDataBytes()[2] &= 0xfe;
   }
}

bool AudioFrameHeader::isPrivateBitSet()
{
   // the private bit is located in bit 0 for byte 2, so
   // AND against 1
   // privateBit is enabled if the bit is NOT cleared
   return (getDataBytes()[2] & 0x01) != 0;
}

void AudioFrameHeader::getChannelMode(AudioChannelMode& cm)
{
   // the channel mode is located in bits 7 and 6 for byte 3, so shift
   // to the right 6 and then AND with 0x03
   // with 11000000 >> 6 = 00000011 & 0x03 = 11
   cm.setBitValues((getDataBytes()[3] >> 6) & 0x03);
}

int AudioFrameHeader::getChannelCount()
{
   // get the channel mode
   AudioChannelMode cm;
   getChannelMode(cm);

   // return the number of channels
   return cm.channels;
}

bool AudioFrameHeader::isBitrateChannelModeCombinationValid()
{
   bool rval = false;

   // get the layer
   AudioLayer layer;
   getLayer(layer);

   if(layer.type == AudioLayer::Layer2)
   {
      // get the bitrate
      int bitrate = getBitrate();

      // if the bitrate is free, 64000, or between 96000 and 192000 then
      // any channel mode is permissible
      if(bitrate == 0 || bitrate == 64000 ||
         (bitrate >= 96000 && bitrate <= 192000))
      {
         rval = true;
      }
      else
      {
         // get the channel mode
         AudioChannelMode cm;
         getChannelMode(cm);

         // if the channel mode is single channel, then the bitrate must
         // be between 32000 and 56000 or be 80000
         if(cm.type == AudioChannelMode::SingleChannel)
         {
            if((bitrate >= 32000 && bitrate <= 56000) || bitrate == 80000)
            {
               rval = true;
            }
         }
         else
         {
            // bitrates greater than 224000 and less than 384000 are valid
            // for stereo channel modes
            if(bitrate >= 224000 && bitrate <= 384000)
            {
               rval = true;
            }
         }
      }
   }
   else
   {
      // all combinations are valid for Layers I & III
      rval = true;
   }

   return rval;
}

void AudioFrameHeader::getChannelModeExtension(AudioChannelModeExtension& cme)
{
   // the channel mode extension is located in bits 5 and 4 for byte 3, so
   // shift to the right 4 and then AND with 0x03
   // with 00110000 >> 4 = 00000011 & 0x03 = 11
   AudioLayer layer;
   getLayer(layer);
   cme.setBitValues(layer, (getDataBytes()[3] >> 4) & 0x03);
}

int AudioFrameHeader::getJointStereoBound()
{
   // get the channel mode extension
   AudioChannelModeExtension cme;
   getChannelModeExtension(cme);

   // return the upper band
   return cme.upperBand;
}

void AudioFrameHeader::setCopyrighted(bool copyrighted)
{
   // the copyright bit is located in bit 3 for byte 3,
   // if enabling, then SET bit 3 by ORing with 0x08,
   // if disabling, then CLEAR bit 3 by ANDing with 0xF7
   if(copyrighted)
   {
      getDataBytes()[3] |= 0x08;
   }
   else
   {
      getDataBytes()[3] &= 0xf7;
   }
}

bool AudioFrameHeader::isCopyrighted()
{
   // the copyright bit is located in bit 3 for byte 3, so
   // AND against 8
   // is copyrighted if the bit is NOT cleared
   return (getDataBytes()[3] & 0x08) != 0;
}

void AudioFrameHeader::setOriginal(bool original)
{
   // the original bit is located in bit 2 for byte 3,
   // if enabling, then SET bit 2 by ORing with 0x04,
   // if disabling, then CLEAR bit 2 by ANDing with 0xFB
   if(original)
   {
      getDataBytes()[3] |= 0x04;
   }
   else
   {
      getDataBytes()[3] &= 0xfb;
   }
}

bool AudioFrameHeader::isOriginal()
{
   // the original bit is located in bit 2 for byte 3, so
   // AND against 4
   // is original if the bit is NOT cleared
   return (getDataBytes()[3] & 0x04) != 0;
}

void AudioFrameHeader::getEmphasis(AudioEmphasis& emphasis)
{
   // the emphasis is located in bits 1 and 0 for byte 3, so shift
   // AND with 0x03
   // with 00000011 & 0x03 = 11
   emphasis.setBitValues(getDataBytes()[3] & 0x03);
}

int AudioFrameHeader::getSideInformationLength()
{
   int rval = 0;

   // get version
   AudioVersion version;
   getVersion(version);

   // get channel count
   int channels = getChannelCount();

   if(version.type == AudioVersion::Mpeg1)
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

int AudioFrameHeader::getAudioDataLength()
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

int AudioFrameHeader::getFrameLength()
{
   int rval = 0;

   // get the version and layer for the frame
   AudioVersion version;
   getVersion(version);
   AudioLayer layer;
   getLayer(layer);

   // get the bitrate for the frame
   int bitrate = getBitrate();

   if(bitrate != 0)
   {
      // bitrate is not free format, so we know the frame length
      rval = calculateFrameLength(
         version, layer, bitrate, getSamplingRate(), isPadded());
   }
   else
   {
      // bitrate is free format, so we don't know the frame length,
      // use the maximum
      rval = calculateMaxFrameLength(version, layer);
   }

   return rval;
}

double AudioFrameHeader::getAudioLength()
{
   double rval = 0.0;

   // get the version and layer
   AudioVersion version;
   getVersion(version);
   AudioLayer layer;
   getLayer(layer);

   // calculate the audio length based on the version and layer
   switch(layer.type)
   {
      case AudioLayer::Layer1:
         rval = 384.0 / getSamplingRate();
         break;
      case AudioLayer::Layer2:
         rval = 1152.0 / getSamplingRate();
         break;
      case AudioLayer::Layer3:
         if(version.type == AudioVersion::Mpeg1)
         {
            rval = 1152.0 / getSamplingRate();
         }
         else
         {
            rval = 576.0 / getSamplingRate();
         }
         break;
      default:
         break;
   }

   return rval;
}

ByteBuffer* AudioFrameHeader::getBytes()
{
   if(mData == NULL)
   {
      mData = new ByteBuffer(4);

      // set the first and second bytes to frame sync values
      ((unsigned char*)mData->bytes())[0] = 0xff;
      ((unsigned char*)mData->bytes())[1] = 0xf0;

      // clear other bytes
      mData[2] = 0;
      mData[3] = 0;
   }

   return mData;
}

string AudioFrameHeader::toString()
{
   string str;

   // get audio characteristics
   AudioVersion v;
   AudioLayer l;
   AudioChannelMode cm;
   AudioChannelModeExtension cme;
   AudioEmphasis e;
   getVersion(v);
   getLayer(l);
   getChannelMode(cm);
   getChannelModeExtension(cme);
   getEmphasis(e);

   // for converting numbers to strings
   char temp[30];

   str.append("[MpegAudioFrameHeader]\n");
   str.append(v.name);
   str.push_back('\n');
   str.append(l.name);

   int bitrate = getBitrate();
   if(bitrate > 0)
   {
      str.append("\nBitrate: ");
      sprintf(temp, "%i bps", bitrate);
      str.append(temp);
   }
   else if(bitrate == 0)
   {
      str.append("\nBitrate: Free Format");
   }
   else
   {
      str.append("\nBitrate: Invalid");
   }

   int samplingRate = getSamplingRate();
   if(samplingRate != -1)
   {
      str.append("\nSampling Rate: ");
      sprintf(temp, "%i Hz", samplingRate);
   }
   else
   {
      str.append("\nSampling Rate: Invalid");
   }

   str.append("\nChannel Mode: ");
   str.append(cm.name);
   str.append("\nChannel Mode Extension: ");
   str.append(cme.name);
   str.append("\nEmphasis: ");
   str.append(e.name);

   str.append("\nCRC-16 Protected: ");
   str.append(isCrcEnabled() ? "true" : "false");
   str.append("\nPadded: ");
   str.append(isPadded() ? "true" : "false");
   str.append("\nPrivate Bit Set: ");
   str.append(isPrivateBitSet() ? "true" : "false");
   str.append("\nCopyrighted: ");
   str.append(isCopyrighted() ? "true" : "false");
   str.append("\nOriginal: ");
   str.append(isOriginal() ? "true" : "false");

   str.append("\nFrame Length: ");
   sprintf(temp, "%i bytes", getFrameLength());
   str.append(temp);

   str.append("\nAudio Length: ");
   sprintf(temp, "%f seconds", getAudioLength());
   str.append(temp);

   return str;
}

int AudioFrameHeader::calculateFrameLength(
   const AudioVersion& version, const AudioLayer& layer,
   double bitrate, double samplingRate, bool paddingEnabled)
{
   int rval = 0;

   // get the padding for the frame
   int padding = (paddingEnabled) ? 1 : 0;

   // calculate the frame length based on the version and layer
   switch(layer.type)
   {
      case AudioLayer::Layer1:
         rval = (int)floor((12 * (bitrate / samplingRate) + padding) * 4);
         break;
      case AudioLayer::Layer2:
         rval = (int)floor(144 * (bitrate / samplingRate) + padding);
         break;
      case AudioLayer::Layer3:
         if(version.type == AudioVersion::Mpeg1)
         {
            rval = (int)floor(144 * (bitrate / samplingRate) + padding);
         }
         else
         {
            rval = (int)floor(72 * (bitrate / samplingRate) + padding);
         }
         break;
      default:
         break;
   }

   return rval;
}

int AudioFrameHeader::calculateMinFrameLength(
   const AudioVersion& version, const AudioLayer& layer)
{
   int rval = 0;

   // get the minimum bitrate and maximum sampling rate
   int bitrate = sBitrateTable.getMinBitrate(version, layer);
   int samplingRate = sSamplingRateTable.getMaxSamplingRate(version);

   // calculate frame length, use padding
   rval = calculateFrameLength(version, layer, bitrate, samplingRate, true);

   return rval;
}

int AudioFrameHeader::calculateMaxFrameLength(
   const AudioVersion& version, const AudioLayer& layer)
{
   int rval = 0;

   // get the maximum bitrate and minimum sampling rate
   int bitrate = sBitrateTable.getMaxBitrate(version, layer);
   int samplingRate = sSamplingRateTable.getMinSamplingRate(version);

   // calculate frame length, use padding
   rval = calculateFrameLength(version, layer, bitrate, samplingRate, true);

   return rval;
}
