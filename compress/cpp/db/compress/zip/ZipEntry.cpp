/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/compress/zip/ZipEntry.h"

#include "db/rt/System.h"

#include <cstdlib>
#include <cstring>

using namespace db::compress::zip;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

// define compression methods (DEFLATE = 8)
const uint16_t ZipEntry::COMPRESSION_NONE = 0;
const uint16_t ZipEntry::COMPRESSION_DEFLATE = 8;

ZipEntryImpl::ZipEntryImpl() :
   mCompressedSize(0),
   mUncompressedSize(0),
   mCrc32(0),
   mLocalHeaderOffset(0),
   mCompressionMethod(ZipEntry::COMPRESSION_DEFLATE)
{
   mFilename = strdup("");
   mFileComment = strdup("");
   setDate(NULL);
}

ZipEntryImpl::~ZipEntryImpl()
{
   free(mFilename);
   free(mFileComment);
}

uint32_t ZipEntryImpl::getLocalFileHeaderSize()
{
   return 30 + strlen(mFilename) + strlen(mFileComment);
}

uint32_t ZipEntryImpl::getFileHeaderSize()
{
   return 46 + strlen(mFilename) + strlen(mFileComment);
}

void ZipEntryImpl::setFilename(const char* filename)
{
   free(mFilename);
   
   // Note: The filename must be normalized such that it
   // does not contain a drive letter or leading slash and
   // such that all slashes are forward slashes. It can
   // contain a relative path.
   
   // FIXME: this needs a more full implementation to remove
   // drive letters, etc
   
   // remove leading slash
   while(filename[0] != 0 && filename[0] == '/')
   {
      filename++;
   }
   
   mFilename = strdup(filename);
}

const char* ZipEntryImpl::getFilename()
{
   return mFilename;
}

void ZipEntryImpl::setFileComment(const char* comment)
{
   free(mFileComment);
   mFileComment = strdup(comment);
}

const char* ZipEntryImpl::getFileComment()
{
   return mFileComment;
}

void ZipEntryImpl::setDate(Date* date)
{
   if(date == NULL)
   {
      // use current time
      Date d;
      mDosTime = d.dosTime();
   }
   else
   {
      mDosTime = date->dosTime();
   }
}

void ZipEntryImpl::setDosTime(uint32_t dosTime)
{
   mDosTime = dosTime;
}

uint32_t ZipEntryImpl::getDosTime()
{
   return mDosTime;
}

void ZipEntryImpl::setCompressedSize(uint32_t size)
{
   mCompressedSize = size;
}

uint32_t ZipEntryImpl::getCompressedSize()
{
   return mCompressedSize;
}

void ZipEntryImpl::setUncompressedSize(uint32_t size)
{
   mUncompressedSize = size;
}

uint32_t ZipEntryImpl::getUncompressedSize()
{
   return mUncompressedSize;
}

void ZipEntryImpl::setCrc32(uint32_t crc)
{
   mCrc32 = crc;
}

uint32_t ZipEntryImpl::getCrc32()
{
   return mCrc32;
}

void ZipEntryImpl::setLocalFileHeaderOffset(uint32_t offset)
{
   mLocalHeaderOffset = offset;
}

uint32_t ZipEntryImpl::getLocalFileHeaderOffset()
{
   return mLocalHeaderOffset;
}

void ZipEntryImpl::disableCompression(bool disable)
{
   mCompressionMethod =
      (disable ? ZipEntry::COMPRESSION_NONE : ZipEntry::COMPRESSION_DEFLATE);
}

uint16_t ZipEntryImpl::getCompressionMethod()
{
   return mCompressionMethod;
}

ZipEntry::ZipEntry() :
   Collectable<ZipEntryImpl>(new ZipEntryImpl())
{
}

ZipEntry::ZipEntry(ZipEntryImpl* impl) :
   Collectable<ZipEntryImpl>(impl)
{
}

ZipEntry::~ZipEntry()
{
}
