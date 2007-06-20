/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.data.media.audio;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

/**
 * The Id3Tag class is used to parse, display/edit, and write ID3 version
 * ID3v2.3.0 compliant tags.
 * 
 * An Id3Tag contains an ID3v2 header and Id3TagFrames. The maximum tag
 * size is 256 megabytes and the maximum tag frame size is 16 megabytes.
 * 
 * This tag can be parsed from an array of bytes, but only those frames
 * that are added to this tag before the conversion will have their data
 * stored in memory. All frame headers will be stored.
 * 
 * @author Dave Longley
 */
public class Id3Tag
{
   /**
    * The ID3v2 header.
    */
   protected Id3v2Header mHeader;
   
   /**
    * Stores tag frame headers.
    */
   protected LinkedList<Id3TagFrameHeader> mFrameHeaders;
   
   /**
    * Stores a reference to the first tag frame header with a given ID.
    */
   protected HashMap<String, Id3TagFrameHeader> mFrameHeaderMap;
   
   /**
    * Creates an ID3 tag.
    */
   public Id3Tag()
   {
      mHeader = new Id3v2Header();
      mFrameHeaders = new LinkedList<Id3TagFrameHeader>();
      mFrameHeaderMap = new HashMap<String, Id3TagFrameHeader>();
   }
   
   /**
    * Adds a tag frame header to this ID3 tag. If the passed frame header is
    * the first with its given ID, it will be added to a map for later quick
    * lookups.
    * 
    * The total tag size for this tag will be updated based on the frame size
    * in the given header.
    * 
    * @param header the frame header to add.
    * @param updateSize true to update the tag size, false not to.
    */
   public void addFrameHeader(Id3TagFrameHeader header, boolean updateSize)
   {
      // add map entry as appropriate
      if(mFrameHeaderMap.get(header.getId()) == null)
      {
         mFrameHeaderMap.put(header.getId(), header);
      }
      
      // add frame header
      mFrameHeaders.add(header);
      
      if(updateSize)
      {
         // update tag size
         getHeader().setTagSize(
            getHeader().getTagSize() + header.getFrameSize() + 10);
      }
   }
   
   /**
    * Gets the first tag frame header with the given ID from this ID3 tag.
    * 
    * @param id the ID of the tag frame header to get.
    * 
    * @return the first tag frame header with the given ID or null if none
    *         exists.
    */
   public Id3TagFrameHeader getFrameHeader(String id)
   {
      return mFrameHeaderMap.get(id);
   }
   
   /**
    * Gets all the tag frame headers with the given IDs.
    * 
    * @param id the ID of the frame headers to retrieve.
    * 
    * @return a list with all of the tag frame headers with the given ID.
    */
   public List<Id3TagFrameHeader> getFrameHeaders(String id)
   {
      LinkedList<Id3TagFrameHeader> list = new LinkedList<Id3TagFrameHeader>();
      for(Id3TagFrameHeader header: mFrameHeaders)
      {
         if(header.getId().equals(id))
         {
            list.add(header);
         }
      }
      
      return list;
   }
   
   /**
    * Gets all the tag frame headers.
    * 
    * @return a list with all of the tag frame headers.
    */
   public List<Id3TagFrameHeader> getFrameHeaders()
   {
      return mFrameHeaders;
   }
   
   /**
    * Gets the ID3v2 header for this tag.
    * 
    * @return the ID3v2 header for this tag.
    */
   public Id3v2Header getHeader()
   {
      return mHeader;
   }
}
