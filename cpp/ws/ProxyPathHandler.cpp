/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/ws/ProxyPathHandler.h"

#include "monarch/http/HttpClient.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/logging/Logging.h"
#include "monarch/net/SocketTools.h"
#include "monarch/util/StringTokenizer.h"
#include "monarch/util/StringTools.h"
#include "monarch/ws/PathHandlerDelegate.h"

#include <algorithm>

using namespace std;
using namespace monarch::http;
using namespace monarch::io;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::ws;

// Note: Current implementation doesn't lock on proxy map. It is assumed it
// will be set up before use and not changed thereafter.

ProxyPathHandler::ProxyPathHandler(const char* path) :
   mPath(strdup(path))
{
}

ProxyPathHandler::~ProxyPathHandler()
{
   free(mPath);
   for(ProxyDomainList::iterator i = mDomains.begin(); i != mDomains.end(); ++i)
   {
      ProxyDomain* pd = *i;
      PathToRule& rules = pd->rules;
      for(PathToRule::iterator ri = rules.begin(); ri != rules.end(); ++ri)
      {
         free((char*)ri->first);
      }
      free(pd->domain);
      delete pd;
   }
}

static PatternRef _compileDomainRegex(const char* domain)
{
   PatternRef rval(NULL);

   string regex = "^";
   regex.append(domain);
   regex.append("$");

   // escape all periods
   StringTools::replaceAll(regex, ".", "\\.");

   // replace all wildcards with (.*)
   StringTools::replaceAll(regex, "*", ".*");

   // try to compile the pattern (match case, no-sub matches allowed)
   rval = Pattern::compile(regex.c_str(), true, false);
   if(rval.isNull())
   {
      ExceptionRef e = new Exception(
         "Could not add proxy domain. Invalid domain format.",
         "monarch.ws.ProxyPathHandler.InvalidDomainFormat");
      e->getDetails()["domain"] = domain;
      e->getDetails()["regex"] = regex.c_str();
      Exception::push(e);
   }

   return rval;
}

static int _countWildcards(const char* domain)
{
   int rval = 0;
   for(; *domain != '\0'; ++domain)
   {
      if(*domain == '*')
      {
         ++rval;
      }
   }
   return rval;
}

/**
 * Sort function that roughly sorts domains by how specific they are. The
 * fewer the wildcards, the "lesser" the value. If wildcard counts are the
 * same, then the shorter string is used.
 *
 * @param a ProxyDomain 1.
 * @param b ProxyDomain 2.
 *
 * @return true if a is less than b.
 */
bool ProxyPathHandler::ProxyDomainSorter::operator()(
   const ProxyDomain* a, const ProxyDomain* b)
{
   bool rval = false;

   // the 'all' wildcard is never less, so skip it
   if(strcmp(a->domain, "*") != 0)
   {
      // a is always less than the 'all' wildcard
      if(strcmp(b->domain, "*") == 0)
      {
         rval = true;
      }
      else
      {
         // count wildcards in the domains
         int wc_a = _countWildcards(a->domain);
         int wc_b = _countWildcards(b->domain);

         // if wildcard counts are the same, the longer string is less specific
         if(wc_a == wc_b)
         {
            // a is less than b if a is longer
            rval = (strlen(a->domain) > strlen(b->domain));
         }
         // if a count is less than b count, a is less
         else if(wc_a < wc_b)
         {
            rval = true;
         }
      }
   }

   return rval;
}

bool ProxyPathHandler::addProxyRule(
   const char* domain, const char* path, const char* url, bool rewriteHost)
{
   return addRule(Rule::Proxy, domain, path, url, rewriteHost, false);
}

bool ProxyPathHandler::addRedirectRule(
   const char* domain, const char* path, const char* url, bool permanent)
{
   return addRule(Rule::Redirect, domain, path, url, false, permanent);
}

/**
 * Proxies HTTP traffic coming from the "in" connection to the "out" connection.
 *
 * @param header the related HTTP header.
 * @param in the incoming HTTP connection.
 * @param out the outgoing HTTP connection.
 *
 * @return true if successful, false if an exception occurred.
 */
static bool _proxyHttp(
   HttpHeader* header, HttpConnection* in, HttpConnection* out)
{
   bool rval;

   // send header
   rval = out->sendHeader(header);

   // see if there is content to proxy
   if(rval && header->hasContent())
   {
      // proxy content
      HttpTrailer trailer;
      InputStream* is = in->getBodyInputStream(header, &trailer);
      rval = out->sendBody(header, is, &trailer);
      is->close();
      delete is;
   }

   return rval;
}

