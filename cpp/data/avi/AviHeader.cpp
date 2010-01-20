/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

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

static uint32_t _readDWord(char* data, int offset)
{
   uint32_t rval = 0;

   if((offset + 4) <= AviHeader::HEADER_SIZE)
   {
      rval = MO_UINT32_FROM_LE(*((uint32_t*)(data + offset)));
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
   return _readDWord(mData, 0);
}

uint32_t AviHeader::getMaxBytesPerSecond()
{
   return _readDWord(mData, 4);
}

uint32_t AviHeader::getPaddingGranularity()
{
   return _readDWord(mData, 8);
}

bool AviHeader::isAviHasIndex()
{
   return _readDWord(mData, 12) & AVIF_HASINDEX;
}

bool AviHeader::isAviMustUseIndex()
{
   return _readDWord(mData, 12) & AVIF_MUSTUSEINDEX;
}

bool AviHeader::isAviIsInterleaved()
{
   return _readDWord(mData, 12) & AVIF_ISINTERLEAVED;
}

bool AviHeader::isAviWasCaptureFile()
{
   return _readDWord(mData, 12) & AVIF_WASCAPTUREFILE;
}

bool AviHeader::isAviCopyrighted()
{
   return _readDWord(mData, 12) & AVIF_COPYRIGHTED;
}

uint32_t AviHeader::getTotalFrames()
{
   return _readDWord(mData, 16);
}

uint32_t AviHeader::getInitialFrames()
{
   return _readDWord(mData, 20);
}

uint32_t AviHeader::getStreamCount()
{
   return _readDWord(mData, 24);
}

uint32_t AviHeader::getSuggestedBufferSize()
{
   return _readDWord(mData, 28);
}

uint32_t AviHeader::getWidth()
{
   return _readDWord(mData, 32);
}

uint32_t AviHeader::getHeight()
{
   return _readDWord(mData, 36);
}

uint32_t AviHeader::getTimeScale()
{
   return _readDWord(mData, 40);
}

uint32_t AviHeader::getDataRate()
{
   return _readDWord(mData, 44);
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
   return _readDWord(mData, 48);
}

uint32_t AviHeader::getVideoLength()
{
   return _readDWord(mData, 52);
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
