/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/avi/AviStreamFormat.h"

#include "monarch/util/Data.h"

#include <cstring>

using namespace monarch::data;
using namespace monarch::data::avi;
using namespace monarch::data::riff;
using namespace monarch::io;

AviStreamFormat::AviStreamFormat() :
   mRiffHeader(CHUNK_ID),
   mData(NULL)
{
}

AviStreamFormat::~AviStreamFormat()
{
   if(mData != NULL)
   {
      free(mData);
   }
}

static uint16_t _readWord(char* data, int offset, int max)
{
   uint16_t rval = 0;

   if((offset + 2) <= max)
   {
      rval = MO_UINT16_FROM_LE(*((uint16_t*)(data + offset)));
   }

   return rval;
}

static uint32_t _readDWord(char* data, int offset, int max)
{
   uint32_t rval = 0;

   if((offset + 4) <= max)
   {
      rval = MO_UINT32_FROM_LE(*((uint32_t*)(data + offset)));
   }

   return rval;
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
         mData = (char*)malloc(getChunkSize());
         memcpy(mData, b + RiffChunkHeader::HEADER_SIZE, getChunkSize());

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

uint32_t AviStreamFormat::getBitmapInfoSize()
{
   return _readDWord(mData, 0, getChunkSize());
}

uint32_t AviStreamFormat::getBitmapWidth()
{
   return _readDWord(mData, 4, getChunkSize());
}

uint32_t AviStreamFormat::getBitmapHeight()
{
   return _readDWord(mData, 8, getChunkSize());
}

uint16_t AviStreamFormat::getBitCount()
{
   // skip WORD planes @ 12
   return _readWord(mData, 14, getChunkSize());
}

uint32_t AviStreamFormat::getCompression()
{
   return _readDWord(mData, 16, getChunkSize());
}

uint32_t AviStreamFormat::getImageSize()
{
   return _readDWord(mData, 20, getChunkSize());
}

uint32_t AviStreamFormat::getHorizontalResolution()
{
   return _readDWord(mData, 24, getChunkSize());
}

uint32_t AviStreamFormat::getVerticalResolution()
{
   return _readDWord(mData, 28, getChunkSize());
}

uint32_t AviStreamFormat::getColorInidices()
{
   return _readDWord(mData, 32, getChunkSize());
}

uint32_t AviStreamFormat::getColorInidicesRequired()
{
   return _readDWord(mData, 36, getChunkSize());
}

uint16_t AviStreamFormat::getAudioFormatTag()
{
   return _readWord(mData, 0, getChunkSize());
}

uint16_t AviStreamFormat::getAudioChannels()
{
   return _readWord(mData, 2, getChunkSize());
}

uint32_t AviStreamFormat::getAudioSamplesPerSecond()
{
   return _readWord(mData, 4, getChunkSize());
}

uint32_t AviStreamFormat::getAudioAvgBytesPerSecond()
{
   return _readDWord(mData, 8, getChunkSize());
}

uint16_t AviStreamFormat::getBlockAligmentUnit()
{
   return _readWord(mData, 12, getChunkSize());
}

uint16_t AviStreamFormat::getAudioBitsPerSample()
{
   return _readWord(mData, 14, getChunkSize());
}

uint16_t AviStreamFormat::getExtraAudioInfoSize()
{
   return _readWord(mData, 16, getChunkSize());
}
