/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/mpeg/AudioSamplingRateTable.h"

#include <cstdlib>

using namespace std;
using namespace db::data::mpeg;

AudioSamplingRateTable::AudioSamplingRateTable()
{
   // add the sampling rate table entries:
   AudioVersion mpeg1(AudioVersion::Mpeg1);
   AudioVersion mpeg2(AudioVersion::Mpeg2);
   AudioVersion mpeg25(AudioVersion::Mpeg25);

   // add sampling rates for Version 1
   addSamplingRate(0x00, mpeg1, 44100);
   addSamplingRate(0x01, mpeg1, 48000);
   addSamplingRate(0x02, mpeg1, 32000);

   // add sampling rates for Version 2
   addSamplingRate(0x00, mpeg2, 22050);
   addSamplingRate(0x01, mpeg2, 24000);
   addSamplingRate(0x02, mpeg2, 16000);

   // add sampling rates for Version 2.5
   addSamplingRate(0x00, mpeg25, 11025);
   addSamplingRate(0x01, mpeg25, 12000);
   addSamplingRate(0x02, mpeg25, 8000);
}

AudioSamplingRateTable::~AudioSamplingRateTable()
{
   // clean up keys in table
   for(SamplingRateMap::iterator i = mMap.begin(); i != mMap.end(); i++)
   {
      free((char*)i->first);
   }

   // clear map
   mMap.clear();
}

void AudioSamplingRateTable::addSamplingRate(
   unsigned char index, const AudioVersion& version, int samplingRate)
{
   // build key from index and version
   unsigned char* key = (unsigned char*)malloc(2);
   key[0] = index;
   key[1] = version.bitValues;

   // update key if it already exists, otherwise add it
   SamplingRateMap::iterator i = mMap.find(key);
   if(i != mMap.end())
   {
      free(key);
      i->second = samplingRate;
   }
   else
   {
      mMap[key] = samplingRate;
   }
}

int AudioSamplingRateTable::getSamplingRate(
   unsigned char index, const AudioVersion& version) const
{
   int rval = -1;

   unsigned char key[2];
   key[0] = index;
   key[1] = version.bitValues;

   SamplingRateMap::const_iterator i = mMap.find(key);
   if(i != mMap.end())
   {
      rval = i->second;
   }

   return rval;
}

int AudioSamplingRateTable::getMinSamplingRate(
   const AudioVersion& version) const
{
   return getSamplingRate(0x02, version);
}

int AudioSamplingRateTable::getMaxSamplingRate(
   const AudioVersion& version) const
{
   return getSamplingRate(0x01, version);
}
