/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/data/avi/BitMapInfoHeader.h"
#include "db/data/Data.h"
#include "db/util/Data.h"

#include <cstring>

using namespace db::data;
using namespace db::data::avi;
using namespace db::io;

BitMapInfoHeader::BitMapInfoHeader()
{
}

BitMapInfoHeader::~BitMapInfoHeader()
{
}
uint32_t BitMapInfoHeader::readDWord(int offset)
{
   uint32_t rval = 0;

   if((offset + 4) <= HEADER_SIZE)
   {
      rval = DB_UINT32_FROM_LE(*((uint32_t*)(mData + offset)));
   }

   return rval;
}

bool BitMapInfoHeader::writeTo(OutputStream& os)
{
   bool rval;

   // write data
   rval = os.write(mData, HEADER_SIZE);

   return rval;
}

bool BitMapInfoHeader::convertFromBytes(const char* b, int length)
{
   bool rval = false;

   // make sure length has enough data
   if(length >= HEADER_SIZE)
   {
      memcpy(mData, b, getSize());

      // converted successfully
      rval = true;
   }

   return rval;
}

/**
 * Gets the size of this BitMapInfoHeader.
 *
 * @return the size of this BitMapInfoHeader.
 */
int BitMapInfoHeader::getSize()
{
   return HEADER_SIZE;
}

/**
 * Gets the size of the BitMapInfo.
 *
 * @return the size of the BitMapInfo.
 */
int BitMapInfoHeader::getBitMapInfoSize()
{
   return (int)readDWord(0);
}
