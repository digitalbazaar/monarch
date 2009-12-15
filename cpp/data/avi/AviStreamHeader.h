/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_avi_AviStreamHeader_H
#define monarch_data_avi_AviStreamHeader_H

#include "monarch/data/riff/RiffChunkHeader.h"
#include "monarch/io/OutputStream.h"

namespace monarch
{
namespace data
{
namespace avi
{

/**
 * An AVI Stream Header ('strh').
 *
 * AVI Format is as follows:
 *
 * AVI Form Header ('RIFF' size 'AVI ' data)
 *    Header List ('LIST' size 'hdrl' data)
 *       AVI Header ('avih' size data)
 *       Video Stream Header List ('LIST' size 'strl' data)
 *          Video Stream Header ('strh' size data)
 *          Video Stream Format ('strf' size data)
 *          Video Stream Data ('strd' size data) - IGNORED, for DRIVERS
 *       Audio Stream Header List ('LIST' size 'strl' data)
 *          Audio Stream Header ('strh' size data)
 *          Audio Stream Format ('strf' size data)
 *          Audio Stream Data ('strd' size data) - IGNORED, for DRIVERS
 *    Info List ('LIST' size 'INFO' data)
 *       Index Entry ({'ISBJ','IART','ICMT',...} size data)
 *    Movie List ('LIST' size 'movi' data)
 *       Movie Entry ({'00db','00dc','01wb'} size data)
 *    Index Chunk ('idx1' size data)
 *       Index Entry ({'00db','00dc','01wb',...})
 *
 * -----------------------------------------------------------------
 * In a Stream Header 'strh'
 * (FOURCC means a four-character code, 4 bytes each)
 * (a DWORD is 4 bytes)
 * (10 DWORDS = 40 bytes + 2 FOURCC (8 bytes) + 8 bytes = 56 bytes):
 * -----------------------------------------------------------------
 * FOURCC type - 'vids' video, 'auds' audio, 'txts' text, 'mids' MIDI
 * FOURCC handler - the installable compressor or decompressor for the data
 * DWORD flags
 * DWORD reserved (WORD Priority, WORD Language)
 * DWORD initial frames (how far audio data is ahead of video data)
 * DWORD scale*
 * DWORD rate
 * DWORD start
 * DWORD length
 * DWORD suggested buffer size
 * DWORD quality
 * DWORD sample size
 * 4 shorts (8 bytes) for a rectangular frame (left, top, right, bottom),
 * units are in pixels and relative to the upper-left corner of the entire
 * movie rectangle.
 *
 * The flags for the Stream Header:
 *
 * AVISF_DISABLED - whether or not the data should only be rendered when
 * explicitly enabled by the user
 *
 * AVISF_VIDEO_PALCHANGES - whether or not palette changes are embedded
 * in the file (chunks tagged like '00pc')
 *
 * *Note: Dividing the rate by the scale gives the number of samples per
 * second. This is the frame rate for video streams. For audio streams,
 * this rate corresponds to the time required to play "nBlockAlign" bytes of
 * audio. "nBlockAlign" is a data member of the WAVEFORMATEX structure
 * that describes audio.
 *
 * @author Dave Longley
 * @author David I. Lehn
 */
class AviStreamHeader
{
public:
   /**
    * Size of this header.
    */
   static const int HEADER_SIZE = 56;

   /**
    * Chunk fourcc id "strh".
    */
   static const fourcc_t CHUNK_ID = MO_FOURCC_FROM_CHARS('s','t','r','h');

protected:
   /**
    * The AVI stream header RIFF header.
    */
   monarch::data::riff::RiffChunkHeader mRiffHeader;

   /**
    * The data for this chunk, not including its header.
    */
   char mData[HEADER_SIZE];

public:
   /**
    * Constructs a new AviStreamHeader.
    */
   AviStreamHeader();

   /**
    * Destructs a AviStreamHeader.
    */
   virtual ~AviStreamHeader();

   /**
    * Writes this AviStreamHeader, including the RIFF header, to an
    * OutputStream.
    *
    * @param os the OutputStream to write to.
    *
    * @return true on success, false on an Exception.
    */
   virtual bool writeTo(monarch::io::OutputStream& os);

   /**
    * Converts this AviStreamHeader from a byte array.
    *
    * @param b the byte array to convert from.
    * @param length the number of valid bytes in the buffer.
    *
    * @return true if successful, false if not.
    */
   virtual bool convertFromBytes(const char* b, int length);

   /**
    * Returns whether or not this AviStreamHeader is valid.
    *
    * @return true if valid, false if not.
    */
   virtual bool isValid();

   /**
    * Gets the size of this AviStreamHeader, excluding its chunk header.
    *
    * @return the size of this AviStreamHeader chunk.
    */
   virtual int getChunkSize();

   /**
    * Gets the size of this AviStreamHeader, including its chunk header.
    *
    * @return the size of this AviStreamHeader.
    */
   virtual int getSize();
};

} // end namespace avi
} // end namespace data
} // end namespace monarch
#endif