void ProxyPathHandler::operator()(ServiceChannel* ch)
{
   // get request host
   HttpRequestHeader* hrh = ch->getRequest()->getHeader();
   string host = hrh->getFieldValue("X-Forwarded-Host");
   if(host.length() == 0)
   {
      host = hrh->getFieldValue("Host");
   }

   // find a rule
   Rule* rule = findRule(ch, host);
   if(rule == NULL)
   {
      // delegate to restful handler
      RestfulHandler::operator()(ch);
   }
   else
   {
      // get URL to proxy or redirect to
      UrlRef url = rule->url;

      // get url host
      string urlHost;
      HttpResponse* res = ch->getResponse();
      bool secure = res->getConnection()->isSecure();

      // if URL has no host, reuse incoming host
      if(url->getHost().length() == 0)
      {
         urlHost = host;
      }
      // if URL has no port or uses a default port number, only use URL host
      else if(
         (url->getPort() == 0) ||
         (secure && url->getPort() == 443) ||
         (!secure && url->getPort() == 80))
      {
         urlHost = url->getHost();
      }
      // use URL host and port
      else
      {
         urlHost = url->getHostAndPort();
      }

      // handle 0.0.0.0 (any host) by replacing it with the request host
      if(strncmp(urlHost.c_str(), "0.0.0.0", 8) == 0)
      {
         // 0.0.0.0 is 8 chars long
         urlHost.replace(0, 8, host.substr(0, host.find(':')).c_str());
      }

      // rewrite the request path if it does not match URL path
      string path = hrh->getPath();
      if(strcmp(path.c_str(), url->getPath().c_str()) != 0)
      {
         // check for path wildcard
         if(strcmp(rule->path, "*") == 0)
         {
            // since a wildcard is used, prepend the URL path to the
            // path (if the url path isn't '/')
            string urlPath = url->getPath();
            if(urlPath.length() > 1)
            {
               path.insert(0, url->getPath().c_str());
            }
         }
         else
         {
            // replace the part of the path that matched the proxy
            // rule with the rewrite path from the proxy URL
            path.replace(0, strlen(rule->path), url->getPath().c_str());
         }
      }

      // do redirect if appropriate
      if(rule->type == Rule::Redirect)
      {
         // set response code
         HttpResponseHeader* header = res->getHeader();
         if(rule->permanent)
         {
            header->setStatus(301, "Moved Permanently");
         }
         else
         {
            header->setStatus(302, "Found");
         }

         // build new location url
         bool secure = res->getConnection()->isSecure();
         header->setField("Location", StringTools::format("%s://%s%s",
            secure ? "https" : "http", urlHost.c_str(), path.c_str()));
         ch->sendNoContent();
      }
      // do proxy
      else if(rule->type == Rule::Proxy)
      {
         // get client-side request
         HttpRequest* req = ch->getRequest();

         // do path rewrite
         hrh->setPath(path.c_str());

         // add X-Forwarded headers
         hrh->appendFieldValue("X-Forwarded-For",
            req->getConnection()->getRemoteAddress()->toString(true).c_str());
         hrh->appendFieldValue("X-Forwarded-Host",
            hrh->getFieldValue("Host").c_str());
         hrh->appendFieldValue("X-Forwarded-Server",
            SocketTools::getHostname().c_str());

         // rewrite host if rule specifies it
         if(rule->rewriteHost)
         {
            hrh->setField("Host", urlHost.c_str());
         }

         // do proxy:
         MO_CAT_INFO(MO_WS_CAT,
            "ProxyPathHandler proxying %s%s => %s%s",
            host.c_str(), hrh->getPath(), urlHost.c_str(), path.c_str());
         MO_CAT_DEBUG(MO_WS_CAT,
            "ProxyPathHandler request header for %s%s => %s%s:\n%s",
            host.c_str(), hrh->getPath(),
            urlHost.c_str(), path.c_str(),
            hrh->toString().c_str());

         // get a connection
         HttpConnection* conn = HttpClient::createConnection(&(*url));
         if(conn == NULL)
         {
            // send service unavailable
            HttpResponseHeader* header = ch->getResponse()->getHeader();
            header->setStatus(503, "Service Unavailable");
            string content =
               "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n"
               "<html><head>\n"
               "<title>503 Service Unavailable</title>\n"
               "</head><body>\n"
               "<h1>Service Unavailable</h1>\n"
               "<p>The service was not available.</p>\n"
               "</body></html>";
            ByteBuffer b(content.length());
            b.put(content.c_str(), content.length(), false);
            ByteArrayInputStream bais(&b);
            ch->sendContent(&bais);
         }
         else
         {
            // proxy the client's request and receive server's header (by
            // writing it into the client's response header)
            HttpResponse* res = ch->getResponse();
            if(_proxyHttp(req->getHeader(), req->getConnection(), conn) &&
               conn->receiveHeader(res->getHeader()))
            {
               // proxy the server's response, consider result sent
               _proxyHttp(res->getHeader(), conn, req->getConnection());
               ch->setSent();
            }

            // close connection
            conn->close();

            // clean up
            delete conn;
         }

         if(!ch->hasSent())
         {
            // send exception (client's fault if code < 500)
            ExceptionRef e = Exception::get();
            bool clientsFault =
               e->getDetails()->hasMember("httpStatusCode") &&
               e->getDetails()["httpStatusCode"]->getInt32() < 500;
            ch->sendException(e, clientsFault);
         }
      }
   }
}

