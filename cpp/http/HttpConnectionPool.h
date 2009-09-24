/*
 * Copyright (c) 2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_http_HttpConnectionPool_H
#define db_http_HttpConnectionPool_H

#include "db/http/HttpConnection.h"
#include "db/net/Url.h"
#include "db/util/StringTools.h"

#include <map>
#include <list>

namespace db
{
namespace http
{

/**
 * An HttpConnectionPool pools a number of HttpConnections together so that
 * they can be reused.
 *
 * The current implementation does not handle creating connections, it simply
 * stores idle connections so that they can be reused.
 *
 * @author Dave Longley
 */
class HttpConnectionPool
{
protected:
   /**
    * A list of http connections.
    */
   typedef std::list<HttpConnectionRef> HttpConnectionList;

   /**
    * A map of url string to pools of idle http connections.
    */
   typedef std::map<
      const char*, HttpConnectionList*, db::util::StringComparator> PoolMap;
   PoolMap mPools;

   /**
    * A lock for manipulating the pool map.
    */
   db::rt::ExclusiveLock mPoolsLock;

public:
   /**
    * Creates a new HttpConnectionPool.
    */
   HttpConnectionPool();

   /**
    * Destructs this HttpConnectionPool.
    */
   virtual ~HttpConnectionPool();

   /**
    * Adds an idle connection to this pool.
    *
    * @param url the url for the connection.
    * @param conn the idle connection to add.
    */
   virtual void addConnection(db::net::Url* url, HttpConnectionRef conn);

   /**
    * Gets an idle connection from this pool to a particular url. If no
    * connection is available, returns NULL.
    *
    * @param url the url to get a connection to.
    *
    * @return the available connection, NULL if none is available.
    */
   virtual HttpConnectionRef getConnection(db::net::Url* url);
};

// typedef for a counted reference to an HttpConnectionPool
typedef db::rt::Collectable<HttpConnectionPool> HttpConnectionPoolRef;

} // end namespace http
} // end namespace db
#endif
