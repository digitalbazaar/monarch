/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_avi_AviHeader_H
#define monarch_data_avi_AviHeader_H

#include "monarch/io/OutputStream.h"
#include "monarch/data/riff/RiffChunkHeader.h"

namespace monarch
{
namespace data
{
namespace avi
{

/**
 * An AVI main header ('avih').
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
 * -------------------------------------------
 * In the AVI Header
 * (a DWORD is 4 bytes, 14 DWORDS = 56 bytes):
 * -------------------------------------------
 * DWORD microseconds per frame
 * DWORD maximum bytes per second
 * DWORD padding granularity
 * DWORD flags
 * DWORD total frames
 * DWORD initial frames (how far in front audio data is of video data)*
 * DWORD streams - 2 for video+audio streams
 * DWORD suggested buffer size - large enough to contain largest chunk
 * DWORD width - in pixels
 * DWORD height - in pixels
 * DWORD scale - time scale in samples per second = rate / scale
 * DWORD rate - see scale
 * DWORD start - starting time for the AVI file, usually zero
 * DWORD length - total time of the file using rate & scale units
 *
 * The flags for the AVI Header 'avih':
 *
 * AVIF_HASINDEX (bit 4) - indicates whether or not the AVI has an Index Chunk
 *
 * AVIF_MUSTUSEINDEX (bit 5) - indicates whether or not the index should
 * determine the order of the data
 *
 * AVIF_ISINTERLEAVED (bit 8) - indicates whether or not the file is
 * interleaved
 *
 * AVIF_WASCAPTUREFILE (bit 16) - indicates whether or not the file is used for
 * capturing real-time video
 *
 * AVIF_COPYRIGHTED (bit 17) - indicates whether or not the file contains
 * copyrighted data
 *
 * *Note: The initial frames data member specifies the number of frames
 * the audio data is ahead of the video data. Typically, audio data should
 * be moved forward enough frames to allow 0.75 seconds of audio data to
 * be preloaded before the video data is loaded. This is only for interleaved
 * files -- this should be ZERO for non-interleaved files.
 *
 * @author Dave Longley
 * @author David I. Lehn
 */
class AviHeader
{
public:
   /**
    * Size of this header.
    */
   static const int HEADER_SIZE = 56;

   /**
    * Chunk fourcc id "avih".
    */
   static const fourcc_t CHUNK_ID = DB_FOURCC_FROM_CHARS('a','v','i','h');

   /**
    * Indicates whether or not the AVI has an Index Chunk.
    */
   static const uint32_t AVIF_HASINDEX = 0x00000010;

   /**
    * Indicates whether or not the index should determine the order of
    * the data.
    */
   static const uint32_t AVIF_MUSTUSEINDEX = 0x00000020;

   /**
    * Indicates whether or not the file is interleaved.
    */
   static const uint32_t AVIF_ISINTERLEAVED = 0x00000100;

   /**
    * Indicates whether or not the file is used for capturing real-time video.
    */
   static const uint32_t AVIF_WASCAPTUREFILE = 0x00010000;

   /**
    * Indicates whether or not the file contains copyrighted data.
    */
   static const uint32_t AVIF_COPYRIGHTED = 0x00020000;

protected:
   /**
    * The AVI header RIFF header.
    */
   monarch::data::riff::RiffChunkHeader mRiffHeader;

   /**
    * The data for this chunk, not including its header.
    */
   char mData[HEADER_SIZE];

   /**
    * Reads a DWORD from the internal data and discards an IO error.
    *
    * @param offset the offset to read from.
    *
    * @return the read DWORD.
    */
   uint32_t readDWord(int offset);

public:
   /**
    * Constructs a new AviHeader.
    */
   AviHeader();

   /**
    * Destructs an AviHeader.
    */
   virtual ~AviHeader();

   /**
    * Writes this AviHeader, including the RIFF header, to an OutputStream.
    *
    * @param os the OutputStream to write to.
    *
    * @return true on success, false on IO exception.
    */
   virtual bool writeTo(monarch::io::OutputStream& os);

   /**
    * Converts this AviHeader from a byte array.
    *
    * @param b the byte array to convert from.
    * @param length the number of valid bytes in the buffer.
    *
    * @return true if successful, false if not.
    */
   virtual bool convertFromBytes(const char* b, int length);

