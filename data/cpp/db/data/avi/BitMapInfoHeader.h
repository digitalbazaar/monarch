/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_avi_BitMapInfoHeader_H
#define db_data_avi_BitMapInfoHeader_H

#include <inttypes.h>

#include "db/io/OutputStream.h"

namespace db
{
namespace data
{
namespace avi
{

/**
 * A BITMAPINFOHEADER structure.
 * 
 * -------------------------------------------------------------------------
 * (a DWORD is 4 bytes, a WORD is 2 bytes, a LONG is 8 bytes)
 * (5 DWORDS = 20 bytes + 2 WORDS (4 bytes) + 4 LONGS (32 bytes) = 56 bytes:
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
 * DWORD number of color indices in  the color table, zero = maximum
 * DWORD number of color indices required for displaying the map, zero = all
 * 
 * @author Dave Longley
 * @author David I. Lehn
 */
class BitMapInfoHeader
{
public:
   /**
    * Size of this header.
    */
   static const int HEADER_SIZE = 56;
   
protected:
   /**
    * The data for structure.
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
    * Constructs a new BitMapInfoHeader.
    */
   BitMapInfoHeader();
   
   /**
    * Destructs a BitMapInfoHeader.
    */
   virtual ~BitMapInfoHeader();
   
   /**
    * Writes this BitMapInfoHeader to an OutputStream.
    * 
    * @param os the OutputStream to write to.
    * 
    * @return true on success, false on an Exception.
    */
   virtual bool writeTo(db::io::OutputStream& os);
   
   /**
    * Converts this BitMapInfoHeader from a byte array.
    * 
    * @param b the byte array to convert from.
    * @param length the number of valid bytes in the buffer.
    * 
    * @return true if successful, false if not.
    */
   virtual bool convertFromBytes(const char* b, int length);
   
   /**
    * Gets the size of this BitMapInfoHeader.
    * 
    * @return the size of this BitMapInfoHeader.
    */
   virtual int getSize();
   
   /**
    * Gets the size of the BitMapInfo.
    * 
    * @return the size of the BitMapInfo.
    */
   virtual int getBitMapInfoSize();
};

} // end namespace avi
} // end namespace data
} // end namespace db
#endif
