/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/ByteArrayInputStream.h"

using namespace db::io;

ByteArrayInputStream::ByteArrayInputStream(const char* b, int length)
{
   mBytes = b;
   mLength = length;
   mBuffer = NULL;
}

ByteArrayInputStream::ByteArrayInputStream(ByteBuffer* b)
{
   mBytes = NULL;
   mLength = 0;
   mBuffer = b;
}

ByteArrayInputStream::~ByteArrayInputStream()
{
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
   mBuffer = NULL;
}

void ByteArrayInputStream::setByteBuffer(ByteBuffer* b)
{
   mBytes = NULL;
   mLength = 0;
   mBuffer = b;
}
