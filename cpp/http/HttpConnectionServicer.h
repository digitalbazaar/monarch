/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_http_HttpConnectionServicer_H
#define monarch_http_HttpConnectionServicer_H

#include "monarch/rt/SharedLock.h"
#include "monarch/net/ConnectionServicer.h"
#include "monarch/http/HttpConnection.h"
#include "monarch/http/HttpRequestServicer.h"
#include "monarch/util/StringTools.h"
#include "monarch/util/regex/Pattern.h"

#include <map>
#include <vector>
#include <string>

namespace monarch
{
namespace http
{

/**
 * An HttpConnectionServicer services HttpConnections.
 *
 * It contains a list of domains with HttpRequestServicers for individual
 * paths in a given domain. HttpRequestServicers may be added to domains
 * in secure and/or non-secure mode. Only one HttpRequestServicer may be
 * specified per domain per path. Domains must be specified as host names
 * without a port. Wildcards (*) may appear in the domain and will be converted
 * into the regex: (.*).
 *
 * For example: '*.mywebsite.com' will produce the regex '(.*)\.mywebsite\.com'
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
    * A map of path to HttpRequestServicer.
    */
   typedef std::map<
      const char*, HttpRequestServicer*, monarch::util::StringComparator>
      ServicerMap;

   /**
    * A ServiceDomain contains a regex for domains that match the 'Host' header
    * in HTTP requests (ignoring the port) and lists of secure and non-secure
    * HttpRequestServicers for individual paths on that domain (for a host that
    * matches the regex).
    */
   struct ServiceDomain
   {
      char* domain;
      monarch::util::regex::PatternRef regex;
      ServicerMap nonSecureMap;
      ServicerMap secureMap;
   };

   /**
    * Sorts ServiceDomains.
    */
   struct ServiceDomainSorter
   {
      bool operator()(const ServiceDomain* a, const ServiceDomain* b);
   };

   /**
    * The ServiceDomains supported by this HttpConnectionServicer.
    */
   typedef std::vector<ServiceDomain*> ServiceDomainList;
   ServiceDomainList mDomains;

   /**
    * A lock for manipulating service domains.
    */
   monarch::rt::SharedLock mDomainLock;

public:
   /**
    * Creates a new HttpConnectionServicer with the given default server
    * name.
    *
    * @param serverName the default server name for this servicer.
    */
   HttpConnectionServicer(
      const char* serverName = "Monarch Http Server v1.0");

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
    * Adds an HttpRequestServicer to a domain. If a servicer with the same
    * path, at the same given domain, and with the same security status already
    * exists, this method will set an exception and return false.
    *
    * To add the same servicer in both secure and non-secure mode, then call
    * this method twice, once with secure = false, and once with secure = true.
    *
    * The domain parameter must be a valid host name (without a port) or a
    * wildcard, for example:
    *
    * *.mywebsite.com
    * *.mywebsite.*
    *
    * @param s the HttpRequestServicer to add.
    * @param secure true if this servicer should service only secure
    *           connections, false if it should service only
    *           non-secure connections.
    * @param domain the domain for the servicer, defaults to "*" (all domains).
    *
    * @return true if successful, false if not.
    */
   virtual bool addRequestServicer(
      HttpRequestServicer* s, bool secure, const char* domain = "*");

   /**
    * Removes an HttpRequestServicer.
    *
    * If a servicer services both secure and non-secure connections and
    * the caller wants to remove the servicer completely, the caller must
    * call this method twice, once with secure = false, and once with
    * secure = true.
    *
    * @param s the HttpRequestServicer to remove.
    * @param secure true to remove a secure servicer, false to remove a
    *           non-secure servicer.
    * @param domain the domain for the servicer, defaults to "*" (all domains).
    */
   virtual void removeRequestServicer(
      HttpRequestServicer* s, bool secure, const char* domain = "*");

   /**
    * Removes the HttpRequestServicer at the given domain and path. Only the
    * given path will be checked for a servicer to remove, not any of the path's
    * parents.
    *
    * If a servicer services both secure and non-secure connections and
    * the caller wants to remove the servicer completely, the caller must
    * call this method twice, once with secure = false, and once with
    * secure = true.
    *
    * @param path the path to remove a servicer from.
    * @param secure true to remove a secure servicer, false to remove a
    *           non-secure servicer.
    * @param domain the domain for the servicer, defaults to "*" (all domains).
    *
    * @return the HttpRequestServicer that was removed, or NULL if none was.
    */
   virtual HttpRequestServicer* removeRequestServicer(
      const char* path, bool secure, const char* domain = "*");

   /**
    * Gets the HttpRequestServicer at the given domain and path. Only the given
    * path will be checked for a servicer, not any of the path's parents.
    *
    * @param path the path to get the servicer for.
    * @param secure true if the servicer is secure, false if it is non-secure.
    * @param domain the domain for the servicer, defaults to "*" (all domains).
    *
    * @return the HttpRequestServicer that was found, or NULL if none was.
    */
   virtual HttpRequestServicer* getRequestServicer(
      const char* path, bool secure, const char* domain = "*");

protected:
   /**
    * Finds an HttpRequestServicer for the given path in the given map.
    *
    * @param host the host from the HTTP 'Host' header.
    * @param path the path to search along.
    * @param secure true to find a secure servicer, false for a non-secure one.
    *
    * @return the HttpRequestServicers for the given path.
    */
   virtual HttpRequestServicer* findRequestServicer(
      std::string& host, char* path, bool secure);
};

} // end namespace http
} // end namespace monarch
#endif
