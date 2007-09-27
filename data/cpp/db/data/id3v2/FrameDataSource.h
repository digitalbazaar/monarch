/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_id3v2_FrameDataSource_H
#define db_data_id3v2_FrameDataSource_H

#include "db/data/id3v2/FrameHeader.h"
#include "db/io/ByteBuffer.h"

namespace db
{
namespace data
{
namespace id3v2
{

/**
 * A FrameDataSource is used to provide source data for id3 tag Frames.
 * 
 * @author Dave Longley
 */
class FrameDataSource
{
public:
   /**
    * Creates a new FrameDataSource.
    */
   FrameDataSource() {};
   
   /**
    * Destructs this FrameDataSource.
    */
   virtual ~FrameDataSource() {};
   
   /**
    * Reads frame data for the passed frame header.
    * 
    * @param header the frame header.
    * @param dest the ByteBuffer to populate with data.
    * @param resize true to permit resizing the ByteBuffer, false not to.
    * 
    * @return the number of bytes read, 0 if the end of the data has been
    *         reached, or -1 if an IO exception occurred.
    */
   virtual int readFrameData(
      FrameHeader* header, db::io::ByteBuffer* dest, bool resize) = 0;
};

} // end namespace id3v2
} // end namespace data
} // end namespace db
#endif
