/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/mpeg/AudioBitrateTable.h"

using namespace std;
using namespace db::data::mpeg;

AudioBitrateTable::AudioBitrateTable()
{
   // add the bitrate table entries:
   AudioVersion mpeg1(AudioVersion::Mpeg1);
   AudioVersion mpeg2(AudioVersion::Mpeg2);
   AudioLayer layer1(AudioLayer::Layer1);
   AudioLayer layer2(AudioLayer::Layer2);
   AudioLayer layer3(AudioLayer::Layer3);
   
   // add bitrates for Version 1 Layer I
   for(char index = 0x00; index < 0x0f; index++)
   {
      addBitrate(index, mpeg1, layer1, index * 32);
   }
   
   // FIXME: these bitrates could be added in simple loops,
   // but do we want to do that -- or is it easier to read as is?
   // do we care about readability in the code for these values?
   
   // add bitrates for Version 1 Layer II
   addBitrate(0x00, mpeg1, layer2, 0);
   addBitrate(0x01, mpeg1, layer2, 32);
   addBitrate(0x02, mpeg1, layer2, 48);
   addBitrate(0x03, mpeg1, layer2, 56);
   addBitrate(0x04, mpeg1, layer2, 64);
   addBitrate(0x05, mpeg1, layer2, 80);
   addBitrate(0x06, mpeg1, layer2, 96);
   addBitrate(0x07, mpeg1, layer2, 112);
   addBitrate(0x08, mpeg1, layer2, 128);
   addBitrate(0x09, mpeg1, layer2, 160);
   addBitrate(0x0a, mpeg1, layer2, 192);
   addBitrate(0x0b, mpeg1, layer2, 224);
   addBitrate(0x0c, mpeg1, layer2, 256);
   addBitrate(0x0d, mpeg1, layer2, 320);
   addBitrate(0x0e, mpeg1, layer2, 384);
   
   // add bitrates for Version 1 Layer III
   addBitrate(0x00, mpeg1, layer3, 0);
   addBitrate(0x01, mpeg1, layer3, 32);
   addBitrate(0x02, mpeg1, layer3, 40);
   addBitrate(0x03, mpeg1, layer3, 48);
   addBitrate(0x04, mpeg1, layer3, 56);
   addBitrate(0x05, mpeg1, layer3, 64);
   addBitrate(0x06, mpeg1, layer3, 80);
   addBitrate(0x07, mpeg1, layer3, 96);
   addBitrate(0x08, mpeg1, layer3, 112);
   addBitrate(0x09, mpeg1, layer3, 128);
   addBitrate(0x0a, mpeg1, layer3, 160);
   addBitrate(0x0b, mpeg1, layer3, 192);
   addBitrate(0x0c, mpeg1, layer3, 224);
   addBitrate(0x0d, mpeg1, layer3, 256);
   addBitrate(0x0e, mpeg1, layer3, 320);
   
   // add bitrates for Version 2 Layer I
   addBitrate(0x00, mpeg2, layer1, 0);
   addBitrate(0x01, mpeg2, layer1, 32);
   addBitrate(0x02, mpeg2, layer1, 48);
   addBitrate(0x03, mpeg2, layer1, 56);
   addBitrate(0x04, mpeg2, layer1, 64);
   addBitrate(0x05, mpeg2, layer1, 80);
   addBitrate(0x06, mpeg2, layer1, 96);
   addBitrate(0x07, mpeg2, layer1, 112);
   addBitrate(0x08, mpeg2, layer1, 128);
   addBitrate(0x09, mpeg2, layer1, 144);
   addBitrate(0x0a, mpeg2, layer1, 160);
   addBitrate(0x0b, mpeg2, layer1, 176);
   addBitrate(0x0c, mpeg2, layer1, 192);
   addBitrate(0x0d, mpeg2, layer1, 224);
   addBitrate(0x0e, mpeg2, layer1, 256);
   
   // add bitrates for Version 2 Layer II
   addBitrate(0x00, mpeg2, layer2, 0);
   addBitrate(0x01, mpeg2, layer2, 8);
   addBitrate(0x02, mpeg2, layer2, 16);
   addBitrate(0x03, mpeg2, layer2, 24);
   addBitrate(0x04, mpeg2, layer2, 32);
   addBitrate(0x05, mpeg2, layer2, 40);
   addBitrate(0x06, mpeg2, layer2, 48);
   addBitrate(0x07, mpeg2, layer2, 56);
   addBitrate(0x08, mpeg2, layer2, 64);
   addBitrate(0x09, mpeg2, layer2, 80);
   addBitrate(0x0a, mpeg2, layer2, 96);
   addBitrate(0x0b, mpeg2, layer2, 112);
   addBitrate(0x0c, mpeg2, layer2, 128);
   addBitrate(0x0d, mpeg2, layer2, 144);
   addBitrate(0x0e, mpeg2, layer2, 160);
   
   // add bitrates for Version 2 Layer III
   addBitrate(0x00, mpeg2, layer3, 0);
   addBitrate(0x01, mpeg2, layer3, 8);
   addBitrate(0x02, mpeg2, layer3, 16);
   addBitrate(0x03, mpeg2, layer3, 24);
   addBitrate(0x04, mpeg2, layer3, 32);
   addBitrate(0x05, mpeg2, layer3, 40);
   addBitrate(0x06, mpeg2, layer3, 48);
   addBitrate(0x07, mpeg2, layer3, 56);
   addBitrate(0x08, mpeg2, layer3, 64);
   addBitrate(0x09, mpeg2, layer3, 80);
   addBitrate(0x0a, mpeg2, layer3, 96);
   addBitrate(0x0b, mpeg2, layer3, 112);
   addBitrate(0x0c, mpeg2, layer3, 128);
   addBitrate(0x0d, mpeg2, layer3, 144);
   addBitrate(0x0e, mpeg2, layer3, 160);
}

