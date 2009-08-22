/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media.audio;

import com.db.util.ByteBuffer;

/**
 * An Id3TagFrameDataSink is used to provide a destination to store data for
 * Id3TagFrames.
 * 
 * @author Dave Longley
 */
public interface Id3TagFrameDataSink
{
   /**
    * Writes frame data to this sink for the passed frame header.
    * 
    * @param header the frame header.
    * @param src the ByteBuffer with bytes to read.
    * @param length the number of bytes to read.
    */
   public void writeTagFrameData(
      Id3TagFrameHeader header, ByteBuffer src, int length);
}
