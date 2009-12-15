/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/compress/gzip/Trailer.h"

#include "monarch/util/Data.h"
#include <zlib.h>

using namespace monarch::compress::gzip;
using namespace monarch::io;

Trailer::Trailer()
{
   mCrc32 = 0;
   mInputSize = 0;
}

Trailer::~Trailer()
{
}

int Trailer::convertFromBytes(char* b, int length)
{
   int rval = 0;

   // make sure there are at least 8 bytes available -- the trailer size
   if(length < 8)
   {
      rval = 8 - length;
   }
   else
   {
      // wrap input in a ByteBuffer
      ByteBuffer bb(b, 0, length, length, false);

      // read crc-32
      bb.get((char*)&mCrc32, 4);
      mCrc32 = MO_UINT32_FROM_LE(mCrc32);

      // read input size
      bb.get((char*)&mInputSize, 4);
      mInputSize = MO_UINT32_FROM_LE(mInputSize);
   }

   return rval;
}

void Trailer::convertToBytes(ByteBuffer* b)
{
   // write crc-32 and input size
   uint32_t crc32 = MO_UINT32_TO_LE(mCrc32);
   uint32_t isize = MO_UINT32_TO_LE(mInputSize);
   b->put((char*)&crc32, 4, true);
   b->put((char*)&isize, 4, true);
}

void Trailer::setCrc32(unsigned int crc)
{
   mCrc32 = crc;
}

unsigned int Trailer::getCrc32()
{
   return mCrc32;
}

void Trailer::setInputSize(unsigned int iSize)
{
   mInputSize = iSize;
}

unsigned int Trailer::getInputSize()
{
   return mInputSize;
}
