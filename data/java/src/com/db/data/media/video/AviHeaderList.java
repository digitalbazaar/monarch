/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media.video;

import java.io.IOException;
import java.io.OutputStream;
import java.util.Vector;

import com.db.data.media.RiffListHeader;
import com.db.logging.Logger;
import com.db.logging.LoggerManager;

/**
 * An AVI Header List. This list contains an AVI main header and stream
 * header lists.
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
 * @author Dave Longley
 */
public class AviHeaderList
{
   /**
    * The Header List RIFF header.
    */
   protected RiffListHeader mRiffHeader;
   
   /**
    * The main AviHeader for this header list.
    */
   protected AviHeader mMainHeader;
   
   /**
    * The AviStreamHeaderLists in this header list.
    */
   protected Vector<AviStreamHeaderList> mStreamHeaderLists;
   
   /**
    * Constructs a new AviHeaderList.
    */
   public AviHeaderList()
   {
      // create the RIFF header
      mRiffHeader = new RiffListHeader("hdrl");
      
      // create the main avi header
      mMainHeader = new AviHeader();
      
      // create the container for the stream header lists
      mStreamHeaderLists = new Vector<AviStreamHeaderList>();
   }
   
   /**
    * Writes this AviHeaderList, including the RIFF header, to an OutputStream.
    * 
    * @param os the OutputStream to write to.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   public void writeTo(OutputStream os) throws IOException
   {
      // write RIFF header
      mRiffHeader.writeTo(os);
      
      // write the main AVI header
      mMainHeader.writeTo(os);
      
      // write each stream header list
      for(AviStreamHeaderList list: mStreamHeaderLists)
      {
         list.writeTo(os);
      }
   }
   
   /**
    * Converts the chunk header from a byte array with at least 8 bytes.
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
      
      // convert the header
      if(mRiffHeader.convertFromBytes(b, offset, length) &&
         mRiffHeader.getIdentifier().equals("hdrl"))
      {
         // step forward past RIFF header
         offset += RiffListHeader.LIST_HEADER_SIZE;
         length -= RiffListHeader.LIST_HEADER_SIZE;
         
         // ensure there is enough data remaining to convert the header list
         if(length >= mRiffHeader.getListSize())
         {
            // set length to size of list
            length = (int)mRiffHeader.getListSize();
            
            // convert main header
            if(mMainHeader.convertFromBytes(b, offset, length))
            {
               // main header converted
               rval = true;
               
               // move past header
               offset += mMainHeader.getSize();
               length -= mMainHeader.getSize();
               
               // convert all stream header lists
               while(length > 0)
               {
                  AviStreamHeaderList list = new AviStreamHeaderList();
                  if(list.convertFromBytes(b, offset, length))
                  {
                     mStreamHeaderLists.add(list);
                  }
                  else
                  {
                     // invalid stream header list
                     getLogger().error(getClass(),
                        "Invalid stream header list 'strl' detected at " +
                        "offset " + offset);
                     break;
                  }
                  
                  // move to next stream header list
                  offset += list.getSize();
                  length -= list.getSize();
               }
            }
         }
      }
      
      return rval;
   }
   
   /**
    * Returns whether or not this AviHeaderList is valid.
    * 
    * @return true if valid, false if not.
    */
   public boolean isValid()
   {
      return mRiffHeader.isValid() &&
         mRiffHeader.getIdentifier().equals("hdrl") && mMainHeader.isValid();
   }
   
   /**
    * Gets the size of this AviHeaderList, including its RIFF header.
    * 
    * @return the size of this AviHeaderList.
    */
   public long getSize()
   {
      return mRiffHeader.getListSize() + RiffListHeader.LIST_HEADER_SIZE;
   }
   
   /**
    * Gets the logger.
    * 
    * @return the logger.
    */
   public Logger getLogger()
   {
      return LoggerManager.getLogger("dbdata");
   }
}
