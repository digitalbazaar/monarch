/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_id3v2_FrameSource_H
#define monarch_data_id3v2_FrameSource_H

#include "monarch/data/id3v2/FrameHeader.h"
#include "monarch/io/ByteBuffer.h"

namespace monarch
{
namespace data
{
namespace id3v2
{

/**
 * A FrameSource is used to provide binary data for id3v2 tag frames.
 *
 * @author Dave Longley
 */
class FrameSource
{
public:
   /**
    * Creates a new FrameSource.
    */
   FrameSource() {};

   /**
    * Destructs this FrameSource.
    */
   virtual ~FrameSource() {};

   /**
    * Updates the passed frame header's set frame size according to the
    * data this source can provide for the frame.
    *
    * @param header the id3v2 tag frame header to update.
    */
   virtual void updateFrameSize(FrameHeader* header) = 0;

   /**
    * Prepares this frame source for retrieving the frame data for the
    * given header.
    *
    * @param header the id3v2 tag frame header to start retrieving data for.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool startFrame(FrameHeader* header) = 0;

   /**
    * Gets the frame bytes for the current frame header, including the frame
    * header in binary format. This method can be called repeated to retrieve
    * the entire frame.
    *
    * @param dst the ByteBuffer to populate with data.
    * @param resize true to permit resizing the ByteBuffer, false not to.
    *
    * @return the number of bytes read, 0 if the end of the data has been
    *         reached, or -1 if an exception occurred.
    */
   virtual int getFrame(monarch::io::ByteBuffer* dst, bool resize) = 0;
};

} // end namespace id3v2
} // end namespace data
} // end namespace monarch
#endif