AudioBitrateTable::~AudioBitrateTable()
{
   // clean up keys in table
   for(BitrateMap::iterator i = mMap.begin(); i != mMap.end(); i++)
   {
      free((char*)i->first);
   }
   
   // clear map
   mMap.clear();
}

void AudioBitrateTable::addBitrate(
   char index, const AudioVersion& version,
   const AudioLayer& layer, int bitrate)
{
   // build key from index, version, and layer
   char* key = (char*)malloc(3);
   
   key[0] = index;
   key[1] = version.bitValues;
   key[2] = layer.bitValues;
   
   // update key if it already exists, otherwise add it
   BitrateMap::iterator i = mMap.find(key);
   if(i != mMap.end())
   {
      free(key);
      i->second = bitrate;
   }
   else
   {
      mMap[key] = bitrate;
   }
}

int AudioBitrateTable::getBitrate(
   char index, const AudioVersion& version, const AudioLayer& layer) const
{
   int rval = -1;
   
   // MPEG version 2.5 uses the same bitrate table as MPEG version 2
   AudioVersion v = version;
   if(v.type == AudioVersion::Mpeg25)
   {
      v.setType(AudioVersion::Mpeg2);
   }
   
   // find bitrate
   char key[3];
   key[0] = index;
   key[1] = v.bitValues;
   key[2] = layer.bitValues;
   
   BitrateMap::const_iterator i = mMap.find(key);
   if(i != mMap.end())
   {
      // convert from kilobits to bits
      rval = i->second * 1000;
   }
   
   return rval;
}

int AudioBitrateTable::getMinBitrate(
   const AudioVersion& version, const AudioLayer& layer) const
{
   return getBitrate(0x01, version, layer);
}

int AudioBitrateTable::getMaxBitrate(
   const AudioVersion& version, const AudioLayer& layer) const
{
   return getBitrate(0x0e, version, layer);
}
