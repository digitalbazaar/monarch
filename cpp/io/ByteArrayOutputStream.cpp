/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/io/ByteArrayOutputStream.h"

#include "monarch/io/IOException.h"

using namespace std;
using namespace monarch::io;
using namespace monarch::rt;

ByteArrayOutputStream::ByteArrayOutputStream() :
   mBuffer(new ByteBuffer()),
   mResize(true),
   mCleanup(true)
{
}

ByteArrayOutputStream::ByteArrayOutputStream(ByteBuffer* b, bool resize) :
   mBuffer(b),
   mResize(resize),
   mCleanup(false)
{
}

ByteArrayOutputStream::~ByteArrayOutputStream()
{
   if(mCleanup)
   {
      delete mBuffer;
   }
}

bool ByteArrayOutputStream::write(const char* b, int length)
{
   bool rval = true;

   // put bytes in byte buffer
   int written = mBuffer->put(b, length, getResize());
   if(written != length)
   {
      IOException* e = new IOException(
         "Could not write all data, ByteBuffer is full.");
      e->setUsedBytes(written);
      e->setUnusedBytes(length - written);
      ExceptionRef ref = e;
      Exception::set(ref);
      rval = false;
   }

   return rval;
}

ByteBuffer* ByteArrayOutputStream::getByteArray()
{
   return mBuffer;
}

string ByteArrayOutputStream::str()
{
   return string(mBuffer->data(), mBuffer->length());
}

void ByteArrayOutputStream::setResize(bool resize)
{
   mResize = resize;
}

bool ByteArrayOutputStream::getResize()
{
   return mResize;
}