bool ProxyPathHandler::addRule(
   Rule::RuleType rt, const char* domain, const char* path, const char* url,
   bool rewriteHost, bool permanent)
{
   bool rval = false;

   // try to compile domain regex first
   PatternRef regex = _compileDomainRegex(domain);
   rval = !regex.isNull();
   if(rval)
   {
      // prepend scheme to absolute URLs if needed
      string tmpUrl;
      if(url[0] != '/' &&
         strncmp(url, "http://", 7) != 0 &&
         strncmp(url, "https://", 8) != 0)
      {
         tmpUrl = "http://";
      }
      tmpUrl.append(url);

      /* Build the absolute path that will be searched for in an HTTP request
         so that it can be mapped to the given url. Only append the proxy
         service path if it isn't the root path and only add the given path if
         it isn't a wildcard. The end result will be either a wildcard or the
         proxy service path concatenated with the relative path without having
         a leading double-slash when the proxy service is the root service.

         The code that looks for matches will first look for a domain match,
         and then for the absolute path within that domain, and then for a
         path wildcard if that fails.
      */
      string absPath;
      bool root = (strcmp(mPath, "/") == 0);
      bool pathWildcard = (strcmp(path, "*") == 0);
      if(pathWildcard)
      {
         absPath.push_back('*');
      }
      else
      {
         if(!root)
         {
            absPath.append(mPath);
         }
         absPath.append(path);
      }

      // try to find exact domain
      ProxyDomain* pd = NULL;
      for(ProxyDomainList::iterator i = mDomains.begin();
          pd == NULL && i != mDomains.end(); ++i)
      {
         if(strcmp((*i)->domain, domain) == 0)
         {
            pd = *i;
         }
      }

      // add a new domain if one wasn't found
      if(pd == NULL)
      {
         // insert new domain
         pd = new ProxyDomain;
         pd->domain = strdup(domain);
         pd->regex = regex;
         mDomains.push_back(pd);

         // sort domains
         sort(mDomains.begin(), mDomains.end(), ProxyDomainSorter());
      }

      // remove any duplicate rule
      PathToRule& rules = pd->rules;
      PathToRule::iterator ri = rules.find(absPath.c_str());
      if(ri != rules.end())
      {
         MO_CAT_INFO(MO_WS_CAT,
            "ProxyPathHandler removed %s rule: %s%s/* => %s%s/*",
            ruleTypeToString(ri->second.type),
            domain, (pathWildcard || strlen(ri->first) == 0) ? "" : ri->first,
            (ri->second.url->getHost().length() == 0) ?
               "" : ri->second.url->getHostAndPort().c_str(),
            (ri->second.url->getPath().length() == 1) ?
               "" : ri->second.url->getPath().c_str());

         free((char*)ri->first);
         rules.erase(ri);
      }

      // add new rule and log it
      Rule rule;
      rule.url = new Url(tmpUrl.c_str());
      rule.type = rt;
      if(rule.type == Rule::Redirect)
      {
         rule.permanent = permanent;
      }
      else if(rule.type == Rule::Proxy)
      {
         rule.rewriteHost = rewriteHost;
      }
      rule.path = strdup(absPath.c_str());
      rules[rule.path] = rule;
      MO_CAT_INFO(MO_WS_CAT,
         "ProxyPathHandler added %s rule: %s%s/* => %s%s/*",
         ruleTypeToString(rt),
         domain, (pathWildcard || absPath.length() == 0) ? "" : absPath.c_str(),
         (rule.url->getHost().length() == 0) ?
            "" : rule.url->getHostAndPort().c_str(),
         (rule.url->getPath().length() == 1) ?
            "" : rule.url->getPath().c_str());
   }

   return rval;
}

ProxyPathHandler::Rule* ProxyPathHandler::findRule(
   ServiceChannel* ch, string& host)
{
   ProxyPathHandler::Rule* rval = NULL;

   // strip port number, if any, from the host
   size_t pos = host.find(':');
   if(pos != string::npos)
   {
      host.erase(pos);
   }

   // try to find regex-matching domain
   for(ProxyDomainList::iterator i = mDomains.begin();
       rval == NULL && i != mDomains.end(); ++i)
   {
      ProxyDomain* pd = *i;
      if(pd->regex->match(host.c_str()))
      {
         // try to find the proxy URL based on the incoming absolute path
         PathToRule& rules = pd->rules;
         bool pathWildcard = false;
         string parent;
         string path = ch->getPath();
         PathToRule::iterator ri;
         do
         {
            ri = rules.find(path.c_str());
            if(ri == rules.end())
            {
               string parent = Url::getParentPath(path.c_str());
               if(strcmp(parent.c_str(), path.c_str()) != 0)
               {
                  // haven't hit root path yet, keep checking
                  path = parent;
               }
               else
               {
                  // path not found, check for wildcard path
                  pathWildcard = true;
                  ri = rules.find("*");
               }
            }
         }
         while(ri == rules.end() && !pathWildcard);
         if(ri != rules.end())
         {
            // get rule
            rval = &ri->second;
         }
      }
   }

   return rval;
}

const char* ProxyPathHandler::ruleTypeToString(Rule::RuleType rt)
{
   const char* rval = NULL;

   switch(rt)
   {
      case Rule::Proxy:
         rval = "proxy";
         break;
      case Rule::Redirect:
         rval = "redirect";
         break;
   }

   return rval;
}
