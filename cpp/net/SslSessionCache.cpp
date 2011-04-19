/*
 * Copyright (c) 2008-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/SslSessionCache.h"

using namespace std;
using namespace monarch::net;
using namespace monarch::util;

SslSessionCache::SslSessionCache(unsigned int capacity) :
   mCapacity(capacity)
{
}

SslSessionCache::~SslSessionCache()
{
   // clean up cache entries
   for(SessionMap::iterator i = mSessions.begin(); i != mSessions.end(); ++i)
   {
      free((char*)i->first);
   }
}

static string _getSessionKey(const char* host, const char* vHost)
{
   string key;
   key.append(host);
   if(vHost != NULL)
   {
      key.push_back(':');
      key.append(vHost);
   }
   return key;
}

void SslSessionCache::storeSession(
   const char* host, SslSession& session, const char* vHost)
{
   // lock to write to cache
   mLock.lockExclusive();
   {
      // find existing session
      string key = _getSessionKey(host, vHost);
      SessionMap::iterator i = mSessions.find(key.c_str());
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
         mSessions.insert(std::make_pair(strdup(key.c_str()), session));
      }
   }
   mLock.unlockExclusive();
}

inline void SslSessionCache::storeSession(
   Url* url, SslSession& session, const char* vHost)
{
   storeSession(url->getAuthority().c_str(), session, vHost);
}

SslSession SslSessionCache::getSession(const char* host, const char* vHost)
{
   SslSession rval(NULL);

   // lock to read from cache
   mLock.lockShared();
   {
      string key = _getSessionKey(host, vHost);
      SessionMap::iterator i = mSessions.find(key.c_str());
      if(i != mSessions.end())
      {
         rval = i->second;
      }
   }
   mLock.unlockShared();

   return rval;
}

inline SslSession SslSessionCache::getSession(Url* url, const char* vHost)
{
   return getSession(url->getAuthority().c_str(), vHost);
}
