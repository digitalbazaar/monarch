/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/data/avi/AviStreamHeader.h"

#include <cstring>

using namespace db::data;
using namespace db::data::avi;
using namespace db::data::riff;
using namespace db::io;

AviStreamHeader::AviStreamHeader() :
   mRiffHeader(CHUNK_ID)
{
}

AviStreamHeader::~AviStreamHeader()
{
}

bool AviStreamHeader::writeTo(OutputStream& os)
{
   bool rval;
   
   // write RIFF header
   rval = mRiffHeader.writeTo(os);
   
   if(rval)
   {
      // write data
      rval = os.write(mData, HEADER_SIZE);
   }
   
   return rval;
}

bool AviStreamHeader::convertFromBytes(const char* b, int length)
{
   bool rval = false;
   
   // convert the RIFF header
   if(mRiffHeader.convertFromBytes(b, length) && isValid())
   {
      // make sure length has enough data for the chunk
      if(length >= getSize())
      {
         memcpy(mData, b + RiffChunkHeader::HEADER_SIZE, getChunkSize());
         
         // converted successfully
         rval = true;
      }
   }
   
   return rval;
}

bool AviStreamHeader::isValid()
{
   return mRiffHeader.isValid() && mRiffHeader.getIdentifier() == CHUNK_ID;
}

int AviStreamHeader::getChunkSize()
{
   // AVI stream header size is 56 bytes
   return (int)mRiffHeader.getChunkSize();
}

int AviStreamHeader::getSize()
{
   return getChunkSize() + RiffChunkHeader::HEADER_SIZE;
}
