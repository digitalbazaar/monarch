/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/io/BufferedOutputStream.h"

using namespace monarch::io;

BufferedOutputStream::BufferedOutputStream(
   ByteBuffer* b, OutputStream* os, bool cleanup) :
   FilterOutputStream(os, cleanup),
   mBuffer(b)
{
}

BufferedOutputStream::~BufferedOutputStream()
{
}

bool BufferedOutputStream::write(const char* b, int length)
{
   bool rval = true;

   int written = 0;
   while(rval && written < length)
   {
      // put bytes into buffer
      written += mBuffer->put(b + written, length - written, false);

      // flush buffer if full
      if(mBuffer->isFull())
      {
         rval = flush();
      }
   }

   return rval;
}

bool BufferedOutputStream::flush()
{
   bool rval = mOutputStream->write(mBuffer->data(), mBuffer->length());
   mBuffer->clear();
   return rval;
}

void BufferedOutputStream::close()
{
   // make sure to flush ;)
   flush();
   mOutputStream->close();
}

void BufferedOutputStream::setBuffer(ByteBuffer* b)
{
   mBuffer = b;
}
