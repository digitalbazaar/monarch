/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.format;

import java.io.IOException;
import java.io.OutputStream;

/**
 * An AVI Stream Header List ('strl').
 * 
 * AVI Format is as follows:
 * 
 * AVI Form Header ('RIFF' size 'AVI ' data)
 *    Header List ('LIST' size 'hdrl' data)
 *       AVI Header ('avih' size data)
 *       Video Stream Header List ('LIST' size 'strl' data)
 *          Video Stream Header ('strh' size data)
 *          Video Stream Format ('strf' size data)
 *          Video Stream Data ('strd' size data) - IGNORED, for DRIVERS
 *       Audio Stream Header List ('LIST' size 'strl' data)
 *          Audio Stream Header ('strh' size data)
 *          Audio Stream Format ('strf' size data)
 *          Audio Stream Data ('strd' size data) - IGNORED, for DRIVERS
 *    Info List ('LIST' size 'INFO' data)
 *       Index Entry ({'ISBJ','IART','ICMT',...} size data)
 *    Movie List ('LIST' size 'movi' data)
 *       Movie Entry ({'00db','00dc','01wb'} size data)
 *    Index Chunk ('idx1' size data)
 *       Index Entry ({'00db','00dc','01wb',...})
 * 
 * The Stream Header List 'strl' applies to the first stream in the 'movi'
 * LIST, the second applies to the second stream, etc.
 * 
 * @author Dave Longley
 */
public class AviStreamHeaderList
{
   /**
    * The AVI stream header RIFF header.
    */
   protected RiffListHeader mRiffHeader;
   
   /**
    * The Stream Header.
    */
   protected AviStreamHeader mStreamHeader;
   
   /**
    * The Stream Format.
    */
   protected AviStreamFormat mStreamFormat;
   
   /**
    * The Stream Data.
    */
   protected AviStreamData mStreamData;
   
   /**
    * Constructs a new AviStreamHeaderList.
    */
   public AviStreamHeaderList()
   {
      // create RIFF header
      mRiffHeader = new RiffListHeader("strl");
      
      // create stream header and format
      mStreamHeader = new AviStreamHeader();
      mStreamFormat = new AviStreamFormat();
      
      // no stream data by default
      mStreamData = null;
   }
   
   /**
    * Writes this AviStreamHeaderList, including the RIFF header, to an
    * OutputStream.
    * 
    * @param os the OutputStream to write to.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public void writeTo(OutputStream os) throws IOException
   {
      // write RIFF header
      mRiffHeader.writeTo(os);
      
      // write stream header
      mStreamHeader.writeTo(os);
      
      // write stream format
      mStreamFormat.writeTo(os);
      
      // write stream data, if present
      if(mStreamData != null)
      {
         mStreamData.writeTo(os);
      }
   }
   
   /**
    * Converts this AviStreamHeaderList from a byte array.
    * 
    * @param b the byte array to convert from.
    * @param offset the offset to start converting from.
    * @param length the number of valid bytes in the buffer following the
    *               offset.
    * 
    * @return true if successful, false if not.
    */
   public boolean convertFromBytes(byte[] b, int offset, int length)
   {
      boolean rval = false;
      
      // convert the RIFF header
      if(mRiffHeader.convertFromBytes(b, offset, length) &&
         mRiffHeader.getIdentifier().equals("strl"))
      {
         // step forward past RIFF header
         offset += RiffListHeader.LIST_HEADER_SIZE;
         length -= RiffListHeader.LIST_HEADER_SIZE;
         
         // make sure there is enough data to convert the header
         if(length >= mRiffHeader.getListSize())
         {
            // convert AviStreamHeaders:
            
            // FIXME:
            
            
            // converted successfully
            rval = true;
         }
      }
      
      return rval;
   }
   
   /**
    * Returns whether or not this AviStreamHeaderList is valid.
    * 
    * @return true if valid, false if not.
    */
   public boolean isValid()
   {
      return mRiffHeader.isValid();
   }
   
   /**
    * Gets the size of this AviStreamHeaderList, not including its chunk header.
    * 
    * @return the size of this AviStreamHeaderList.
    */
   public long getSize()
   {
      return mRiffHeader.getListSize();
   }
}
