/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/avi/AviHeaderList.h"

using namespace std;
using namespace db::data;
using namespace db::data::avi;
using namespace db::data::riff;
using namespace db::io;

AviHeaderList::AviHeaderList() :
   mRiffHeader(CHUNK_ID)
{
}

AviHeaderList::~AviHeaderList()
{
}

bool AviHeaderList::writeTo(OutputStream& os)
{
   bool rval;

   // write RIFF header
   rval = mRiffHeader.writeTo(os);
   
   if(rval)
   {
      // write the main AVI header
      rval = mMainHeader.writeTo(os);
   }
   
   // write each stream header list
   for(list<AviStreamHeaderList>::iterator i = mStreamHeaderLists.begin();
      i != mStreamHeaderLists.end() && rval;
      i++)
   {
      rval = (*i).writeTo(os);
   }
   
   return rval;
}

bool AviHeaderList::convertFromBytes(const char* b, int length)
{
   bool rval = false;
   
   // convert the header
   int offset = 0;
   if(mRiffHeader.convertFromBytes(b + offset, length) &&
      mRiffHeader.getIdentifier() == CHUNK_ID)
   {
      // step forward past RIFF header
      offset += RiffListHeader::HEADER_SIZE;
      length -= RiffListHeader::HEADER_SIZE;
      
      // convert main header
      if(mMainHeader.convertFromBytes(b + offset, length))
      {
         // main header converted
         rval = true;
         
         // ensure there is enough data remaining to convert the header list
         if(length >= (int)mRiffHeader.getListSize())
         {
            // set length to size of list
            length = (int)mRiffHeader.getListSize();
            
            // move past header
            offset += mMainHeader.getSize();
            length -= mMainHeader.getSize();
            
            // convert all stream header lists
            while(length > 0)
            {
               AviStreamHeaderList list;
               if(list.convertFromBytes(b + offset, length))
               {
                  mStreamHeaderLists.push_back(list);
                  
                  // move to next stream header list
                  offset += list.getSize();
                  length -= list.getSize();
               }
               else
               {
                  // invalid stream header list
                  DB_CAT_ERROR(DB_DATA_CAT, "AviHeaderList: "
                     "Invalid stream header list detected at offset %d",
                     offset);
                  break;
               }
            }
         }
      }
   }
   
   return rval;
}

bool AviHeaderList::isValid()
{
   return mRiffHeader.isValid() &&
      mRiffHeader.getIdentifier() == CHUNK_ID && mMainHeader.isValid();
}

int AviHeaderList::getSize()
{
   // AVI header list is expected to be under 32-bits
   return (int)mRiffHeader.getListSize() + RiffListHeader::HEADER_SIZE;
}

AviHeader& AviHeaderList::getMainHeader()
{
   return mMainHeader;
}
