/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/mpeg/AudioCrc16.h"

#include <math.h>

using namespace db::data::mpeg;

AudioCrc16::AudioCrc16()
{
   // reset
   reset();
}

AudioCrc16::~AudioCrc16()
{
}

void AudioCrc16::reset()
{
   // set CRC value to maximum for mpeg audio CRC-16
   mCrcValue = 0xffff;
}

int AudioCrc16::getAudioDataAmount(AudioFrameHeader* header)
{
   int rval = 0;
   
   // determine the number of bits of audio data to use in the CRC
   int audioDataBits = 0;
   
   // get the channel count and stereo subband bound
   int channels = header->getChannelCount();
   int bound = header->getJointStereoBound();
   
   // how many bits to use depends on the layer type
   AudioLayer layer;
   header->getLayer(layer);
   switch(layer.type)
   {
      case AudioLayer::Layer1:
         // layer I determines the amount of data to pass through the
         // CRC algorithm by multiplying stereo bands by 2 channels
         // and adding them to mono bands -- then multiplying by 4
         // this algorithm simplifies that calculation ... when
         // no stereo is used the bound is set to 32
         audioDataBits = 4 * (channels * bound + (32 - bound));
         break;
      case AudioLayer::Layer2:
         // assume CRC is correct, not implemented
         break;
      case AudioLayer::Layer3:
         // layer III uses side information for the CRC (x8 to get bits)
         audioDataBits = header->getSideInformationLength() * 8;
         break;
      default:
         // do nothing, no audio data bits
         break;
   }
   
   if(audioDataBits > 0)
   {
      // determine the number of audio data bytes (round up)
      rval = (int)round(((double)audioDataBits / 8));
   }
   
   return rval;      
}

int AudioCrc16::calculateCrc(AudioFrameHeader* header, const char* audioData)
{
   // reset the CRC value
   reset();
   
   // update the CRC with the last 2 header bytes
   update(header->getBytes()->bytes() + 2, 2);
   
   // update the CRC with the audio data amount
   update(audioData, getAudioDataAmount(header));
   
   // return the CRC check sum
   return getChecksum();
}
