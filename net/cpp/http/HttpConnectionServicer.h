/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpConnectionServicer_H
#define db_net_http_HttpConnectionServicer_H

#include "Object.h"
#include "ConnectionServicer.h"
#include "HttpConnection.h"
#include "HttpRequestServicer.h"

#include <map>
#include <string>

namespace db
{
namespace net
{
namespace http
{

typedef struct PathComparator
{
   /**
    * Compares two null-terminated strings, returning true if the first is
    * less than the second, false if not.
    * 
    * @param s1 the first string.
    * @param s2 the second string.
    * 
    * @return true if the s1 < s2, false if not.
    */
   bool operator()(const char* s1, const char* s2) const
   {
      return strcmp(s1, s2) < 0;
   }
};

/**
 * An HttpConnectionServicer services HttpConnections. It maintains a list
 * of HttpRequestServicers that service HttpRequests received over the
 * HttpConnections. One HttpRequestServicer can be assigned per available
 * request path.
 * 
 * @author Dave Longley
 */
class HttpConnectionServicer :
public virtual db::rt::Object, public db::net::ConnectionServicer
{
protected:
   /**
    * The default server name for this servicer.
    */
   std::string mServerName;
   
   /**
    * Non-secure HttpRequestServicers. This maps paths to HttpRequestServicers
    * that require non-secure connections.
    */
   std::map<const char*, HttpRequestServicer*, PathComparator>
      mNonSecureServicers;
   
   /**
    * Secure HttpRequestServicers. This maps paths to HttpRequestServicers
    * that require secure connections.
    */
   std::map<const char*, HttpRequestServicer*, PathComparator>
      mSecureServicers;
   
   /**
    * Ensures a path starts and ends with forward slashes.
    * 
    * @param path the path to normalize.
    */
   virtual void normalizePath(std::string& path);
   
   /**
    * Finds an HttpRequestServicer for the given path in the given map.
    * 
    * @param path the path to search along.
    * @param servicerMap the map to search in.
    * 
    * @return the HttpRequestServicers for the given path.
    */
   virtual HttpRequestServicer* findRequestServicer(
      std::string& path,
      std::map<const char*, HttpRequestServicer*, PathComparator>& servicerMap);
   
public:
   /**
    * Creates a new HttpConnectionServicer with the given default server
    * name.
    * 
    * @param serverName the default server name for this servicer.
    */
   HttpConnectionServicer(
      const std::string& serverName = "Modest Http Server v1.0");
   
   /**
    * Destructs this HttpConnectionServicer.
    */
   virtual ~HttpConnectionServicer();
   
   /**
    * Services the passed Connection. The connection will automatically be
    * closed after it is serviced.
    * 
    * @param c the Connection to service.
    */
   virtual void serviceConnection(Connection* c);
   
   /**
    * Adds an HttpRequestServicer. If a servicer already exists at the new
    * servicer's path it will be replaced with the new servicer.
    * 
    * A servicer may be added as both a secure and non-secure servicer --
    * by calling this method twice, once with secure = false, and once
    * with secure = true.
    * 
    * @param s the HttpRequestServicer to add.
    * @param secure true if this servicer should service only secure
    *               connections, false if it should service only
    *               non-secure connections.
    */
   virtual void addRequestServicer(HttpRequestServicer* s, bool secure);
   
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
    */
   virtual void removeRequestServicer(const char* path, bool secure);
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
