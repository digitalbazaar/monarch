/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/IgnoreOutputStream.h"

using namespace db::io;

IgnoreOutputStream::IgnoreOutputStream(OutputStream* os, bool cleanup) :
   FilterOutputStream(os, cleanup)
{
   mIgnoreCount = 0;
}

IgnoreOutputStream::~IgnoreOutputStream()
{
}

bool IgnoreOutputStream::write(const char* b, int length)
{
   bool rval = true;
   
   // determine the length to write out and new ignore count
   int ignored = mIgnoreCount;
   if(mIgnoreCount > length)
   {
      length = 0;
      mIgnoreCount -= length;
   }
   else
   {
      length -= mIgnoreCount;
      mIgnoreCount = 0;
   }
   
   if(length > 0)
   {
      rval = mOutputStream->write(b + ignored, length);
   }
   
   return rval;
}

void IgnoreOutputStream::setIgnoreCount(int count)
{
   mIgnoreCount = count;
}

int IgnoreOutputStream::getIgnoreCount()
{
   return mIgnoreCount;
}
