/*
 * Copyright (c) 2009-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_http_HttpConnectionPool_H
#define monarch_http_HttpConnectionPool_H

#include "monarch/http/HttpConnection.h"
#include "monarch/util/StringTools.h"
#include "monarch/util/Url.h"

#include <map>
#include <list>

namespace monarch
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
    * A map of url key to pools of idle http connections.
    */
   typedef std::map<
      const char*, HttpConnectionList*, monarch::util::StringComparator> PoolMap;
   PoolMap mPools;

   /**
    * A lock for manipulating the pool map.
    */
   monarch::rt::ExclusiveLock mPoolsLock;

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
    * @param vHost an optional virtual host identifier, if the URL references
    *           a virtual host in some custom fashion.
    */
   virtual void addConnection(
      monarch::util::Url* url, HttpConnectionRef conn,
      const char* vHost = NULL);

   /**
    * Gets an idle connection from this pool to a particular url. If no
    * connection is available, returns NULL.
    *
    * @param url the url to get a connection to.
    * @param vHost an optional virtual host identifier, if the URL references
    *           a virtual host in some custom fashion.
    *
    * @return the available connection, NULL if none is available.
    */
   virtual HttpConnectionRef getConnection(
      monarch::util::Url* url, const char* vHost = NULL);
};

// typedef for a counted reference to an HttpConnectionPool
typedef monarch::rt::Collectable<HttpConnectionPool> HttpConnectionPoolRef;

} // end namespace http
} // end namespace monarch
#endif
