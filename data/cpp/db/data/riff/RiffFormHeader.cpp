/*
 * Copyright (c) 2005-2008 Digital Bazaar, Inc.  All rights reserved.
 */
 
#include "db/data/riff/RiffFormHeader.h"

using namespace db::data;
using namespace db::data::riff;

RiffFormHeader::RiffFormHeader(fourcc_t form, uint32_t fileSize) :
   RiffListHeader(form, fileSize)
{
   mChunkHeader.setIdentifier(CHUNK_ID);
}

RiffFormHeader::~RiffFormHeader()
{
}

bool RiffFormHeader::convertFromBytes(const char* b, int offset, int length)
{
   bool rval = false;
   
   if(b != NULL && length >= HEADER_SIZE)
   {
      if(mChunkHeader.convertFromBytes(b, offset, length))
      {
         // make sure chunk identifier is LIST
         if(mChunkHeader.getIdentifier() == CHUNK_ID)
         {
            mId = DB_FOURCC_FROM_STR(b + offset + RiffChunkHeader::HEADER_SIZE);
            rval = true;
         }
      }
   }
   
   setValid(rval);
   
   return rval;      
}

void RiffFormHeader::setFileSize(uint32_t fileSize)
{
   setChunkSize(fileSize);
}

uint32_t RiffFormHeader::getFileSize()
{
   return getChunkSize();
}

uint32_t RiffFormHeader::getHeaderSize()
{
   return RiffListHeader::HEADER_SIZE;
}
