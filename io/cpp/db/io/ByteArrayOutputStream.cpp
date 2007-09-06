/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/ByteArrayOutputStream.h"

using namespace db::io;
using namespace db::rt;

ByteArrayOutputStream::ByteArrayOutputStream(ByteBuffer* b, bool resize)
{
   mBuffer = b;
   mResize = resize;
}

ByteArrayOutputStream::~ByteArrayOutputStream()
{
}

bool ByteArrayOutputStream::write(const char* b, int length)
{
   bool rval = true;
   
   // put bytes in byte buffer
   if(mBuffer->put(b, length, getResize()) != length)
   {
      // FIXME: probably want to add something to IO exception for
      // storing bytes that couldn't be read/written, etc
      rval = false;
      Exception::setLast(new IOException(
         "Could not write all data, ByteBuffer is full!"));
   }
   
   return rval;
}

ByteBuffer* ByteArrayOutputStream::getByteArray()
{
   return mBuffer;
}

void ByteArrayOutputStream::setResize(bool resize)
{
   mResize = resize;
}

bool ByteArrayOutputStream::getResize()
{
   return mResize;
}
