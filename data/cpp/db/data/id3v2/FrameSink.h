/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_id3v2_FrameSink_H
#define db_data_id3v2_FrameSink_H

#include "db/data/id3v2/FrameHeader.h"
#include "db/io/ByteBuffer.h"

namespace db
{
namespace data
{
namespace id3v2
{

/**
 * A FrameSink is a destination to write id3v2 tag frame data to.
 * 
 * @author Dave Longley
 */
class FrameSink
{
public:
   /**
    * Creates a new FrameSink.
    */
   FrameSink() {};
   
   /**
    * Destructs this FrameSink.
    */
   virtual ~FrameSink() {};
   
   /**
    * Prepares this frame sink to receive frame data for the given header.
    * 
    * @param header the id3v2 tag frame header to receive data for.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool startFrame(db::data::id3v2::FrameHeader* header) = 0;
   
   /**
    * Writes frame data to this sink for the current frame header. The source
    * should begin at the start of the frame data, after the frame header.
    * 
    * @param src the ByteBuffer with frame data bytes to read.
    * 
    * @return true if successful, false if an exception occurred.
    */
   virtual bool putFrameData(db::io::ByteBuffer* src) = 0;
};

} // end namespace id3v2
} // end namespace data
} // end namespace db
#endif
