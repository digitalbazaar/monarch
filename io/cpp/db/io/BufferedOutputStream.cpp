/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/BufferedOutputStream.h"

using namespace db::io;

BufferedOutputStream::BufferedOutputStream(
   ByteBuffer* b, OutputStream* os, bool cleanup) :
   FilterOutputStream(os, cleanup)
{
   mBuffer = b;
}

BufferedOutputStream::~BufferedOutputStream()
{
}

bool BufferedOutputStream::write(const char* b, int length)
{
   bool rval = true;
   
   while(rval && length > 0)
   {
      // put bytes into buffer
      length -= mBuffer->put(b, length, false);
      
      // flush buffer if full
      if(mBuffer->isFull())
      {
         rval = mOutputStream->write(mBuffer->data(), mBuffer->length());
         mBuffer->clear();
      }
   }
   
   return rval;
}
