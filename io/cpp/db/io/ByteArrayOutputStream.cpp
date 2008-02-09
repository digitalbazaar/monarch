/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
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
   int written = mBuffer->put(b, length, getResize());
   if(written != length)
   {
      // FIXME: probably want to add something to IO exception for
      // storing bytes that couldn't be read/written, etc
      rval = false;
      IOException* e = new IOException(
         "Could not write all data, ByteBuffer is full!");
      e->setUsedBytes(written);
      e->setUnusedBytes(length - written);
      ExceptionRef ref = e;
      Exception::setLast(ref, false);
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
