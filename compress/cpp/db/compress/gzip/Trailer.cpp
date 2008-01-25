/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/compress/gzip/Trailer.h"

#include <zlib.h>

using namespace db::compress::gzip;
using namespace db::io;

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
      ByteBuffer bb(b, 0, length, false);
      
      // read crc-32
      bb.get((char*)&mCrc32, 4);
      
      // read input size
      bb.get((char*)&mInputSize, 4);
   }
   
   return rval;
}

void Trailer::convertToBytes(ByteBuffer* b)
{
   // write crc-32 and input size
   b->put((char*)&mCrc32, 4, true);
   b->put((char*)&mInputSize, 4, true);
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
