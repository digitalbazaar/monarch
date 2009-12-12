/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_data_riff_RiffListHeader_H
#define db_data_riff_RiffListHeader_H

#include <inttypes.h>

#include "monarch/io/OutputStream.h"
#include "monarch/data/riff/RiffChunkHeader.h"

namespace db
{
namespace data
{
namespace riff
{

/**
 * A RIFF List Header.
 *
 * The format of a RIFF List is such:
 *
 * | bytes 1-4  | bytes 5-8 | bytes 9-12 | bytes 13+ |
 * ---------------------------------------------------
 * |   "LIST"   | list size |   "XXXX"   | list data |
 *
 * @author Dave Longley
 * @author David I. Lehn
 */
class RiffListHeader
{
public:
   /**
    * The header size for a list chunk.
    */
   static const int HEADER_SIZE = 12;

   /**
    * The chunk header id for a list chunk.
    */
   static const fourcc_t CHUNK_ID = DB_FOURCC_FROM_CHARS('L','I','S','T');

protected:
   /**
    * A Riff Chunk Header.
    */
   RiffChunkHeader mChunkHeader;

   /**
    * The 4 byte list identifier.
    */
   fourcc_t mId;

public:
   /**
    * Constructs a Riff List Header with the passed parameters.
    *
    * @param id the identifier of the list.
    * @param size the size of the list.
    */
   RiffListHeader(fourcc_t id = CHUNK_ID, uint32_t size = 0);

   /**
    * Destructs a Riff List Header.
    */
   virtual ~RiffListHeader();

   /**
    * Writes this RiffListHeader to an OutputStream.
    *
    * @param os the OutputStream to write to.
    *
    * @return true on success, false if an IO error occurs.
    */
   virtual bool writeTo(db::io::OutputStream& os);

   /**
    * Converts the header into a 12 byte array.
    *
    * @param bytes destination of the header as a 12 byte array.
    */
   virtual void convertToBytes(char* b);

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
    * Sets the 4 byte identifier for this list if the passed string is
    * 4 characters long.
    *
    * @param id the identifier to set.
    */
   virtual void setIdentifier(fourcc_t id);

   /**
    * Gets the 4 byte identifier for this list.
    *
    * @return the list identifier.
    */
   virtual fourcc_t getIdentifier();

   /**
    * Sets the chunk size. The chunk size includes the FOURCC for the list.
    *
    * @param size the chunk size of the Riff list.
    */
   virtual void setChunkSize(uint32_t size);

   /**
    * Gets the chunk size. The chunk size includes the FOURCC for the list.
    *
    * @return the chunk size of the Riff list.
    */
   virtual uint32_t getChunkSize();

   /**
    * Sets the list size. The list size excludes the FOURCC for the list.
    *
    * @param size the size of the Riff list.
    */
   virtual void setListSize(uint32_t size);

   /**
    * Gets the list size. The list size excludes the FOURCC for the list.
    *
    * @return the size of the Riff list.
    */
   virtual uint32_t getListSize();

   /**
    * Returns whether or not this list is valid.
    *
    * @return true if valid, false if not.
    */
   virtual bool isValid();

   /**
    * Sets whether or not this list is valid.
    *
    * @param valid true to set to valid, false to set to invalid.
    */
   virtual void setValid(bool valid);
};

} // end namespace riff
} // end namespace data
} // end namespace db
#endif
