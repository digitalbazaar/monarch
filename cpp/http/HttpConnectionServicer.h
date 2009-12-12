/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_http_HttpConnectionServicer_H
#define db_http_HttpConnectionServicer_H

#include "monarch/rt/SharedLock.h"
#include "monarch/net/ConnectionServicer.h"
#include "monarch/http/HttpConnection.h"
#include "monarch/http/HttpRequestServicer.h"
#include "monarch/util/StringTools.h"

#include <map>
#include <string>

namespace db
{
namespace http
{

/**
 * An HttpConnectionServicer services HttpConnections. It maintains a list
 * of HttpRequestServicers that service HttpRequests received over the
 * HttpConnections. One HttpRequestServicer can be assigned per available
 * request path.
 *
 * @author Dave Longley
 */
class HttpConnectionServicer : public monarch::net::ConnectionServicer
{
protected:
   /**
    * The default server name for this servicer.
    */
   char* mServerName;

   /**
    * Non-secure HttpRequestServicers. This maps paths to HttpRequestServicers
    * that require non-secure connections.
    */
   typedef std::map<
      const char*, HttpRequestServicer*, monarch::util::StringComparator>
      ServicerMap;
   ServicerMap mNonSecureServicers;

   /**
    * Secure HttpRequestServicers. This maps paths to HttpRequestServicers
    * that require secure connections.
    */
   ServicerMap mSecureServicers;

   /**
    * A lock for manipulating request servicers.
    */
   monarch::rt::SharedLock mRequestServicerLock;

   /**
    * Finds an HttpRequestServicer for the given path in the given map.
    *
    * @param path the path to search along.
    * @param servicerMap the map to search in.
    *
    * @return the HttpRequestServicers for the given path.
    */
   virtual HttpRequestServicer* findRequestServicer(
      char* path, ServicerMap& servicerMap);

public:
   /**
    * Creates a new HttpConnectionServicer with the given default server
    * name.
    *
    * @param serverName the default server name for this servicer.
    */
   HttpConnectionServicer(
      const char* serverName = "Modest Http Server v1.0");

   /**
    * Destructs this HttpConnectionServicer.
    */
   virtual ~HttpConnectionServicer();

   /**
    * Services the passed Connection. This method should end by closing the
    * passed Connection. After this method returns, the Connection will be
    * automatically closed, if necessary, and cleaned up.
    *
    * @param c the Connection to service.
    */
   virtual void serviceConnection(monarch::net::Connection* c);

   /**
    * Adds an HttpRequestServicer. If a servicer already exists at the given
    * path, this method will set an exception and return false.
    *
    * However, w servicer may be added as both a secure and non-secure
    * servicer -- by calling this method twice, once with secure = false, and
    * once with secure = true.
    *
    * @param s the HttpRequestServicer to add.
    * @param secure true if this servicer should service only secure
    *               connections, false if it should service only
    *               non-secure connections.
    *
    * @return true if successful, false if not.
    */
   virtual bool addRequestServicer(HttpRequestServicer* s, bool secure);

   /**
    * Removes an HttpRequestServicer.
    *
    * If a servicer servicers both secure and non-secure connections and
    * the caller wants to remove the servicer completely, the caller must
    * call this method twice, once with secure = false, and once with
    * secure = true.
    *
    * @param s the HttpRequestServicer to remove.
    * @param secure true to remove a secure servicer, false to remove a
    *               non-secure servicer.
    */
   virtual void removeRequestServicer(HttpRequestServicer* s, bool secure);

   /**
    * Removes the HttpRequestServicer at the given path. Only the given
    * path will be checked for a servicer to remove, not any of the path's
    * parents.
    *
    * If a servicer servicers both secure and non-secure connections and
    * the caller wants to remove the servicer completely, the caller must
    * call this method twice, once with secure = false, and once with
    * secure = true.
    *
    * @param path the path to remove a servicer from.
    * @param secure true to remove a secure servicer, false to remove a
    *               non-secure servicer.
    *
    * @return the HttpRequestServicer that was removed, or NULL if none was.
    */
   virtual HttpRequestServicer* removeRequestServicer(
      const char* path, bool secure);
};

} // end namespace http
} // end namespace db
#endif
