/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_SslSessionCache_H
#define db_net_SslSessionCache_H

#include "db/net/SslSession.h"
#include "db/net/Url.h"
#include "db/rt/SharedLock.h"

#include <map>

namespace db
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
    * A HostnameComparator compares two hostnames.
    */
   struct HostnameComparator
   {
      /**
       * Compares two null-terminated strings, returning true if the first is
       * less than the second, false if not. The compare is case-insensitive.
       * 
       * @param s1 the first string.
       * @param s2 the second string.
       * 
       * @return true if the s1 < s2, false if not.
       */
      bool operator()(const char* s1, const char* s2) const
      {
         return strcasecmp(s1, s2) < 0;
      }
   };
   
   /**
    * A mapping of hostnames to re-usable SSL sessions.
    */
   typedef std::map<const char*, db::net::SslSession, HostnameComparator>
      SessionMap;
   SessionMap mSessions;
   
   /**
    * Stores the capacity of this cache.
    */
   unsigned int mCapacity;
   
   /**
    * A lock for modifying the session map.
    */
   db::rt::SharedLock mLock;
   
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
typedef db::rt::Collectable<SslSessionCache> SslSessionCacheRef;

} // end namespace net
} // end namespace db
#endif
