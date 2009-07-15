/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/io/TruncateInputStream.h"

using namespace db::io;

TruncateInputStream::TruncateInputStream(
   uint64_t max, InputStream* is, bool cleanup) :
FilterInputStream(is, cleanup)
{
   mMax = max;
   mTotal = 0;
}

TruncateInputStream::~TruncateInputStream()
{
}

int TruncateInputStream::read(char* b, int length)
{
   int rval = 0;
   
   // only read up to max amount, truncate rest
   if(mTotal < mMax)
   {
      // allow maximum amount of data to be read
      int remaining = (int)(mMax - mTotal);
      length = (length > remaining ? remaining : length);
      rval = FilterInputStream::read(b, length);
      if(rval > 0)
      {
         mTotal += rval;
      }
   }
   
   return rval;
}
