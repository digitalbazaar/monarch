/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/PeekInputStream.h"

using namespace db::io;

PeekInputStream::PeekInputStream(InputStream* is, bool cleanup) :
   FilterInputStream(is, cleanup),
   mPeekBuffer(0)
{
}

PeekInputStream::~PeekInputStream()
{
}

int PeekInputStream::read(char* b, int length)
{
   int rval = 0;
   
   if(!mPeekBuffer.isEmpty())
   {
      // read from the peek buffer first
      rval = mPeekBuffer.get(b, length);
   }
   else
   {
      // read from the underlying stream
      rval = mInputStream->read(b, length);
   }
   
   return rval;
}

int PeekInputStream::peek(char* b, int length, bool block)
{
   int rval = -1;
   
   // see if more data needs to be read
   if(block && length > mPeekBuffer.length())
   {
      // allocate enough space in the peek buffer
      mPeekBuffer.allocateSpace(length, true);
      
      // read into the peek buffer from the underlying stream
      mPeekBuffer.put(mInputStream);
   }
   
   // check for peeked bytes
   if(!mPeekBuffer.isEmpty())
   {
      // read from the peek buffer
      rval = mPeekBuffer.get(b, length);
      
      // reset peek buffer
      mPeekBuffer.reset(rval);
   }
   else if(!block)
   {
      // not-blocking, so return 0
      rval = 0;
   }
   
   return rval;
}
