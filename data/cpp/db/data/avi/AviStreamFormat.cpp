/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/data/avi/AviStreamFormat.h"

using namespace db::data;
using namespace db::data::avi;
using namespace db::data::riff;
using namespace db::io;

AviStreamFormat::AviStreamFormat() :
   mRiffHeader(CHUNK_ID)
{
   // create empty data
   mData = NULL;
}

AviStreamFormat::~AviStreamFormat()
{
}

bool AviStreamFormat::writeTo(OutputStream& os)
{
   bool rval;
   
   // write RIFF header
   rval = mRiffHeader.writeTo(os);
   
   if(rval)
   {
      // write data
      rval = os.write(mData, 0);
   }
   
   return rval;
}

bool AviStreamFormat::convertFromBytes(const char* b, int length)
{
   bool rval = false;
   
   // convert the RIFF header
   if(mRiffHeader.convertFromBytes(b, length) && isValid())
   {
      // make sure length has enough data for the chunk
      if(length >= getSize())
      {
         // FIXME: do not copy data for now
//            mData = new byte[getChunkSize()];
//            System.arraycopy(b, offset + RiffChunkHeader.CHUNK_HEADER_SIZE,
//               mData, 0, getChunkSize());
         
         // converted successfully
         rval = true;
      }
   }
   
   return rval;
}

bool AviStreamFormat::isValid()
{
   return mRiffHeader.isValid() && mRiffHeader.getIdentifier() == CHUNK_ID;
}

int AviStreamFormat::getChunkSize()
{
   // AVI stream formats are smaller than 32-bits
   return (int)mRiffHeader.getChunkSize();
}

int AviStreamFormat::getSize()
{
   return getChunkSize() + RiffChunkHeader::HEADER_SIZE;
}

const char* AviStreamFormat::getData()
{
   return mData;
}
