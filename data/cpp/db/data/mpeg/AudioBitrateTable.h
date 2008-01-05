/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_mpeg_AudioBitrateTable_H
#define db_data_mpeg_AudioBitrateTable_H

#include <map>
#include "db/data/mpeg/AudioCharacteristics.h"

namespace db
{
namespace data
{
namespace mpeg
{

/**
 * An AudioBitrateTable maps Bitrate Indices (as bit values in this header) to
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
class AudioBitrateTable
{
protected:
   /**
    * A KeyComparator compares the keys for the bitrate map.
    */
   typedef struct KeyComparator
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
         return memcmp(k1, k2, 3) < 0;
      }
   };
   
   /**
    * The underlying map used to map indices, versions, and layers to bitrates.
    */
   typedef std::map<const unsigned char*, int, KeyComparator> BitrateMap;
   BitrateMap mMap;
   
   /**
    * Adds a bitrate entry for the given index, version, and layer.
    * 
    * @param index the bit values for the bitrate index.
    * @param version the MPEG version.
    * @param layer the MPEG layer.
    * @param bitrate the bitrate with 0 indicating a free format bitrate.
    */
   virtual void addBitrate(
      unsigned char index, const AudioVersion& version,
      const AudioLayer& layer, int bitrate);
   
public:
   /**
    * Creates a new AudioBitrateTable.
    */
   AudioBitrateTable();
   
   /**
    * Destructs this AudioBitrateTable.
    */
   virtual ~AudioBitrateTable();
   
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
   virtual int getBitrate(
      unsigned char index,
      const AudioVersion& version, const AudioLayer& layer) const;
   
   /**
    * Gets the minimum non-free-format bitrate for the given version and
    * layer.
    * 
    * @param version the MPEG version.
    * @param layer the MPEG layer type.
    * 
    * @return the minimum non-free-format bitrate (in bits/second).
    */
   virtual int getMinBitrate(
      const AudioVersion& version, const AudioLayer& layer) const;
   
   /**
    * Gets the maximum bitrate for the given version and layer.
    * 
    * @param version the MPEG version.
    * @param layer the MPEG layer type.
    * 
    * @return the maximum bitrate (in bits/second).
    */
   virtual int getMaxBitrate(
      const AudioVersion& version, const AudioLayer& layer) const;
};

} // end namespace mpeg
} // end namespace data
} // end namespace db
#endif
