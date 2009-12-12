/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/id3v2/TagHeader.h"

#include <string>
#include <cstring>

using namespace db::data::id3v2;

// initialize static variables
const unsigned char TagHeader::sSupportedVersion = 3;
const unsigned char TagHeader::sSupportedRevision = 0;
const int TagHeader::sHeaderSize = 10;
const int TagHeader::sMaxTagSize = 0x10000000; // == 268435456
const unsigned char TagHeader::sUnsynchronizedBit = 0x80;
const unsigned char TagHeader::sExtendedHeaderBit = 0x40;
const unsigned char TagHeader::sExperimentalBit = 0x20;

TagHeader::TagHeader()
{
   mVersion = sSupportedVersion;
   mRevision = sSupportedRevision;
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
      b |= sUnsynchronizedBit;
   }

   if(mExtendedHeaderFlag)
   {
      b |= sExtendedHeaderBit;
   }

   if(mExperimentalFlag)
   {
      b |= sExperimentalBit;
   }

   return b;
}

void TagHeader::convertToBytes(char* b)
{
   unsigned char* ub = (unsigned char*)b;

   memcpy(ub, "ID3", 3);
   ub[3] = mVersion;
   ub[4] = mRevision;
   ub[5] = getFlagByte();

   // get size
   convertIntToSynchsafeBytes(mTagSize, b + 6);
}

bool TagHeader::convertFromBytes(const char* b)
{
   bool rval = false;

   unsigned char* ub = (unsigned char*)b;

   // check for "ID3"
   if(memcmp(ub, "ID3", 3) == 0)
   {
      unsigned char version = sSupportedVersion;
      unsigned char revision = sSupportedRevision;

      // check version and revision
      if(ub[3] <= version && ub[4] <= revision)
      {
         mVersion = version;
         mRevision = revision;
         setFlags(ub[5]);
         mTagSize = convertSynchsafeBytesToInt(b + 6);
         rval = true;
      }
   }

   return rval;
}

inline void TagHeader::setVersion(unsigned char version)
{
   mVersion = version;
}

inline unsigned char TagHeader::getVersion()
{
   return mVersion;
}

inline void TagHeader::setRevision(unsigned char revision)
{
   mRevision = revision;
}

inline unsigned char TagHeader::getRevision()
{
   return mRevision;
}

void TagHeader::setFlags(unsigned char b)
{
   mUnsynchronizedFlag = (b & sUnsynchronizedBit) != 0;
   mExtendedHeaderFlag = (b & sExtendedHeaderBit) != 0;
   mExperimentalFlag = (b & sExperimentalBit) != 0;
}

inline void TagHeader::setUnsynchronizedFlag(bool flag)
{
   mUnsynchronizedFlag = flag;
}

inline bool TagHeader::getUnsynchronizedFlag()
{
   return mUnsynchronizedFlag;
}

inline void TagHeader::setExtendedHeaderFlag(bool flag)
{
   mExtendedHeaderFlag = flag;
}

inline bool TagHeader::getExtendedHeaderFlag()
{
   return mExtendedHeaderFlag;
}

inline void TagHeader::setExperimentalFlag(bool flag)
{
   mExperimentalFlag = flag;
}

inline bool TagHeader::getExperimentalFlag()
{
   return mExperimentalFlag;
}

inline void TagHeader::setTagSize(int tagSize)
{
   mTagSize = tagSize;
}

inline int TagHeader::getTagSize()
{
   return mTagSize;
}

void TagHeader::convertIntToSynchsafeBytes(int integer, char* b)
{
   unsigned char* ub = (unsigned char*)b;

   // we may want to ensure the int is 32-bit
   // only 28 significant bits in the integer
   for(int i = 0; i < 4; i++)
   {
      ub[i] = ((integer >> (28 - ((i + 1) * 7))) & 0x7F);
   }
}

int TagHeader::convertSynchsafeBytesToInt(const char* b)
{
   int rval = 0;

   unsigned char* ub = (unsigned char*)b;

   // we may want to ensure the int is 32-bit
   // most significant byte first
   for(int i = 0; i < 4; i++)
   {
      rval |= ((ub[i] & 0x7F) << ((3 - i) * 7));
   }

   return rval;
}
