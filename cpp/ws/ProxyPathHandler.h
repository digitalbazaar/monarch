/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_ProxyPathHandler_H
#define monarch_ws_ProxyPathHandler_H

#include "monarch/ws/RestfulHandler.h"
#include "monarch/util/Pattern.h"

namespace monarch
{
namespace ws
{

/**
 * A ProxyPathHandler redirects HTTP traffic to another URL. This may be
 * done through HTTP redirects or through proxying.
 *
 * An HTTP request is processed by this handler as follows:
 *
 * 1. Do proxy or redirection rules.
 * 2. Do local path handling.
 *
 * In pseudo-code:
 *
 * If there is a proxy or redirect rule, do it.
 *    Done.
 * Else delegate to RestfulHandler.
 *    Done.
 *
 * @author Dave Longley
 */
class ProxyPathHandler : public monarch::ws::RestfulHandler
{
protected:
   /**
    * The path to this handler.
    */
   char* mPath;

   /**
    * Data for a proxy or redirect rule.
    */
   struct Rule
   {
      /**
       * Types for rules.
       */
      enum RuleType
      {
         Proxy,
         Redirect
      } type;
      monarch::util::UrlRef url;
      union
      {
         bool rewriteHost;
         bool permanent;
      };
      const char* path;
   };

   /**
    * A map of incoming path to rule.
    */
   typedef std::map<const char*, Rule, monarch::util::StringComparator>
      PathToRule;

   /**
    * A list of domains with proxy rules.
    */
   struct ProxyDomain
   {
      char* domain;
      monarch::util::PatternRef regex;
      PathToRule rules;
   };

   /**
    * Sorts ProxyDomains.
    */
   struct ProxyDomainSorter
   {
      bool operator()(const ProxyDomain* a, const ProxyDomain* b);
   };

   /**
    * The ProxyDomains supported by this handler.
    */
   typedef std::vector<ProxyDomain*> ProxyDomainList;
   ProxyDomainList mDomains;

public:
   /**
    * Creates a new ProxyPathHandler.
    *
    * @param path the path to this handler.
    */
   ProxyPathHandler(const char* path);

   /**
    * Destructs this ProxyPathHandler.
    */
   virtual ~ProxyPathHandler();

   /**
    * Adds a proxy rule. If the given domain and path are received in an
    * HTTP request, that request will be proxied to the given URL and the
    * proceeding response will be proxied back to the client. The URLs may be
    * relative or absolute and a path value of '*' will proxy all paths for
    * the given domain.
    *
    * Any sub-path of the given path will also be proxied.
    *
    * The domain parameter must be a valid host name (without a port) or a
    * wildcard, for example:
    *
    * *.mywebsite.com
    * *.mywebsite.*
    *
    * It will be matched against the 'Host' header in an incoming HTTP request.
    *
    * Note: The given path will be interpreted relative to the path of the
    * proxy handler. This means that if the proxy handler is for the path:
    *
    * /path/to/handler
    *
    * Then a path of "/foo/bar" will look for "/path/to/handler/foo/bar" in
    * the HTTP request.
    *
    * @param domain the incoming domain, "*" for any host.
    * @param path the incoming path to map to another URL, relative to the
    *           proxy handler's path.
    * @param url the URL to map to, which may be relative or absolute and
    *           will, at proxy time, have any sub-paths appended to it.
    * @param rewriteHost true to rewrite the host header, false not to.
    *
    * @return true if successful, false if not.
    */
   virtual bool addProxyRule(
      const char* domain, const char* path, const char* url, bool rewriteHost);

   /**
    * Adds a redirection rule. If the given domain and path are received in
    * an HTTP request, that request will be responded to with a 30x redirect to
    * the given URL. The URLs may be relative (the host in the request will be
    * used) or absolute and a path value of '*' will redirect all paths for the
    * given domain.
    *
    * Any sub-path of the given path will also be proxied/redirected.
    *
    * The domain parameter must be a valid host name (without a port) or a
    * wildcard, for example:
    *
    * *.mywebsite.com
    * *.mywebsite.*
    *
    * It will be matched against the 'Host' header in an incoming HTTP request.
    *
    * Note: The given path will be interpreted relative to the path of the
    * proxy handler. This means that if the proxy handler is for the path:
    *
    * /path/to/handler
    *
    * Then a path of "/foo/bar" will look for "/path/to/handler/foo/bar" in
    * the HTTP request.
    *
    * @param domain the incoming domain, "*" for any host.
    * @param path the incoming path to map to another URL, relative to the
    *           proxy handler's path.
    * @param url the URL to map to, which may be relative or absolute and
    *           will, at redirect time, have any sub-paths appended to it.
    * @param permanent true to send a permanent redirect response, false to
    *           send a temporary one.
    *
    * @return true if successful, false if not.
    */
   virtual bool addRedirectRule(
      const char* domain, const char* path, const char* url, bool permanent);

   /**
    * Proxies incoming HTTP traffic to another server or redirects a user-agent
    * to another url.
    *
    * HTTP equivalent: METHOD .../
    *
    * @param ch the communication channel with the client.
    */
   virtual void operator()(ServiceChannel* ch);

protected:
   /**
    * Adds a proxy or redirection rule.
    *
    * @param rt the rule type.
    * @param domain the incoming domain, "*" for any host.
    * @param path the incoming path to map to another URL, relative to the
    *           proxy handler's path.
    * @param url the URL to map to, which may be relative or absolute and
    *           will, at redirect time, have any sub-paths appended to it.
    * @param rewriteHost true to rewrite the host header, false not to.
    * @param permanent true to send a permanent redirect response, false to
    *           send a temporary one.
    *
    * @return true if successful, false if not.
    */
   virtual bool addRule(
      Rule::RuleType rt,
      const char* domain, const char* path, const char* url,
      bool rewriteHost, bool permanent);

   /**
    * Gets the Rule for the given channel.
    *
    * @param ch the channel to get the Rule for.
    * @param host the request host.
    *
    * @return the Rule or NULL if none was found.
    */
   virtual Rule* findRule(ServiceChannel* ch, std::string& host);

   /**
    * Gets the string representation for the given rule type.
    *
    * @param rt the RuleType.
    *
    * @return the string representation.
    */
   static const char* ruleTypeToString(Rule::RuleType rt);
};

} // end namespace ws
} // end namespace monarch
#endif
