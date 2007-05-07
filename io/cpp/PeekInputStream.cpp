/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "PeekInputStream.h"
#include "Math.h"

using namespace db::io;
using namespace db::util;

PeekInputStream::PeekInputStream(InputStream* is, bool cleanup) :
   FilterInputStream(is, cleanup)
{
   // no peek buffer yet
   mPeekBuffer = NULL;
   mPeekSize = 0;
   mPeekOffset = 0;
   mPeekLength = 0;
}

PeekInputStream::~PeekInputStream()
{
   // delete the peek buffer
   if(mPeekBuffer != NULL)
   {
      delete [] mPeekBuffer;
   }
}

bool PeekInputStream::read(char& b) throw(IOException)
{
   bool rval = false;
   
   if(read(&b, 0, 1) != -1)
   {
      rval = true;
   }
   
   return rval;
}

int PeekInputStream::read(
   char* b, unsigned int offset, unsigned int length) throw(IOException)
{
   int rval = -1;
   
   if(mPeekLength > 0)
   {
      // read from the peek buffer first
      int count = Math::minimum(mPeekLength, length);
      memcpy(b + offset, mPeekBuffer + mPeekOffset, count);
      
      // update peek buffer
      mPeekOffset += count;
      mPeekLength -= count;
      if(mPeekLength == 0)
      {
         // reset peek offset
         mPeekOffset = 0;
      }
      
      // update bytes read
      rval = count;
   }
   else
   {
      // read from the underlying stream
      rval = mInputStream->read(b, offset, length);
   }
   
   return rval;
}

int PeekInputStream::peek(char* b, unsigned int offset, unsigned int length)
throw(IOException)
{
   int rval = -1;
   
   // see if more data needs to be read
   if(length > mPeekLength)
   {
      // resize the peek buffer size as necessary
      if(length > (mPeekSize - mPeekOffset))
      {
         // fit the peek buffer to the requested length
         char* oldBuffer = mPeekBuffer;
         mPeekBuffer = new char[length];
         memcpy(mPeekBuffer, oldBuffer + mPeekOffset, mPeekLength);
         delete [] oldBuffer;
         mPeekOffset = 0;
         mPeekSize = length;
      }
      
      // read from the underlying stream until the peek buffer gets more data
      size_t offset = mPeekOffset + mPeekLength;
      int count = mInputStream->read(mPeekBuffer, offset, mPeekSize - offset);
      if(count > 0)
      {
         // increase the peek bytes length
         mPeekLength += count;
      }
   }
   
   // read from the peek buffer
   if(mPeekLength > 0)
   {
      int count = Math::minimum(mPeekLength, length);
      memcpy(b + offset, mPeekBuffer + mPeekOffset, count);
      
      // update bytes read
      rval = count;
   }
   
   return rval;
}
