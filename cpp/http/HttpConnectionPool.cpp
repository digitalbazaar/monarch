/*
 * Copyright (c) 2009-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpConnectionPool.h"

#include <algorithm>

using namespace std;
using namespace monarch::http;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;

HttpConnectionPool::HttpConnectionPool()
{
}

HttpConnectionPool::~HttpConnectionPool()
{
   for(PoolMap::iterator i = mPools.begin(); i != mPools.end(); ++i)
   {
      // clean up url key
      free((char*)i->first);

      // close any open connections
      HttpConnectionList* pool = i->second;
      for(HttpConnectionList::iterator hi = pool->begin();
          hi != pool->end(); ++hi)
      {
         if(!(*hi)->isClosed())
         {
            (*hi)->close();
            (*hi).setNull();
         }
      }

      // clean up pool
      delete pool;
   }
}

static string _getUrlKey(Url* url, const char* vHost)
{
   // build url key
   string key = url->getSchemeHostAndPort();
   if(vHost != NULL)
   {
      key.push_back(':');
      key.append(vHost);
   }
   return key;
}

void HttpConnectionPool::addConnection(
   Url* url, HttpConnectionRef conn, const char* vHost)
{
   mPoolsLock.lock();
   {
      // get url key
      string key = _getUrlKey(url, vHost);

      // find existing connection list
      HttpConnectionList* pool = NULL;
      PoolMap::iterator i = mPools.find(key.c_str());
      if(i != mPools.end())
      {
         pool = i->second;
      }
      else
      {
         // no existing list, so create one
         pool = new HttpConnectionList;
         mPools.insert(make_pair(strdup(key.c_str()), pool));
      }

      // add connection to pool
      pool->push_back(conn);
   }
   mPoolsLock.unlock();
}

HttpConnectionRef HttpConnectionPool::getConnection(Url* url, const char* vHost)
{
   HttpConnectionRef rval(NULL);

   mPoolsLock.lock();
   {
      // get url key
      string key = _getUrlKey(url, vHost);

      // get existing connection list
      PoolMap::iterator i = mPools.find(key.c_str());
      if(i != mPools.end())
      {
         // keep popping connections until one that isn't closed is found
         while(rval.isNull() && !i->second->empty())
         {
            rval = i->second->front();
            i->second->pop_front();

            // drop connection if its closed
            if(rval->isClosed())
            {
               rval.setNull();
            }
         }

         if(rval.isNull())
         {
            // no idle connection available, drop connection list
            free((char*)i->first);
            delete i->second;
            mPools.erase(i);
         }
      }
   }
   mPoolsLock.unlock();

   return rval;
}