   /**
    * Returns whether or not this header is valid.
    *
    * @return true if valid, false if not.
    */
   virtual bool isValid();

   /**
    * Gets the size of this AviHeader, excluding its chunk header.
    *
    * @return the size of this AviHeader chunk.
    */
   virtual int getChunkSize();

   /**
    * Gets the size of this AviHeader including its chunk header.
    *
    * @return the size of this AviHeader.
    */
   virtual int getSize();

   /**
    * Gets the number of microseconds per frame.
    *
    * @return the number of microseconds per frame.
    */
   virtual uint32_t getMicrosecondsPerFrame();

   /**
    * Gets the maximum number of bytes per second.
    *
    * @return the maximum number of bytes per second.
    */
   virtual uint32_t getMaxBytesPerSecond();

   /**
    * Gets the padding granularity (typically 2048). Data should be padded
    * to multiples of this value.
    *
    * @return the padding granularity (typically 2048).
    */
   virtual uint32_t getPaddingGranularity();

   /**
    * Returns true if the AVIF_HASINDEX flag is on.
    *
    * @return true if the AVIF_HASINDEX flag is on, false if not.
    */
   virtual bool isAviHasIndex();

   /**
    * Returns true if the AVIF_MUSTUSEINDEX flag is on.
    *
    * @return true if the AVIF_MUSTUSEINDEX flag is on, false if not.
    */
   virtual bool isAviMustUseIndex();

   /**
    * Returns true if the AVIF_ISINTERLEAVED flag is on.
    *
    * @return true if the AVIF_ISINTERLEAVED flag is on, false if not.
    */
   virtual bool isAviIsInterleaved();

   /**
    * Returns true if the AVIF_WASCAPTUREFILE flag is on.
    *
    * @return true if the AVIF_WASCAPTUREFILE flag is on, false if not.
    */
   virtual bool isAviWasCaptureFile();

   /**
    * Returns true if the AVIF_COPYRIGHTED flag is on.
    *
    * @return true if the AVIF_COPYRIGHTED flag is on, false if not.
    */
   virtual bool isAviCopyrighted();

   /**
    * Gets the total frames.
    *
    * @return the total frames.
    */
   virtual uint32_t getTotalFrames();

   /**
    * Gets the initial frames.
    *
    * @return the initial frames.
    */
   virtual uint32_t getInitialFrames();

   /**
    * Gets the number of streams.
    *
    * @return the number of streams.
    */
   virtual uint32_t getStreamCount();

   /**
    * Gets the suggested playback buffer size in bytes.
    *
    * @return the suggested playback buffer size in bytes.
    */
   virtual uint32_t getSuggestedBufferSize();

   /**
    * Gets the width in pixels.
    *
    * @return the width in pixels.
    */
   virtual uint32_t getWidth();

   /**
    * Gets the height in pixels.
    *
    * @return the height in pixels.
    */
   virtual uint32_t getHeight();

   /**
    * Gets the time scale (typically 30).
    *
    * @return the time scale (typically 30).
    */
   virtual uint32_t getTimeScale();

   /**
    * Gets the data rate.
    *
    * @return the data rate.
    */
   virtual uint32_t getDataRate();

   /**
    * Gets the frame rate (data rate / time scale).
    *
    * @return the frame rate (data rate / time scale).
    */
   virtual double getFrameRate();

   /**
    * Gets the starting time in time scale units (typically 0).
    *
    * @return the starting time in time scale units (typically 0).
    */
   virtual uint32_t getStartingTime();

   /**
    * Gets the video length in time scale units.
    *
    * @return the video length in time scale units.
    */
   virtual uint32_t getVideoLength();

   /**
    * Gets the time as calculated from microseconds/frame * total frames.
    *
    * @return the time as calculated from microseconds/frame * total frames.
    */
   virtual uint32_t getTotalTime();

   /**
    * Gets the time, in seconds, as calculated from microseconds/frame * total
    * frames.
    *
    * @return the time, in seconds, as calculated from microseconds/frame *
    *         total frames.
    */
   virtual double getTotalSeconds();
};

} // end namespace avi
} // end namespace data
} // end namespace monarch
#endif
