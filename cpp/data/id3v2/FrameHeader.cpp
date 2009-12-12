/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/id3v2/FrameHeader.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>

using namespace std;
using namespace monarch::data::id3v2;

// initialize static variables
const int FrameHeader::HEADER_SIZE = 10;
const unsigned char FrameHeader::TAG_ALTERED_DISCARD_FRAME_BIT = 0x80;
const unsigned char FrameHeader::FILE_ALTERED_DISCARD_FRAME_BIT = 0x40;
const unsigned char FrameHeader::READ_ONLY_BIT = 0x20;
const unsigned char FrameHeader::COMPRESSION_BIT = 0x80;
const unsigned char FrameHeader::ENCRYPTION_BIT = 0x40;
const unsigned char FrameHeader::GROUPING_BIT = 0x20;

FrameHeader::FrameHeader(const char* id)
{
   mId = (char*)malloc(5);
   strncpy(mId, id, 4);
   mId[4] = 0;

   mDescription = strdup("");

   mFrameSize = 0;
   mTagAlteredDiscardFrame = false;
   mFileAlteredDiscardFrame = false;
   mReadOnly = false;
   mCompressed = false;
   mEncrypted = false;
   mGrouped = false;
}

FrameHeader::~FrameHeader()
{
   free(mId);
   free(mDescription);
}

void FrameHeader::setFlags1(unsigned char b)
{
   mTagAlteredDiscardFrame = (b & TAG_ALTERED_DISCARD_FRAME_BIT) != 0;
   mFileAlteredDiscardFrame = (b & FILE_ALTERED_DISCARD_FRAME_BIT) != 0;
   mReadOnly = (b & READ_ONLY_BIT) != 0;
}

unsigned char FrameHeader::getFlagByte1()
{
   unsigned char b = 0x00;

   if(mTagAlteredDiscardFrame)
   {
      b |= TAG_ALTERED_DISCARD_FRAME_BIT;
   }

   if(mFileAlteredDiscardFrame)
   {
      b |= FILE_ALTERED_DISCARD_FRAME_BIT;
   }

   if(mReadOnly)
   {
      b |= READ_ONLY_BIT;
   }

   return b;
}

void FrameHeader::setFlags2(unsigned char b)
{
   mCompressed = (b & COMPRESSION_BIT) != 0;
   mEncrypted = (b & ENCRYPTION_BIT) != 0;
   mGrouped = (b & GROUPING_BIT) != 0;
}

unsigned char FrameHeader::getFlagByte2()
{
   unsigned char b = 0x00;

   if(mCompressed)
   {
      b |= COMPRESSION_BIT;
   }

   if(mEncrypted)
   {
      b |= ENCRYPTION_BIT;
   }

   if(mGrouped)
   {
      b |= GROUPING_BIT;
   }

   return b;
}

void FrameHeader::convertFromBytes(const char* b, int length)
{
   // convert ID
   char id[5];
   strncpy(id, b, 4);
   id[4] = 0;
   setId(id);

   // convert frame size
   setFrameSize(convertBytesToInt(b + 4));

   // convert flags
   unsigned char* ub = (unsigned char*)b;
   setFlags1(ub[8]);
   setFlags2(ub[9]);
}

void FrameHeader::convertToBytes(char* b)
{
   // copy ID
   memcpy(b, getId(), 4);

   // set size
   convertIntToBytes(getFrameSize(), b + 4);

   // set flags
   unsigned char* ub = (unsigned char*)b;
   ub[8] = getFlagByte1();
   ub[9] = getFlagByte2();
}

inline void FrameHeader::setId(const char* id)
{
   strncpy(mId, id, 4);
}

inline const char* FrameHeader::getId()
{
   return mId;
}

inline void FrameHeader::setDescription(const char* description)
{
   free(mDescription);
   mDescription = strdup(description);

   // FIXME: uncomment once char encoding is implemented
   // Utf8Codec::decode("ISO-8859-1", description);
}

inline const char* FrameHeader::getDescription()
{
   return mDescription;
}

inline void FrameHeader::setFrameSize(int size)
{
   mFrameSize = size;
}

inline int FrameHeader::getFrameSize()
{
   return mFrameSize;
}

inline void FrameHeader::setTagAlteredDiscardFrame(bool discard)
{
   mTagAlteredDiscardFrame = discard;
}

inline bool FrameHeader::getTagAlteredDiscardFrame()
{
   return mTagAlteredDiscardFrame;
}

inline void FrameHeader::setFileAlteredDiscardFrame(bool discard)
{
   mFileAlteredDiscardFrame = discard;
}

inline bool FrameHeader::getFileAlteredDiscardFrame()
{
   return mFileAlteredDiscardFrame;
}

inline void FrameHeader::setReadOnly(bool readOnly)
{
   mReadOnly = readOnly;
}

inline bool FrameHeader::isReadOnly()
{
   return mReadOnly;
}

inline void FrameHeader::setCompressed(bool compressed)
{
   mCompressed = compressed;
}

inline bool FrameHeader::isCompressed()
{
   return mCompressed;
}

inline void FrameHeader::setEncrypted(bool encrypted)
{
   mEncrypted = encrypted;
}

inline bool FrameHeader::isEncrypted()
{
   return mEncrypted;
}

inline void FrameHeader::setGrouped(bool grouped)
{
   mGrouped = grouped;
}

inline bool FrameHeader::isGrouped()
{
   return mGrouped;
}

string& FrameHeader::toString(string& str)
{
   str.erase();

   str.append("[ID3TagFrameHeader]\n");
   str.append("Frame ID=");
   str.append(getId());
   str.append("\nFrame Size=");

   char temp[20];
   sprintf(temp, "%i", getFrameSize());
   str.push_back('\n');

   return str;
}

void FrameHeader::convertIntToBytes(int integer, char* b)
{
   unsigned char* ub = (unsigned char*)b;

   // FIXME: remove method, use monarch::Data byte ordering macro

   for(int i = 0; i < 4; i++)
   {
      ub[i] = ((integer >> ((3 - i) * 8)) & 0xFF);
   }
}

int FrameHeader::convertBytesToInt(const char* b)
{
   int rval = 0;

   // FIXME: remove method, use monarch::Data byte ordering macro

   unsigned char* ub = (unsigned char*)b;

   // most significant byte first
   for(int i = 0; i < 4; i++)
   {
      rval |= ((ub[i] & 0xFF) << ((3 - i) * 8));
   }

   return rval;
}
