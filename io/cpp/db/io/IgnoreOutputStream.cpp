/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/io/IgnoreOutputStream.h"

#include "db/io/IOException.h"

using namespace db::io;
using namespace db::rt;

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
      if(!rval)
      {
         ExceptionRef ref = Exception::get();
         IOException* e = dynamic_cast<IOException*>(&(*ref));
         if(e != NULL)
         {
            // update used bytes to include ignored bytes
            e->setUsedBytes(ignored + e->getUsedBytes());
         }
      }
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
