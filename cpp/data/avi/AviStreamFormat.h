/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
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
 * A WAVEFORMATEX structure for an Audio Stream Format chunk.
 *
 * A BITMAPINFOHEADER structure:
 *
 * -------------------------------------------------------------------------
 * (a DWORD is 4 bytes, a WORD is 2 bytes, a LONG is 4 bytes)
 * (5 DWORDS = 20 bytes + 2 WORDS (4 bytes) + 4 LONGS (16 bytes) = 40 bytes:
 * -------------------------------------------------------------------------
 * DWORD size - the size of the structure, in bytes (EXCLUDES color table/masks)
 * LONG width - width of the bitmap in pixels
 * LONG height - height of the bitmap in pixels
 * WORD planes - the number of planes for the device, SET TO 1
 * WORD bit count - the number of bits per pixel
 * DWORD compression - the type of compression
 * DWORD image size - the size, in bytes, of the image
 * LONG X pixels per meter - horizontal resolution
 * LONG Y pixels per meter - vertical resolution
 * DWORD number of color indices in the color table, zero = maximum
 * DWORD number of color indices required for displaying the map, zero = all
 *
 * A WAVEFORMATEX structure:
 * 5 WORDS (10 bytes) + 2 DWORDS (8 bytes) = 18 bytes
 *
 * WORD formatTag - the waveform-audio format type
 * WORD nChannels - the number of channels
 * DWORD nSamplesPerSec - samples per second (hz)
 * DWORD nAvgBytesPerSec - required average bytes/second for transfer rate
 * WORD nBlockAlign - block alignment (minimum atomic unit for format type)
 * WORD wBitsPerSample - the number of bits per sample
 * WORD cbSize - the size of the extra format information
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
   static const fourcc_t CHUNK_ID = MO_FOURCC_FROM_CHARS('s','t','r','f');

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
    * Gets the size of BitmapInfo structure excluding the color table/masks.
    *
    * @return the size of the BitmapInfo structure.
    */
   virtual uint32_t getBitmapInfoSize();

   /**
    * Gets the width of the bitmap in pixels.
    *
    * @return the width of the bitmunk in pixels.
    */
   virtual uint32_t getBitmapWidth();

   /**
    * Gets the height of the bitmap in pixels.
    *
    * @return the height of the bitmap in pixels.
    */
   virtual uint32_t getBitmapHeight();

   /**
    * Gets the number of bits per pixel.
    *
    * @return the number of bits per pixel.
    */
   virtual uint16_t getBitCount();

   /**
    * Gets the type of compression.
    *
    * @return the type of compression.
    */
   virtual uint32_t getCompression();

   /**
    * Gets the image size in bytes.
    *
    * @return the image size in bytes.
    */
   virtual uint32_t getImageSize();

   /**
    * Gets the horizontal resolution in pixels per meter.
    *
    * @return the horizontal resolution in pixels per meter.
    */
   virtual uint32_t getHorizontalResolution();

   /**
    * Gets the vertical resolution in pixels per meter.
    *
    * @return the vertical resolution in pixels per meter.
    */
   virtual uint32_t getVerticalResolution();

   /**
    * Gets the number of color indices (0 for all).
    *
    * @return the number of color indices (0 for all).
    */
   virtual uint32_t getColorInidices();

   /**
    * Gets the number of color indices required (0 for all).
    *
    * @return the number of color indices required (0 for all).
    */
   virtual uint32_t getColorInidicesRequired();

   /**
    * Gets the format tag.
    *
    * @return the format tag.
    */
   virtual uint16_t getAudioFormatTag();

   /**
    * Gets the number of audio channels.
    *
    * @return the number of audio channels.
    */
   virtual uint16_t getAudioChannels();

   /**
    * Gets the number of audio samples per second (hz).
    *
    * @return the number of audio samples per second (hz).
    */
   virtual uint32_t getAudioSamplesPerSecond();

   /**
    * Gets the required average transfer rate in bytes/second.
    *
    * @return the required average transfer rate in bytes/second.
    */
   virtual uint32_t getAudioAvgBytesPerSecond();

   /**
    * Gets the block alignment (minimum atomic unit for the format type).
    *
    * @return the block alignment (minimum atomic unit for the format type).
    */
   virtual uint16_t getBlockAligmentUnit();

   /**
    * Gets the number of audio bits per sample.
    *
    * @return the number of audio bits per sample.
    */
   virtual uint16_t getAudioBitsPerSample();

   /**
    * Gets the size of the extra format information.
    *
    * @return the size of the extra format information.
    */
   virtual uint16_t getExtraAudioInfoSize();
};

} // end namespace avi
} // end namespace data
} // end namespace monarch
#endif
