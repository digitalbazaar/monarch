/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_avi_AviStreamFormat_H
#define monarch_data_avi_AviStreamFormat_H

#include "monarch/data/riff/RiffChunkHeader.h"
#include "monarch/io/OutputStream.h"

namespace monarch
{
namespace data
{
namespace avi
{

/**
 * An AVI Stream Format ('strf').
 *
 * AVI Format is as follows:
 *
 * AVI Form Header ('RIFF' size 'AVI ' data)
 *    Header List ('LIST' size 'hdrl' data)
 *       AVI Header ('avih' size data)
 *          Video Stream Header List ('LIST' size 'strl' data)
 *             Video Stream Header ('strh' size data)
 *             Video Stream Format ('strf' size data)
 *             Video Stream Data ('strd' size data) - IGNORED, for DRIVERS
 *          Audio Stream Header List ('LIST' size 'strl' data)
 *             Audio Stream Header ('strh' size data)
 *             Audio Stream Format ('strf' size data)
 *             Audio Stream Data ('strd' size data) - IGNORED, for DRIVERS
 *    Info List ('LIST' size 'INFO' data)
 *       Index Entry ({'ISBJ','IART','ICMT',...} size data)
 *    Movie List ('LIST' size 'movi' data)
 *       Movie Entry ({'00db','00dc','01wb'} size data)
 *    Index Chunk ('idx1' size data)
 *       Index Entry ({'00db','00dc','01wb',...})
 *
 * --------------------------
 * In a Stream Format 'strf':
 * --------------------------
 * A BITMAPINFO structure for a Video Stream Format chunk.
 * A WAVEFORMATEX or PCMWAVEFORMAT structure for an Audio Stream Format chunk.
 *
 * @author Dave Longley
 * @author David I. Lehn
 */
class AviStreamFormat
{
public:
   /**
    * Chunk fourcc id "strf".
    */
   static const fourcc_t CHUNK_ID = DB_FOURCC_FROM_CHARS('s','t','r','f');

protected:
   /**
    * The AVI stream format RIFF header.
    */
   monarch::data::riff::RiffChunkHeader mRiffHeader;

   /**
    * The data for this chunk, not including its header.
    */
   char* mData;

public:
   /**
    * Constructs a new AviStreamFormat.
    */
   AviStreamFormat();

   /**
    * Destructs a AviStreamFormat.
    */
   virtual ~AviStreamFormat();

   /**
    * Writes this AviStreamFormat, including the RIFF header, to an
    * OutputStream.
    *
    * @param os the OutputStream to write to.
    *
    * @exception true on success, false on an Exception.
    */
   virtual bool writeTo(monarch::io::OutputStream& os);

   /**
    * Converts this AviStreamFormat from a byte array.
    *
    * @param b the byte array to convert from.
    * @param length the number of valid bytes in the buffer.
    *
    * @return true if successful, false if not.
    */
   virtual bool convertFromBytes(const char* b, int length);

   /**
    * Returns whether or not this AviStreamFormat is valid.
    *
    * @return true if valid, false if not.
    */
   virtual bool isValid();

   /**
    * Gets the size of this AviStreamFormat, excluding its chunk header.
    *
    * @return the size of this AviStreamFormat chunk.
    */
   virtual int getChunkSize();

   /**
    * Gets the size of this AviStreamFormat, including its chunk header.
    *
    * @return the size of this AviStreamFormat.
    */
   virtual int getSize();

   /**
    * Gets the data in this stream format. This data can be parsed into a
    * BitMapInfo or WaveFormatEx structure.
    *
    * @return the data in this stream format.
    */
   virtual const char* getData();
};

} // end namespace avi
} // end namespace data
} // end namespace monarch
#endif
