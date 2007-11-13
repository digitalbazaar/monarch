/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/json/JsonInputStream.h"
#include "db/io/ByteArrayOutputStream.h"

using namespace std;
using namespace db::data;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

JsonInputStream::JsonInputStream(DynamicObject dyno) :
   mBufferInputStream(NULL, 0)
{
   mHasJSON = false;
   mDyno = dyno;
}

JsonInputStream::~JsonInputStream()
{
}

int JsonInputStream::read(char* b, int length)
{
   int rval = 0;
   
   if(!mHasJSON)
   {
      // Do full JSON conversion
      ByteArrayOutputStream baos(&mBuffer, true);
      if(mJsonWriter.write(mDyno, &baos))
      {
         mBufferInputStream.setByteArray(mBuffer.data(), mBuffer.length());
         mHasJSON = true;
      }
      else
      {
         rval = -1;
      }
   }

   if(rval == 0)
   {
      // Conversion done, just feed more data out
      rval = mBufferInputStream.read(b, length);
   }
   
   return rval;
}

void JsonInputStream::setCompact(bool compact)
{
   mJsonWriter.setCompact(compact);
}

void JsonInputStream::setIndentation(int level, int spaces)
{
   mJsonWriter.setIndentation(level, spaces);
}
