/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/avi/AviHeader.h"

#include "monarch/util/Data.h"

#include <cstring>

using namespace monarch::data;
using namespace monarch::data::avi;
using namespace monarch::data::riff;
using namespace monarch::io;

AviHeader::AviHeader() :
   mRiffHeader(CHUNK_ID)
{
}

AviHeader::~AviHeader()
{
}

uint32_t AviHeader::readDWord(int offset)
{
   uint32_t rval = 0;

   if((offset + 4) <= HEADER_SIZE)
   {
      rval = MO_UINT32_FROM_LE(*((uint32_t*)(mData + offset)));
   }

   return rval;
}

bool AviHeader::writeTo(OutputStream& os)
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

bool AviHeader::convertFromBytes(const char* b, int length)
{
   bool rval = false;

   // convert the RIFF header
   if(mRiffHeader.convertFromBytes(b, length) &&
      mRiffHeader.getIdentifier() == CHUNK_ID)
   {
      // make sure length has enough data for the chunk
      if(length >= getSize())
      {
         memcpy(mData, b + RiffChunkHeader::HEADER_SIZE, HEADER_SIZE);

         // converted successfully
         rval = true;
      }
   }

   return rval;
}

bool AviHeader::isValid()
{
   return mRiffHeader.isValid() && mRiffHeader.getIdentifier() == CHUNK_ID;
}

int AviHeader::getChunkSize()
{
   // AVI header chunk size is 56 bytes
   return (int)mRiffHeader.getChunkSize();
}

int AviHeader::getSize()
{
   return getChunkSize() + RiffChunkHeader::HEADER_SIZE;
}

uint32_t AviHeader::getMicrosecondsPerFrame()
{
   return readDWord(0);
}

uint32_t AviHeader::getMaxBytesPerSecond()
{
   return readDWord(4);
}

uint32_t AviHeader::getPaddingGranularity()
{
   return readDWord(8);
}

bool AviHeader::isAviHasIndex()
{
   return readDWord(12) & AVIF_HASINDEX;
}

bool AviHeader::isAviMustUseIndex()
{
   return readDWord(12) & AVIF_MUSTUSEINDEX;
}

bool AviHeader::isAviIsInterleaved()
{
   return readDWord(12) & AVIF_ISINTERLEAVED;
}

bool AviHeader::isAviWasCaptureFile()
{
   return readDWord(12) & AVIF_WASCAPTUREFILE;
}

bool AviHeader::isAviCopyrighted()
{
   return readDWord(12) & AVIF_COPYRIGHTED;
}

uint32_t AviHeader::getTotalFrames()
{
   return readDWord(16);
}

uint32_t AviHeader::getInitialFrames()
{
   return readDWord(20);
}

uint32_t AviHeader::getStreamCount()
{
   return readDWord(24);
}

uint32_t AviHeader::getSuggestedBufferSize()
{
   return readDWord(28);
}

uint32_t AviHeader::getWidth()
{
   return readDWord(32);
}

uint32_t AviHeader::getHeight()
{
   return readDWord(36);
}

uint32_t AviHeader::getTimeScale()
{
   return readDWord(40);
}

uint32_t AviHeader::getDataRate()
{
   return readDWord(44);
}


double AviHeader::getFrameRate()
{
   double rval = 0.0;

   if(getTimeScale() != 0)
   {
      rval = (double)getDataRate() / getTimeScale();
   }

   return rval;
}

uint32_t AviHeader::getStartingTime()
{
   return readDWord(48);
}

uint32_t AviHeader::getVideoLength()
{
   return readDWord(52);
}

uint32_t AviHeader::getTotalTime()
{
   return getMicrosecondsPerFrame() * getTotalFrames();
}

double AviHeader::getTotalSeconds()
{
   double rval = 0.0;

   if(getTotalFrames() != 0)
   {
      rval = getMicrosecondsPerFrame() / 1000000.0;
      rval *= getTotalFrames();
   }

   return rval;
}
