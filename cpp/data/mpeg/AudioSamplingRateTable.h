/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_mpeg_SamplingRateTable_H
#define monarch_data_mpeg_SamplingRateTable_H

#include <map>
#include "monarch/data/mpeg/AudioCharacteristics.h"

#include <cstring>

namespace monarch
{
namespace data
{
namespace mpeg
{

/**
 * An AudioSamplingRateTable maps Sampling Rate Indices (as bit values in this
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
class AudioSamplingRateTable
{
protected:
   /**
    * A KeyComparator compares the keys for the sample rate map.
    */
   struct KeyComparator
   {
      /**
       * Compares two map keys using a string compare, returning true if the
       * first key is less than the second, false if not.
       *
       * @param k1 the first key.
       * @param k2 the second key.
       *
       * @return true if the k1 < k2, false if not.
       */
      bool operator()(const unsigned char* k1, const unsigned char* k2) const
      {
         return memcmp(k1, k2, 2) < 0;
      }
   };

   /**
    * The underlying hash map used to map indices and versions to sampling
    * rates.
    */
   typedef std::map<const unsigned char*, int, KeyComparator> SamplingRateMap;
   SamplingRateMap mMap;

   /**
    * Adds a sampling rate entry for the given index and version.
    *
    * @param index the bit values for the bitrate index.
    * @param version the MPEG version.
    * @param samplingRate the sampling rate (in Hz).
    */
   virtual void addSamplingRate(
      unsigned char index, const AudioVersion& version, int samplingRate);

public:
   /**
    * Creates a new SamplingRateTable.
    */
   AudioSamplingRateTable();

   /**
    * Destructs this SamplingRateTable.
    */
   virtual ~AudioSamplingRateTable();

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
   virtual int getSamplingRate(
      unsigned char index, const AudioVersion& version) const;

   /**
    * Gets the minimum sampling rate for the given version.
    *
    * @param version the MPEG version.
    *
    * @return the minimum sampling rate (in samples/second).
    */
   virtual int getMinSamplingRate(const AudioVersion& version) const;

   /**
    * Gets the maximum sampling rate for the given version.
    *
    * @param version the MPEG version.
    *
    * @return the maximum sampling rate (in samples/second).
    */
   virtual int getMaxSamplingRate(const AudioVersion& version) const;
};

} // end namespace mpeg
} // end namespace data
} // end namespace monarch
#endif
