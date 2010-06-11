/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include "monarch/data/TemplateCache.h"

#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/io/FileInputStream.h"

using namespace std;
using namespace monarch::data;
using namespace monarch::io;
using namespace monarch::rt;

// Note: Future implementations may malloc a block of memory and store all
// templates together there -- with byte array input streams reading the
// appropriate sections for each template.

TemplateCache::TemplateCache(int capacity) :
   mCapacity(capacity),
   mUsed(0)
{
}

TemplateCache::~TemplateCache()
{
   // clean up cache
   for(Cache::iterator i = mCache.begin(); i != mCache.end(); ++i)
   {
      // clean up filename
      free((char*)i->first);

      // clean up entry data
      free(i->second.data);
   }
}

InputStream* TemplateCache::createStream(const char* filename, off_t* length)
{
   // get stream from cache
   InputStream* rval = getCacheStream(filename, length);
   if(rval == NULL)
   {
      // check file
      File file(filename);
      if(file->isReadable())
      {
         // data will NOT fit in cache
         off_t len = file->getLength();
         if(len > INT32_MAX || (mCapacity != -1 && len > (mCapacity - mUsed)))
         {
            if(length != NULL)
            {
               *length = len;
            }
            rval = new FileInputStream(file);
         }
         // data will fit in cache
         else
         {
            // read bytes from disk
            int ilen = (int)len;
            char* data = (char*)malloc(ilen);
            ByteBuffer b(data, 0, 0, ilen, false);
            if(file.readBytes(&b))
            {
               // cache data
               rval = cache(filename, data, ilen, length);
            }
            else
            {
               free(data);
            }
         }
      }
   }

   return rval;
}

InputStream* TemplateCache::getCacheStream(const char* filename, off_t* length)
{
   InputStream* rval = NULL;

   mLock.lockShared();
   {
      Cache::iterator i = mCache.find(filename);
      if(i != mCache.end())
      {
         if(length != NULL)
         {
            *length = i->second.length;
         }
         rval = new ByteArrayInputStream(i->second.data, i->second.length);
      }
   }
   mLock.unlockShared();

   return rval;
}

InputStream* TemplateCache::cache(
   const char* filename, char* data, int length, off_t* outLength)
{
   InputStream* rval = NULL;

   mLock.lockExclusive();

   Cache::iterator i = mCache.find(filename);
   if(i != mCache.end())
   {
      // already in cache, clean up data, get cache stream
      mLock.unlockExclusive();
      free(data);
      rval = getCacheStream(filename, outLength);
   }
   else
   {
      // insert into cache
      CacheEntry e;
      e.length = length;
      e.data = data;
      mCache[strdup(filename)] = e;
      mLock.unlockExclusive();

      // get cache stream
      if(outLength != NULL)
      {
         *outLength = length;
      }
      rval = new ByteArrayInputStream(data, length);
   }

   return rval;
}
