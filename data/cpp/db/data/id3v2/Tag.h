/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_data_id3v2_Tag_H
#define db_data_id3v2_Tag_H

#include "db/data/id3v2/TagHeader.h"
#include "db/data/id3v2/FrameHeader.h"
#include "db/util/StringTools.h"

#include <list>
#include <map>
#include <cstring>

namespace db
{
namespace data
{
namespace id3v2
{

/**
 * The Tag class is used to parse, display/edit, and write ID3 version
 * ID3v2.3.0 compliant tags.
 * 
 * A Tag contains an ID3v2 header and TagFrames. The maximum tag
 * size is 256 megabytes and the maximum tag frame size is 16 megabytes.
 * 
 * This tag can be parsed from an array of bytes, but only those frames
 * that are added to this tag before the conversion will have their data
 * stored in memory. All frame headers will be stored.
 * 
 * @author Dave Longley
 */
class Tag
{
protected:
   /**
    * The header for this Tag.
    */
   TagHeader mHeader;
   
   /**
    * Stores tag frame headers.
    */
   typedef std::list<FrameHeader*> FrameHeaderList;
   FrameHeaderList mFrameHeaders;
   
   /**
    * Stores a reference to the first frame header with a given ID.
    * Uses StringComparator to compare IDs for FrameHeaders.
    */
   typedef std::map<const char*, FrameHeader*, db::util::StringComparator>
      FrameHeaderMap;
   FrameHeaderMap mFrameHeaderMap;
   
public:
   /**
    * Creates a new Tag.
    */
   Tag();
   
   /**
    * Destructs this Tag.
    */
   virtual ~Tag();
   
   /**
    * Adds a heap-allocated frame header to this tag. If the passed frame
    * header is the first with its given ID, it will be added to a map for
    * later quick lookups.
    * 
    * The total tag size for this tag will be updated based on the frame size
    * in the given header.
    * 
    * The FrameHeader must be heap-allocated as it will be cleaned up by
    * this Tag when it is destructed.
    * 
    * @param header the frame header to add.
    * @param updateSize true to update the tag size, false not to.
    */
   virtual void addFrameHeader(FrameHeader* header, bool updateSize);
   
   /**
    * Gets the first frame header with the given ID from this tag.
    * 
    * @param id the ID of the frame header to get.
    * 
    * @return the first frame header with the given ID or null if none exists.
    */
   virtual FrameHeader* getFrameHeader(const char* id);
   
   /**
    * Gets all the frame headers with the given IDs.
    * 
    * @param id the ID of the frame headers to retrieve.
    * @param l the list to populate with all of the frame headers
    *          with the given ID.
    */
   virtual void getFrameHeaders(const char* id, FrameHeaderList& l);
   
   /**
    * Gets all the frame headers.
    * 
    * @return the list with all of the frame headers.
    */
   virtual FrameHeaderList& getFrameHeaders();
   
   /**
    * Gets the header for this tag.
    * 
    * @return the header for this tag.
    */
   virtual TagHeader* getHeader();
};

} // end namespace id3v2
} // end namespace data
} // end namespace db
#endif
