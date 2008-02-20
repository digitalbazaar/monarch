/*
 * Copyright (c) 2005-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/riff/RiffChunkHeader.h"

using namespace db::data;
using namespace db::data::riff;
using namespace db::io;

RiffChunkHeader::RiffChunkHeader(fourcc_t id, int size)
{
   mId = id;
   mChunkSize = size;
   mValid = true;
}

RiffChunkHeader::~RiffChunkHeader()
{
}

bool RiffChunkHeader::writeTo(OutputStream& os)
{
   bool rval;
   
   char buf[HEADER_SIZE];
   convertToBytes(buf);
   rval = os.write(buf, HEADER_SIZE);
   
   return rval;
}

void RiffChunkHeader::convertToBytes(char* b)
{
   DB_FOURCC_TO_STR(mId, b);   
   *(uint32_t*)(b + 4) = DB_UINT32_TO_LE(mChunkSize);
}

bool RiffChunkHeader::convertFromBytes(const char* b, int length)
{
   bool rval = false;
   
   if(b != NULL && length >= HEADER_SIZE)
   {
      mId = DB_FOURCC_FROM_STR(b);
      mChunkSize = DB_UINT32_FROM_LE(*((uint32_t*)(b + 4)));
      rval = true;
   }
   
   setValid(rval);
   
   return rval;
}

void RiffChunkHeader::setIdentifier(fourcc_t id)
{
   mId = id;
}

fourcc_t RiffChunkHeader::getIdentifier()
{
   return mId;
}

void RiffChunkHeader::setChunkSize(uint32_t size)
{
   mChunkSize = size;
}

uint32_t RiffChunkHeader::getChunkSize()
{
   return mChunkSize;
}

uint32_t RiffChunkHeader::getPaddedSize()
{
   uint32_t rval = mChunkSize;
   
   if(rval % 2 == 1)
   {
      rval++;
   }
   
   return rval;
}

uint32_t RiffChunkHeader::getTotalPaddedSize()
{
   return getPaddedSize() + HEADER_SIZE;
}

bool RiffChunkHeader::isValid()
{
   return mValid;
}

void RiffChunkHeader::setValid(bool valid)
{
   mValid = valid;
}
