/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include "monarch/data/avi/OdmlHeader.h"

#include "monarch/util/Data.h"

#include <cstring>

using namespace monarch::data;
using namespace monarch::data::avi;
using namespace monarch::data::riff;
using namespace monarch::io;

OdmlHeader::OdmlHeader() :
   mRiffHeader(CHUNK_ID),
   mTotalFrames(0)
{
}

OdmlHeader::~OdmlHeader()
{
}

bool OdmlHeader::writeTo(OutputStream& os)
{
   bool rval;

   // get total frames in little endian
   uint32_t tf = MO_UINT32_TO_LE(mTotalFrames);

   // create padding
   int count = getChunkSize() - 4;
   char* padding = (char*)malloc(count);
   memset(padding, 0, count);

   // write RIFF header
   // write total frames
   // write null padding
   rval =
      mRiffHeader.writeTo(os) &&
      os.write((char*)&tf, 4) &&
      os.write(padding, count);

   // free padding
   free(padding);

   return rval;
}

bool OdmlHeader::convertFromBytes(const char* b, int length)
{
   bool rval = false;

   // convert the RIFF header
   if(mRiffHeader.convertFromBytes(b, length) &&
      mRiffHeader.getIdentifier() == CHUNK_ID)
   {
      // make sure length has enough data for the chunk
      if(length >= getSize())
      {
         // skip chunk header, get total frames
         b += RiffChunkHeader::HEADER_SIZE;
         mTotalFrames = MO_UINT32_FROM_LE(*((uint32_t*)(b)));

         // converted successfully
         rval = true;
      }
   }

   return rval;
}

bool OdmlHeader::isValid()
{
   return mRiffHeader.isValid() && mRiffHeader.getIdentifier() == CHUNK_ID;
}

int OdmlHeader::getChunkSize()
{
   return (int)mRiffHeader.getChunkSize();
}

int OdmlHeader::getSize()
{
   return getChunkSize() + RiffChunkHeader::HEADER_SIZE;
}

uint32_t OdmlHeader::getTotalFrames()
{
   return mTotalFrames;
}
