/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/avi/AviStreamHeaderList.h"

using namespace db::data;
using namespace db::data::avi;
using namespace db::data::riff;
using namespace db::io;

AviStreamHeaderList::AviStreamHeaderList() :
   mRiffHeader(CHUNK_ID)
{
}

AviStreamHeaderList::~AviStreamHeaderList()
{
}

bool AviStreamHeaderList::writeTo(OutputStream& os)
{
   bool rval;

   // write RIFF header
   // write stream header
   // write stream format
   rval =
      mRiffHeader.writeTo(os) &&
      mStreamHeader.writeTo(os) &&
      mStreamFormat.writeTo(os);

   if(rval)
   {
      // write stream data, if present
      if(mStreamData.getChunkSize() > 0)
      {
         rval = mStreamData.writeTo(os);
      }
   }

   return rval;
}

bool AviStreamHeaderList::convertFromBytes(const char* b, int length)
{
   bool rval = false;

   // convert the RIFF header
   if(mRiffHeader.convertFromBytes(b, length) &&
      mRiffHeader.getIdentifier() == CHUNK_ID)
   {
      // make sure there is enough data to convert the header
      if(length >= getSize())
      {
         // step forward past RIFF header
         b += RiffListHeader::HEADER_SIZE;

         // set length to list size
         length = (int)mRiffHeader.getListSize();

         // convert header
         if(mStreamHeader.convertFromBytes(b, length))
         {
            // step forward past header
            b += mStreamHeader.getSize();
            length -= mStreamHeader.getSize();

            // convert format
            if(mStreamFormat.convertFromBytes(b, length))
            {
               // header list converted, 'strd' is not used
               rval = true;

               // step forward past format
               b += mStreamFormat.getSize();
               length -= mStreamFormat.getSize();

               // look for stream data anyway
               if(length > 0)
               {
                  // convert stream data
                  mStreamData.convertFromBytes(b, length);
               }
            }
         }
      }
   }

   return rval;
}

bool AviStreamHeaderList::isValid()
{
   return mRiffHeader.isValid() && mRiffHeader.getIdentifier() == CHUNK_ID;
}

int AviStreamHeaderList::getListSize()
{
   // AVI stream header list is expected to be much smaller than 32-bits
   return (int)mRiffHeader.getListSize();
}

int AviStreamHeaderList::getSize()
{
   return getListSize() + RiffListHeader::HEADER_SIZE;
}
