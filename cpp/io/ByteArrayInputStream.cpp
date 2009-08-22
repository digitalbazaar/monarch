/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/io/ByteArrayInputStream.h"

#include <cstring>

using namespace db::io;

ByteArrayInputStream::ByteArrayInputStream(const char* b, int length)
{
   mBytes = b;
   mLength = length;
   mBuffer = NULL;
   mCleanupBuffer = false;
}

ByteArrayInputStream::ByteArrayInputStream(ByteBuffer* b, bool cleanup)
{
   mBytes = NULL;
   mLength = 0;
   mBuffer = b;
   mCleanupBuffer = cleanup;
}

ByteArrayInputStream::~ByteArrayInputStream()
{
   if(mCleanupBuffer && mBuffer != NULL)
   {
      delete mBuffer;
   }
}

int ByteArrayInputStream::read(char* b, int length)
{
   int rval = 0;

   if(mBuffer == NULL)
   {
      // try to read from byte array
      if(mLength > 0)
      {
         // copy bytes into passed buffer
         rval = (length > mLength) ? mLength : length;
         memcpy(b, mBytes, rval);

         // increment bytes pointer, decrement length
         mBytes += rval;
         mLength -= rval;
      }
   }
   else
   {
      // try to read from byte buffer
      if(!mBuffer->isEmpty())
      {
         // get bytes from buffer
         rval = mBuffer->get(b, length);
      }
   }

   return rval;
}

void ByteArrayInputStream::setByteArray(const char* b, int length)
{
   mBytes = b;
   mLength = length;

   if(mCleanupBuffer && mBuffer != NULL)
   {
      delete mBuffer;
      mBuffer = NULL;
      mCleanupBuffer = false;
   }
}

void ByteArrayInputStream::setByteBuffer(ByteBuffer* b, bool cleanup)
{
   mBytes = NULL;
   mLength = 0;

   if(mCleanupBuffer && mBuffer != NULL)
   {
      delete mBuffer;
      mCleanupBuffer = false;
   }

   mBuffer = b;
   mCleanupBuffer = cleanup;
}
