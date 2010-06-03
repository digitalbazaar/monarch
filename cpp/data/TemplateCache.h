/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_data_TemplateCache_H
#define monarch_data_TemplateCache_H

#include "monarch/io/InputStream.h"
#include "monarch/rt/SharedLock.h"
#include "monarch/util/StringTools.h"

#include <map>

namespace monarch
{
namespace data
{

/**
 * A TemplateCache provides InputStreams to read templates based on their
 * file names. The input stream may reads the template from disk or from
 * a cache.
 *
 * Note: Future implementations could extend a more generalized FileCache
 * class. Consider integration with memcached if not overkill.
 *
 * Note: Might need the ability to flush the cache in the future.
 *
 * @author Dave Longley
 */
class TemplateCache
{
protected:
   /**
    * A cache entry.
    */
   struct CacheEntry
   {
      int length;
      char* data;
   };

   /**
    * A map of template filename to cached template.
    */
   typedef std::map<const char*, CacheEntry, monarch::util::StringComparator>
      Cache;
   Cache mCache;

   /**
    * A lock for manipulating the cache.
    */
   monarch::rt::SharedLock mLock;

   /**
    * The maximum size for the cache.
    */
   int mCapacity;

   /**
    * The used space in the cache.
    */
   int mUsed;

public:
   /**
    * Creates a new TemplateCache.
    *
    * @param capacity the maximum size to use for the cache, in bytes, -1 for
    *                 no max.
    */
   TemplateCache(int capacity = -1);

   /**
    * Destructs this TemplateCache.
    */
   virtual ~TemplateCache();

   /**
    * Creates an input stream for reading a template. The caller must delete
    * the returned stream when finished.
    *
    * @param filename the filename of the template.
    * @param length to be set to the length of the template.
    *
    * @return a stream for reading the template, NULL on error.
    */
   virtual monarch::io::InputStream* createStream(
      const char* filename, off_t* length = NULL);

protected:
   /**
    * Creates an input stream to read from the cache.
    *
    * @param filename the filename for the template.
    * @param length to be set to the length of the template.
    *
    * @return the stream to read from, NULL if no such entry.
    */
   virtual monarch::io::InputStream* getCacheStream(
      const char* filename, off_t* length);

   /**
    * Caches a template and returns a stream to it.
    *
    * @param filename the filename for the template.
    * @param data the data for the template.
    * @param length the length of the data in bytes.
    * @param outLength to be set to the length of the template.
    *
    * @return an InputStream to read the template from the cache.
    */
   virtual monarch::io::InputStream* cache(
      const char* filename, char* data, int length, off_t* outLength);
};

} // end namespace data
} // end namespace monarch
#endif
