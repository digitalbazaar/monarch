/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/avi/BitMapInfo.h"

using namespace db::data;
using namespace db::data::avi;
using namespace db::io;

BitMapInfo::BitMapInfo()
{
   // create empty data
   mData = NULL;
}

BitMapInfo::~BitMapInfo()
{
}

bool BitMapInfo::writeTo(OutputStream& os)
{
   bool rval;

   // write data
   rval = os.write(mData, getSize());

   return rval;
}

bool BitMapInfo::convertFromBytes(const char* b, int length)
{
   bool rval = false;

   // convert header
   if(mHeader.convertFromBytes(b, length))
   {
      // make sure length has enough data
      if(length >= mHeader.getBitMapInfoSize())
      {
         // FIXME: do not copy data for now
//            mData = new byte[mHeader.getBitMapInfoSize()];
//            System.arraycopy(b, offset, mData, 0, getSize());

         // converted successfully
         rval = true;
      }
   }

   return rval;
}

/**
 * Gets the size of this BitMapInfo.
 *
 * @return the size of this BitMapInfo.
 */
int BitMapInfo::getSize()
{
   return 0;
}
