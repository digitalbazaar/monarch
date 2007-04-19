/*
 * Copyright (c) 2006-2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media.audio;

import com.db.util.Crc16;

/**
 * This class is used to *correctly* calculate the CRC-16 for MPEG Audio.
 * 
 * The polynomial key used is 0x8005. The starting CRC value is 0xffff.
 * 
 * @author Dave Longley
 */
public class MpegAudioCrc16 extends Crc16
{
   /**
    * Creates a new MpegAudioCrc16.
    */
   public MpegAudioCrc16()
   {
      super(0x8005);
      
      // reset CRC value to 0xffff
      resetValueToMax();
   }
   
   /**
    * Resets the CRC value to 0xffff.
    */
   @Override
   public void reset()
   {
      super.resetValueToMax();
   }
}
