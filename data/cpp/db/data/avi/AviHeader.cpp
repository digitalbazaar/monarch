/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */

#include "db/data/avi/AviHeader.h"

using namespace db::data;
using namespace db::data::avi;
using namespace db::data::riff;
using namespace db::io;

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
      rval = DB_UINT32_FROM_LE(*((uint32_t*)(mData + offset)));
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

bool AviHeader::convertFromBytes(const char* b, int offset, int length)
{
   bool rval = false;
   
   // convert the RIFF header
   if(mRiffHeader.convertFromBytes(b, offset, length) &&
      mRiffHeader.getIdentifier() == CHUNK_ID)
   {
      // make sure length has enough data for the chunk
      if(length >= getSize())
      {
         memcpy(mData, b + offset + RiffChunkHeader::HEADER_SIZE, HEADER_SIZE);
         
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

long AviHeader::getMicrosecondsPerFrame()
{
   return readDWord(0);
}

long AviHeader::getMaxBytesPerSecond()
{
   return readDWord(4);
}

long AviHeader::getPaddingGranularity()
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

long AviHeader::getTotalFrames()
{
   return readDWord(16);
}

long AviHeader::getInitialFrames()
{
   return readDWord(20);
}

long AviHeader::getStreamCount()
{
   return readDWord(24);
}

long AviHeader::getSuggestedBufferSize()
{
   return readDWord(28);
}

long AviHeader::getWidth()
{
   return readDWord(32);
}

long AviHeader::getHeight()
{
   return readDWord(36);
}

long AviHeader::getTimeScale()
{
   return readDWord(40);
}

long AviHeader::getDataRate()
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

long AviHeader::getStartingTime()
{
   return readDWord(48);
}

long AviHeader::getVideoLength()
{
   return readDWord(52);
}

long AviHeader::getTotalTime()
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
