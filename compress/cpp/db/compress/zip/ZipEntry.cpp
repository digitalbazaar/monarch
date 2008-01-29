/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/compress/zip/ZipEntry.h"

#include "db/rt/System.h"

using namespace db::compress::zip;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

ZipEntryImpl::ZipEntryImpl()
{
   mFilename = strdup("");
   mFileComment = strdup("");
   setDate(NULL);
   mCompressedSize = 0;
   mUncompressedSize = 0;
   mCrc32 = 0;
   mLocalHeaderOffset = 0;
}

ZipEntryImpl::~ZipEntryImpl()
{
   free(mFilename);
   free(mFileComment);
}

unsigned int ZipEntryImpl::getLocalFileHeaderSize()
{
   return 30 + strlen(mFilename) + strlen(mFileComment);
}

unsigned int ZipEntryImpl::getFileHeaderSize()
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

void ZipEntryImpl::setDosTime(unsigned int dosTime)
{
   mDosTime = dosTime;
}

unsigned int ZipEntryImpl::getDosTime()
{
   return mDosTime;
}

void ZipEntryImpl::setCompressedSize(unsigned int size)
{
   mCompressedSize = size;
}

unsigned int ZipEntryImpl::getCompressedSize()
{
   return mCompressedSize;
}

void ZipEntryImpl::setUncompressedSize(unsigned int size)
{
   mUncompressedSize = size;
}

unsigned int ZipEntryImpl::getUncompressedSize()
{
   return mUncompressedSize;
}

void ZipEntryImpl::setCrc32(unsigned int crc)
{
   mCrc32 = crc;
}

unsigned int ZipEntryImpl::getCrc32()
{
   return mCrc32;
}

void ZipEntryImpl::setLocalFileHeaderOffset(unsigned int offset)
{
   mLocalHeaderOffset = offset;
}

unsigned int ZipEntryImpl::getLocalFileHeaderOffset()
{
   return mLocalHeaderOffset;
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
