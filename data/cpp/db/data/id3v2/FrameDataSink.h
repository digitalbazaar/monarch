/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_id3v2_FrameDataSink_H
#define db_data_id3v2_FrameDataSink_H

#include "db/data/id3v2/FrameHeader.h"
#include "db/io/ByteBuffer.h"

namespace db
{
namespace data
{
namespace id3v2
{

/**
 * A FrameDataSink is used to provide a destination to store data for
 * id3 tag Frames.
 * 
 * @author Dave Longley
 */
class FrameDataSink
{
public:
   /**
    * Creates a new FrameDataSink.
    */
   FrameDataSink() {};
   
   /**
    * Destructs this FrameDataSink.
    */
   virtual ~FrameDataSink() {};
   
   /**
    * Writes frame data to this sink for the passed frame header.
    * 
    * @param header the frame header.
    * @param src the ByteBuffer with bytes to read.
    * @param length the number of bytes to read.
    * 
    * @return true if the write was successful, false if an IO exception
    *         occurred.
    */
   virtual bool writeFrameData(
      FrameHeader* header, db::io::ByteBuffer* src, int length) = 0;
};

} // end namespace id3v2
} // end namespace data
} // end namespace db
#endif
