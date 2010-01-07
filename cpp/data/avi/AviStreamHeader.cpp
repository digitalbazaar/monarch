/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/avi/AviStreamHeader.h"

#include "monarch/util/Data.h"

#include <cstring>

using namespace monarch::data;
using namespace monarch::data::avi;
using namespace monarch::data::riff;
using namespace monarch::io;

AviStreamHeader::AviStreamHeader() :
   mRiffHeader(CHUNK_ID)
{
}

AviStreamHeader::~AviStreamHeader()
{
}

static uint32_t _readDWord(char* data, int offset)
{
   uint32_t rval = 0;

   if((offset + 4) <= AviStreamHeader::HEADER_SIZE)
   {
      rval = MO_UINT32_FROM_LE(*((uint32_t*)(data + offset)));
   }

   return rval;
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

fourcc_t AviStreamHeader::getType()
{
   return MO_FOURCC_FROM_STR(mData);
}

fourcc_t AviStreamHeader::getHandler()
{
   return MO_FOURCC_FROM_STR(mData + 4);
}

uint32_t AviStreamHeader::getInitialFrames()
{
   // skip DWORD flags and DWORD reserved
   return _readDWord(mData, 16);
}

uint32_t AviStreamHeader::getTimeScale()
{
   return _readDWord(mData, 20);
}

uint32_t AviStreamHeader::getRate()
{
   return _readDWord(mData, 24);
}

uint32_t AviStreamHeader::getStartTime()
{
   return _readDWord(mData, 28);
}

uint32_t AviStreamHeader::getLength()
{
   return _readDWord(mData, 32);
}

uint32_t AviStreamHeader::getSuggestedBufferSize()
{
   return _readDWord(mData, 36);
}

uint32_t AviStreamHeader::getQuality()
{
   return _readDWord(mData, 40);
}

uint32_t AviStreamHeader::getSampleSize()
{
   return _readDWord(mData, 44);
}
