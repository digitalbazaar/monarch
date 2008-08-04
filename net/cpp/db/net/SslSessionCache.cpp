/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/SslSessionCache.h"

using namespace db::net;

SslSessionCache::SslSessionCache(unsigned int capacity)
{
   
}

SslSessionCache::~SslSessionCache()
{
   // clean up cache entries
   for(SessionMap::iterator i = mSessions.begin(); i != mSessions.end(); i++)
   {
      free((char*)i->first);
   }
}

void SslSessionCache::storeSession(const char* host, SslSession& session)
{
   // lock to write to cache
   mLock.lockExclusive();
   {
      // find existing session
      SessionMap::iterator i = mSessions.find(host);
      if(i != mSessions.end())
      {
         // update existing entry
         i->second = session;
      }
      else if(mSessions.size() >= mCapacity)
      {
         // free up space in the map
         i = mSessions.begin();
         free((char*)i->first);
         mSessions.erase(i);
      }
      else
      {
         // insert new entry
         mSessions.insert(std::make_pair(strdup(host), session));
      }
   }
   mLock.unlockExclusive();
}

inline void SslSessionCache::storeSession(Url* url, SslSession& session)
{
   storeSession(url->getAuthority().c_str(), session);
}

SslSession SslSessionCache::getSession(const char* host)
{
   SslSession rval(NULL);
   
   // lock to read from cache
   mLock.lockShared();
   {
      SessionMap::iterator i = mSessions.find(host);
      if(i != mSessions.end())
      {
         rval = i->second;
      }
   }
   mLock.unlockShared();
   
   return rval;
}

inline SslSession SslSessionCache::getSession(Url* url)
{
   return getSession(url->getAuthority().c_str());
}
