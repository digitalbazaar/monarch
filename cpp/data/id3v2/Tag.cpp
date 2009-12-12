/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/id3v2/Tag.h"

using namespace std;
using namespace monarch::data::id3v2;

Tag::Tag()
{
}

Tag::~Tag()
{
   // delete all frame headers
   for(FrameHeaderList::iterator i = mFrameHeaders.begin();
       i != mFrameHeaders.end(); i++)
   {
      delete *i;
   }
}

void Tag::addFrameHeader(FrameHeader* header, bool updateSize)
{
   // add map entry as appropriate
   FrameHeaderMap::iterator i = mFrameHeaderMap.find(header->getId());
   if(i == mFrameHeaderMap.end())
   {
      mFrameHeaderMap[header->getId()] = header;
   }

   // add frame header
   mFrameHeaders.push_back(header);

   if(updateSize)
   {
      // update tag size
      mHeader.setTagSize(
         mHeader.getTagSize() + header->getFrameSize() + 10);
   }
}

FrameHeader* Tag::getFrameHeader(const char* id)
{
   FrameHeader* rval = NULL;

   FrameHeaderMap::iterator i = mFrameHeaderMap.find(id);
   if(i != mFrameHeaderMap.end())
   {
      rval = i->second;
   }

   return rval;
}

void Tag::getFrameHeaders(const char* id, FrameHeaderList& l)
{
   for(FrameHeaderList::iterator i = mFrameHeaders.begin();
       i != mFrameHeaders.end(); i++)
   {
      if(strcmp((*i)->getId(), id) == 0)
      {
         l.push_back(*i);
      }
   }
}

Tag::FrameHeaderList& Tag::getFrameHeaders()
{
   return mFrameHeaders;
}

TagHeader* Tag::getHeader()
{
   return &mHeader;
}
