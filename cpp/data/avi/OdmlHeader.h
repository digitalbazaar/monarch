/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_avi_OdmlHeader_H
#define monarch_data_avi_OdmlHeader_H

#include "monarch/io/OutputStream.h"
#include "monarch/data/riff/RiffChunkHeader.h"

namespace monarch
{
namespace data
{
namespace avi
{

/**
 * An OpenDML list ('odml').
 *
 * OpenDML format is as follows:
 *
 * OpenDML List ('LIST' size 'odml' data)
 *    OpenDML Header ('dmlh' size data)
 *       total_frames (4 bytes)
 *       padding (244 null bytes)
 *
 * @author Dave Longley
 */
class OdmlHeader
{
public:
   /**
    * Chunk fourcc id "dmlh".
    */
   static const fourcc_t CHUNK_ID = MO_FOURCC_FROM_CHARS('d','m','l','h');

protected:
   /**
    * The ODML header RIFF header.
    */
   monarch::data::riff::RiffChunkHeader mRiffHeader;

   /**
    * The total number of actual ODML video frames.
    */
   uint32_t mTotalFrames;

public:
   /**
    * Constructs a new OdmlHeader.
    */
   OdmlHeader();

   /**
    * Destructs an OdmlHeader.
    */
   virtual ~OdmlHeader();

   /**
    * Writes this OdmlHeader, including the RIFF header, to an OutputStream.
    *
    * @param os the OutputStream to write to.
    *
    * @return true on success, false on IO exception.
    */
   virtual bool writeTo(monarch::io::OutputStream& os);

   /**
    * Converts this OdmlHeader from a byte array.
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
    * Gets the size of this OdmlHeader, excluding its chunk header.
    *
    * @return the size of this OdmlHeader chunk.
    */
   virtual int getChunkSize();

   /**
    * Gets the size of this OdmlHeader including its chunk header.
    *
    * @return the size of this OdmlHeader.
    */
   virtual int getSize();

   /**
    * Gets the total number of frames of ODML video data.
    *
    * @return the total number of frames of ODML video data.
    */
   virtual uint32_t getTotalFrames();
};

} // end namespace avi
} // end namespace data
} // end namespace monarch
#endif
