/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_data_avi_BitMapInfo_H
#define db_data_avi_BitMapInfo_H

#include "db/data/avi/BitMapInfoHeader.h"
#include "db/io/OutputStream.h"

namespace db
{
namespace data
{
namespace avi
{

/**
 * A BITMAPINFO structure.
 *
 * The structure contains:
 * -----------------------
 * BITMAPINFOHEADER - header for this BITMAPINFO
 *
 * RGBQUAD[] - an array of RGBQUAD elements
 * OR
 * An array of 16-bit unsigned integers that specify indices into a palette
 *
 * @author Dave Longley
 * @author David I. Lehn
 */
class BitMapInfo
{
protected:
   /**
    * The header for the structure.
    */
   BitMapInfoHeader mHeader;

   /**
    * The data for structure.
    */
   char* mData;

public:
   /**
    * Constructs a new BitMapInfo.
    */
   BitMapInfo();

   /**
    * Destructs a BitMapInfo.
    */
   virtual ~BitMapInfo();

   /**
    * Writes this BitMapInfo to an OutputStream.
    *
    * @param os the OutputStream to write to.
    *
    * @return true on success, false on an Exception.
    */
   virtual bool writeTo(db::io::OutputStream& os);

   /**
    * Converts this BitMapInfo from a byte array.
    *
    * @param b the byte array to convert from.
    * @param length the number of valid bytes in the buffer.
    *
    * @return true if successful, false if not.
    */
   virtual bool convertFromBytes(const char* b, int length);

   /**
    * Gets the size of this BitMapInfo.
    *
    * @return the size of this BitMapInfo.
    */
   virtual int getSize();
};

} // end namespace avi
} // end namespace data
} // end namespace db
#endif
