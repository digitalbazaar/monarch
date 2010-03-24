/*
 * Copyright (c) 2009-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/io/InputStream.h"

#include "monarch/rt/Exception.h"

using namespace monarch::io;
using namespace monarch::rt;

#define INPUT_BUFFER_SIZE   2048

InputStream::InputStream()
{
}

InputStream::~InputStream()
{
}

int InputStream::peek(char* b, int length, bool block)
{
   // extending classes must implement this method if they want support
   ExceptionRef e = new Exception(
      "InputStream::peek() is not implemented.",
      "monarch.io.NotImplemented.");
   Exception::set(e);
   return -1;
}

int64_t InputStream::skip(int64_t count)
{
   int64_t skipped = 0;

   // read and discard bytes
   char b[INPUT_BUFFER_SIZE];
   int numBytes = 0;
   int length = (count < INPUT_BUFFER_SIZE) ? count : INPUT_BUFFER_SIZE;
   while(count > 0 && (numBytes = read(b, length)) > 0)
   {
      skipped += numBytes;
      count -= numBytes;
      length = (count < INPUT_BUFFER_SIZE) ? count : INPUT_BUFFER_SIZE;
   }

   if(skipped == 0 && numBytes == -1)
   {
      skipped = -1;
   }

   return skipped;
}
