/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/id3v2/TagHeader.h"

#include <string>

using namespace db::data::id3v2;

// initialize static variables
const unsigned char TagHeader::SUPPORTED_VERSION = 3;
const unsigned char TagHeader::SUPPORTED_REVISION = 0;
const int TagHeader::HEADER_SIZE = 10;
const int TagHeader::MAX_TAG_SIZE = 268435456;
const unsigned char TagHeader::UNSYNCHRONIZED_BIT = 0x80;
const unsigned char TagHeader::EXTENDEDHEADER_BIT = 0x40;
const unsigned char TagHeader::EXPERIMENTAL_BIT = 0x20;

TagHeader::TagHeader()
{
   mVersion = SUPPORTED_VERSION;
   mRevision = SUPPORTED_REVISION;
   mUnsynchronizedFlag = false;
   mExtendedHeaderFlag = false;
   mExperimentalFlag = false;
   mTagSize = 0;
}

TagHeader::~TagHeader()
{
}

unsigned char TagHeader::getFlagByte()
{
   unsigned char b = 0x00;
   
   if(mUnsynchronizedFlag)
   {
      b |= UNSYNCHRONIZED_BIT;
   }
   
   if(mExtendedHeaderFlag)
   {
      b |= EXTENDEDHEADER_BIT;
   }
   
   if(mExperimentalFlag)
   {
      b |= EXPERIMENTAL_BIT;
   }
   
   return b;
}

void TagHeader::convertToBytes(char* b)
{
   memcpy(b, "ID3", 3);
   b[3] = mVersion;
   b[4] = mRevision;
   b[5] = getFlagByte();
   
   // get size
   convertIntToSynchsafeBytes(mTagSize, b + 6);
}

bool TagHeader::convertFromBytes(const char* b)
{
   bool rval = false;
   
   // check for "ID3"
   if(memcmp(b, "ID3", 3) == 0)
   {
      unsigned char version = SUPPORTED_VERSION;
      unsigned char revision = SUPPORTED_REVISION;
      
      // check version and revision
      if(b[3] <= version && b[4] <= revision)
      {
         mVersion = version;
         mRevision = revision;
         setFlags(b[5]);
         mTagSize = convertSynchsafeBytesToInt(b + 6);
         rval = true;
      }
   }
   
   return rval;
}

void TagHeader::setVersion(unsigned char version)
{
   mVersion = version;
}

unsigned char TagHeader::getVersion()
{
   return mVersion;
}

void TagHeader::setRevision(unsigned char revision)
{
   mRevision = revision;
}

unsigned char TagHeader::getRevision()
{
   return mRevision;
}

void TagHeader::setFlags(unsigned char b)
{
   mUnsynchronizedFlag = (b & UNSYNCHRONIZED_BIT) != 0;
   mExtendedHeaderFlag = (b & EXTENDEDHEADER_BIT) != 0;
   mExperimentalFlag = (b & EXPERIMENTAL_BIT) != 0;
}

void TagHeader::setUnsynchronizedFlag(bool flag)
{
   mUnsynchronizedFlag = flag;
}

bool TagHeader::getUnsynchronizedFlag()
{
   return mUnsynchronizedFlag;
}

void TagHeader::setExtendedHeaderFlag(bool flag)
{
   mExtendedHeaderFlag = flag;
}

bool TagHeader::getExtendedHeaderFlag()
{
   return mExtendedHeaderFlag;
}

void TagHeader::setExperimentalFlag(bool flag)
{
   mExperimentalFlag = flag;
}

bool TagHeader::getExperimentalFlag()
{
   return mExperimentalFlag;
}

void TagHeader::setTagSize(int tagSize)
{
   mTagSize = tagSize;
}

int TagHeader::getTagSize()
{
   return mTagSize;
}

void TagHeader::convertIntToSynchsafeBytes(int integer, char* b)
{
   // we may want to ensure the int is 32-bit
   // only 28 significant bits in the integer
   for(int i = 0; i < 4; i++)
   {
      b[i] = ((integer >> (28 - ((i + 1) * 7))) & 0x7F);
   }
}

int TagHeader::convertSynchsafeBytesToInt(const char* b)
{
   int rval = 0;
   
   // we may want to ensure the int is 32-bit
   // most significant byte first
   for(int i = 0; i < 4; i++)
   {
      rval |= ((((unsigned char)b[i]) & 0x7F) << ((3 - i) * 7));
   }
   
   return rval;
}
