/*
 * Copyright (c) 2005-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_riff_RiffFormHeader_H
#define monarch_data_riff_RiffFormHeader_H

#include <inttypes.h>

#include "monarch/io/OutputStream.h"
#include "monarch/data/riff/RiffListHeader.h"

namespace monarch
{
namespace data
{
namespace riff
{

/**
 * A RIFF Form header. The file size does *NOT* include the header size. So
 * it should, usually, be the length of the file - 8.
 *
 * The format of the header is such:
 *
 * | bytes 1-4  | bytes 5-8 | bytes 9-12 | bytes 13+ |
 * ---------------------------------------------------
 * |   "RIFF"   | file size |   "XXXX"   | file data |
 *
 * For AVI files:
 *
 * | bytes 1-4  | bytes 5-8 | bytes 9-12 | bytes 13+ |
 * ---------------------------------------------------
 * |   "RIFF"   | file size |   "AVI "   | file data |
 *
 * In hex form:
 *
 * 52 49 46 46 yy yy yy yy xx xx xx xx zz ...
 *
 * AVI hex form:
 *
 * 52 49 46 46 yy yy yy yy 41 56 49 20 zz ...
 *
 * @author Dave Longley
 * @author David I. Lehn
 */
class RiffFormHeader : public RiffListHeader
{
public:
   /**
    * The header size for a form chunk.
    */
   static const int HEADER_SIZE = RiffListHeader::HEADER_SIZE;

   /**
    * The chunk header id for a RIFFs chunk.
    */
   static const fourcc_t CHUNK_ID = MO_FOURCC_FROM_CHARS('R','I','F','F');

   /**
    * Constructs a Riff Form header with the passed parameters.
    *
    * @param form the form of RIFF.
    * @param fileSize the file size of the file the header is for.
    */
   RiffFormHeader(fourcc_t form = 0, uint32_t fileSize = 0);

   /**
    * Desctructs a Riff Form header.
    */
   virtual ~RiffFormHeader();

   /**
    * Converts the header from a byte array with at least 12 bytes.
    *
    * @param b the byte array to convert from.
    * @param length the number of valid bytes in the buffer.
    *
    * @return true if successful, false if not.
    */
   virtual bool convertFromBytes(const char* b, int length);

   /**
    * Sets the file size. This is the size of the entire RIFF formatted file.
    *
    * @param fileSize the size of the entire RIFF formatted file.
    */
   virtual void setFileSize(uint32_t fileSize);

   /**
    * Gets the file size. This is the size of the entire RIFF formatted file.
    *
    * @return the size of the entire RIFF formatted file.
    */
   virtual uint32_t getFileSize();

   /**
    * Gets the header size.
    *
    * @return the header size.
    */
   virtual uint32_t getHeaderSize();
};

} // end namespace riff
} // end namespace data
} // end namespace monarch
#endif
