/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_net_SslSessionCache_H
#define monarch_net_SslSessionCache_H

#include "monarch/net/SslSession.h"
#include "monarch/net/Url.h"
#include "monarch/rt/SharedLock.h"
#include "monarch/util/StringTools.h"

#include <map>

namespace monarch
{
namespace net
{

/**
 * An SslSessionCache is a thread-safe cache for SslSessions.
 *
 * @author Dave Longley
 */
class SslSessionCache
{
protected:
   /**
    * A mapping of hostnames to re-usable SSL sessions using case-insensitive
    * comparator to compare hostnames.
    */
   typedef std::map<
      const char*, monarch::net::SslSession,
      monarch::util::StringCaseComparator>
      SessionMap;
   SessionMap mSessions;

   /**
    * Stores the capacity of this cache.
    */
   unsigned int mCapacity;

   /**
    * A lock for modifying the session map.
    */
   monarch::rt::SharedLock mLock;

public:
   /**
    * Creates a new SslSessionCache with the specified capacity.
    *
    * @param capacity the maximum number of sessions to cache.
    */
   SslSessionCache(unsigned int capacity = 50);

   /**
    * Destructs this SslSessionCache.
    */
   virtual ~SslSessionCache();

   /**
    * Stores an SSL session in this cache.
    *
    * @param host the host (including port) for the session.
    * @param session the session to store.
    */
   virtual void storeSession(const char* host, SslSession& session);

   /**
    * Stores an SSL session in this cache.
    *
    * @param url the url for the session.
    * @param session the session to store.
    */
   virtual void storeSession(Url* url, SslSession& session);

   /**
    * Gets a stored SSL session from the cache, if one exists.
    *
    * @param host the host for the session.
    *
    * @return the SslSession (set to NULL if none exists).
    */
   virtual SslSession getSession(const char* host);

   /**
    * Gets a stored SSL session from the cache, if one exists.
    *
    * @param url the url for the session.
    *
    * @return the SslSession (set to NULL if none exists).
    */
   virtual SslSession getSession(Url* url);
};

// type definition for a reference counted SslSessionCache
typedef monarch::rt::Collectable<SslSessionCache> SslSessionCacheRef;

} // end namespace net
} // end namespace monarch
#endif
