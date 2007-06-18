/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media.audio;

import com.db.util.ByteBuffer;

/**
 * An Id3TagFrameDataSource is used to provide source data for Id3TagFrames.
 * 
 * @author Dave Longley
 */
public interface Id3TagFrameDataSource
{
   /**
    * Reads frame data for the passed frame header.
    * 
    * @param header the frame header.
    * @param dest the ByteBuffer to populate with data.
    * @param resize true to permit resizing the ByteBuffer, false not to.
    * 
    * @return the number of bytes read or -1 if the end of the data has been
    *         reached. 
    */
   public int readTagFrameData(
      Id3TagFrameHeader header, ByteBuffer dest, boolean resize);
}
