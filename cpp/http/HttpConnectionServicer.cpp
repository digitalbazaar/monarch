/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpConnectionServicer.h"

#include "monarch/http/HttpRequest.h"
#include "monarch/http/HttpResponse.h"
#include "monarch/io/ByteArrayInputStream.h"
#include "monarch/logging/Logging.h"
#include "monarch/rt/DynamicObjectIterator.h"

#include <cstdlib>
#include <algorithm>

using namespace std;
using namespace monarch::io;
using namespace monarch::http;
using namespace monarch::logging;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::util::regex;

HttpConnectionServicer::HttpConnectionServicer(const char* serverName) :
   mServerName(strdup(serverName)),
   mRequestModifier(NULL)
{
}

HttpConnectionServicer::~HttpConnectionServicer()
{
   free(mServerName);

   // free domains
   for(ServiceDomainList::iterator i = mDomains.begin();
       i != mDomains.end(); i++)
   {
      ServiceDomain* sd = *i;
      free(sd->domain);
      delete sd;
   }
}

void HttpConnectionServicer::setRequestModifier(HttpRequestModifier* hrm)
{
   mRequestModifier = hrm;
}

HttpRequestModifier* HttpConnectionServicer::getRequestModifier()
{
   return mRequestModifier;
}

void HttpConnectionServicer::serviceConnection(Connection* c)
{
   // wrap connection, set default timeouts to 30 seconds
   HttpConnection hc(c, false);
   hc.setReadTimeout(30000);
   hc.setWriteTimeout(30000);

   // create request
   HttpRequest* request = hc.createRequest();
   HttpRequestHeader* reqHeader = request->getHeader();

   // create response
   HttpResponse* response = request->createResponse();
   HttpResponseHeader* resHeader = response->getHeader();

   // handle keep-alive (HTTP/1.1 keep-alive is on by default)
   bool keepAlive = true;
   bool noerror = true;
   while(keepAlive && noerror)
   {
      // set defaults
      resHeader->setVersion("HTTP/1.1");
      resHeader->setDate();
      resHeader->setField("Server", mServerName);

      // receive request header
      if((noerror = request->receiveHeader()))
      {
         // do request modification
         if(mRequestModifier != NULL)
         {
            mRequestModifier->modifyRequest(request);
         }

         // check http version
         bool version10 = (strcmp(reqHeader->getVersion(), "HTTP/1.0") == 0);
         bool version11 = (strcmp(reqHeader->getVersion(), "HTTP/1.1") == 0);

         // only version 1.0 and 1.1 supported
         if(version10 || version11)
         {
            // set response version according to request version
            resHeader->setVersion(reqHeader->getVersion());

            // use proxy'd host field if one was used
            // else use host field if one was used
            string host;
            if(reqHeader->getField("X-Forwarded-Host", host) ||
               reqHeader->getField("Host", host))
            {
               resHeader->setField("Host", host);
            }

            // get connection header
            string connHeader;
            if(reqHeader->getField("Connection", connHeader))
            {
               if(strcasecmp(connHeader.c_str(), "close") == 0)
               {
                  keepAlive = false;
               }
               else if(strcasecmp(connHeader.c_str(), "keep-alive") == 0)
               {
                  keepAlive = true;
               }
            }
            else if(version10)
            {
               // if HTTP/1.0 and no keep-alive header, keep-alive is off
               keepAlive = false;
            }

            // get request path and normalize it
            const char* inPath = reqHeader->getPath();
            char outPath[strlen(inPath) + 2];
            HttpRequestServicer::normalizePath(inPath, outPath);

            // find appropriate request servicer for path
            HttpRequestServicer* hrs = NULL;

            // find secure/non-secure servicer
            hrs = findRequestServicer(host, outPath, hc.isSecure());
            if(hrs != NULL)
            {
               // service request
               hrs->serviceRequest(request, response);

               // if servicer closed connection, turn off keep-alive
               if(c->isClosed())
               {
                  keepAlive = false;
               }

               // turn off keep-alive if response has close connection field
               if(keepAlive)
               {
                  if(resHeader->getField("Connection", connHeader) &&
                     strcasecmp(connHeader.c_str(), "close") == 0)
                  {
                     keepAlive = false;
                  }
               }
            }
            else
            {
               // no servicer, so send 404 Not Found
               const char* html =
                  "<html><body><h2>404 Not Found</h2></body></html>";
               resHeader->setStatus(404, "Not Found");
               resHeader->setField("Content-Type", "text/html");
               resHeader->setField("Content-Length", 48);
               resHeader->setField("Connection", "close");
               if((noerror = response->sendHeader()))
               {
                  ByteArrayInputStream is(html, 48);
                  noerror = response->sendBody(&is);
               }
            }
         }
         else
         {
            // send 505 HTTP Version Not Supported
            const char* html =
               "<html><body>"
               "<h2>505 HTTP Version Not Supported</h2>"
               "</body></html>";
            resHeader->setStatus(505, "HTTP Version Not Supported");
            resHeader->setField("Content-Type", "text/html");
            resHeader->setField("Content-Length", 65);
            resHeader->setField("Connection", "close");
            if((noerror = response->sendHeader()))
            {
               ByteArrayInputStream is(html, 65);
               noerror = response->sendBody(&is);
            }
         }
      }
      else
      {
         // exception occurred while receiving header
         ExceptionRef e = Exception::get();
         if(e->isType("monarch.net.http.BadHeader") ||
            e->isType("monarch.net.http.BadRequest"))
         {
            // send 400 Bad Request
            const char* html =
               "<html><body><h2>400 Bad Request</h2></body></html>";
            response->getHeader()->setStatus(400, "Bad Request");
            response->getHeader()->setField("Content-Type", "text/html");
            response->getHeader()->setField("Content-Length", 50);
            response->getHeader()->setField("Connection", "close");
            if(response->sendHeader())
            {
               ByteArrayInputStream is(html, 50);
               response->sendBody(&is);
            }
         }
         // if the exception was an interruption, then send a 503
         else if(e->isType("monarch.io.InterruptedException") ||
                 e->isType("monarch.rt.Interrupted"))
         {
            // send 503 Service Unavailable
            const char* html =
               "<html><body><h2>503 Service Unavailable</h2></body></html>";
            resHeader->setStatus(503, "Service Unavailable");
            resHeader->setField("Content-Type", "text/html");
            resHeader->setField("Content-Length", 58);
            resHeader->setField("Connection", "close");
            if((noerror = response->sendHeader()))
            {
               ByteArrayInputStream is(html, 58);
               noerror = response->sendBody(&is);
            }
         }
         // if the exception was not a socket error then send an internal
         // server error response
         else if(!e->isType("monarch.net.Socket", true))
         {
            // send 500 Internal Server Error
            const char* html =
               "<html><body><h2>500 Internal Server Error</h2></body></html>";
            resHeader->setStatus(500, "Internal Server Error");
            resHeader->setField("Content-Type", "text/html");
            resHeader->setField("Content-Length", 60);
            resHeader->setField("Connection", "close");
            if((noerror = response->sendHeader()))
            {
               ByteArrayInputStream is(html, 60);
               noerror = response->sendBody(&is);
            }
         }
         else
         {
            // log socket error
            if(e->getDetails()->hasMember("error"))
            {
               // build error string
               string error;
               DynamicObjectIterator i =
                  e->getDetails()["error"].getIterator();
               while(i->hasNext())
               {
                  error.append(i->next()->getString());
                  if(i->hasNext())
                  {
                     error.push_back(',');
                  }
               }
               MO_CAT_ERROR(MO_HTTP_CAT,
                  "Connection error: ['%s','%s','%s']\n",
                  e->getMessage(), e->getType(), error.c_str());
            }
            else
            {
               MO_CAT_ERROR(MO_HTTP_CAT,
                  "Connection error: ['%s','%s']\n",
                  e->getMessage(), e->getType());
            }
         }
      }

      if(keepAlive && noerror)
      {
         // set keep-alive timeout (defaults to 5 minutes)
         hc.setReadTimeout(1000 * 60 * 5);

         // clear request and response header fields
         reqHeader->clearFields();
         resHeader->clearFields();
         resHeader->clearStatus();
      }
   }

   // clean up request and response
   delete request;
   delete response;

   // close connection
   hc.close();
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
         "Could not add http request servicer. Invalid domain format.",
         "monarch.net.http.InvalidDomainFormat");
      e->getDetails()["domain"] = domain;
      e->getDetails()["regex"] = regex.c_str();
      Exception::push(e);
   }

   return rval;
}

static int _countWildcards(const char* domain)
{
   int rval = 0;
   for(; *domain != '\0'; domain++)
   {
      if(*domain == '*')
      {
         rval++;
      }
   }
   return rval;
}

/**
 * Sort function that roughly sorts domains by how specific they are. The
 * fewer the wildcards, the "lesser" the value. If wildcard counts are the
 * same, then the shorter string is used.
 *
 * @param a ServiceDomain 1.
 * @param b ServiceDomain 2.
 *
 * @return true if a is less than b.
 */
bool HttpConnectionServicer::ServiceDomainSorter::operator()(
   const ServiceDomain* a, const ServiceDomain* b)
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

bool HttpConnectionServicer::addRequestServicer(
   HttpRequestServicer* s, bool secure, const char* domain)
{
   bool rval = false;

   // try to compile domain regex first
   PatternRef regex = _compileDomainRegex(domain);
   rval = !regex.isNull();
   if(rval)
   {
      const char* path = s->getPath();
      mDomainLock.lockExclusive();
      {
         // try to find exact domain
         ServiceDomain* sd = NULL;
         for(ServiceDomainList::iterator i = mDomains.begin();
             sd == NULL && i != mDomains.end(); i++)
         {
            if(strcmp((*i)->domain, domain) == 0)
            {
               sd = *i;
            }
         }

         // add a new domain if one wasn't found
         if(sd == NULL)
         {
            // insert new domain
            sd = new ServiceDomain;
            sd->domain = strdup(domain);
            sd->regex = regex;
            mDomains.push_back(sd);
            if(secure)
            {
               sd->secureMap[path] = s;
            }
            else
            {
               sd->nonSecureMap[path] = s;
            }

            // sort domains
            sort(mDomains.begin(), mDomains.end(), ServiceDomainSorter());
         }
         else
         {
            // see if a servicer already exists
            ServicerMap& sm = secure ? sd->secureMap : sd->nonSecureMap;
            ServicerMap::iterator si = sm.find(path);
            if(si == sm.end())
            {
               sm[path] = s;
            }
            else
            {
               // servicer already exists
               ExceptionRef e = new Exception(
                  "Could not add http request servicer. "
                  "Domain/path combination is already in use.",
                  "monarch.net.http.DuplicateEntry");
               e->getDetails()["domain"] = domain;
               e->getDetails()["path"] = path;
               Exception::set(e);
               rval = false;
            }
         }
      }
      mDomainLock.unlockExclusive();
   }

   return rval;
}

void HttpConnectionServicer::removeRequestServicer(
   HttpRequestServicer* s, bool secure, const char* domain)
{
   removeRequestServicer(s->getPath(), secure, domain);
}

HttpRequestServicer* HttpConnectionServicer::removeRequestServicer(
   const char* path, bool secure, const char* domain)
{
   HttpRequestServicer* rval = NULL;

   mDomainLock.lockExclusive();
   {
      // try to find exact domain
      for(ServiceDomainList::iterator i = mDomains.begin();
          rval == NULL && i != mDomains.end(); i++)
      {
         ServiceDomain* sd = *i;
         if(strcmp(sd->domain, domain) == 0)
         {
            // domain matched, look for servicer
            ServicerMap& sm = secure ? sd->secureMap : sd->nonSecureMap;
            ServicerMap::iterator si = sm.find(path);
            if(si != sm.end())
            {
               // get servicer and erase its list entry
               rval = si->second;
               sm.erase(si);

               // if domain is now empty, erase it
               if(sd->secureMap.empty() && sd->nonSecureMap.empty())
               {
                  mDomains.erase(i);
                  free(sd->domain);
                  delete sd;
               }
            }
         }
      }
   }
   mDomainLock.unlockExclusive();

   return rval;
}

HttpRequestServicer* HttpConnectionServicer::getRequestServicer(
   const char* path, bool secure, const char* domain)
{
   HttpRequestServicer* rval = NULL;

   mDomainLock.lockShared();
   {
      // try to find exact domain
      for(ServiceDomainList::iterator i = mDomains.begin();
          rval == NULL && i != mDomains.end(); i++)
      {
         ServiceDomain* sd = *i;
         if(strcmp(sd->domain, domain) == 0)
         {
            // domain matched, look for servicer
            ServicerMap& sm = secure ? sd->secureMap : sd->nonSecureMap;
            ServicerMap::iterator si = sm.find(path);
            if(si != sm.end())
            {
               rval = si->second;
            }
         }
      }
   }
   mDomainLock.unlockShared();

   return rval;
}

HttpRequestServicer* HttpConnectionServicer::findRequestServicer(
   std::string& host, char* path, bool secure)
{
   HttpRequestServicer* rval = NULL;

   // strip any port number from host
   size_t pos = host.find(':');
   if(pos != string::npos)
   {
      host.erase(pos);
   }

   // strip any query from path
   if(path != NULL)
   {
      char* end = strrchr(path, '?');
      if(end != NULL)
      {
         end[0] = 0;
      }
   }

   // get a copy of the path to reuse
   int len = strlen(path);
   char tmp[len + 1];
   strcpy(tmp, path);

   mDomainLock.lockShared();
   {
      // try to find regex-matching domain
      for(ServiceDomainList::iterator i = mDomains.begin();
          rval == NULL && i != mDomains.end(); i++)
      {
         ServiceDomain* sd = *i;
         if(sd->regex->match(host.c_str()))
         {
            // domain matched, look for servicer
            ServicerMap& sm = secure ? sd->secureMap : sd->nonSecureMap;

            // reset path
            strcpy(path, tmp);

            // try to find servicer for path
            ServicerMap::iterator si;
            while(rval == NULL && path != NULL)
            {
               si = sm.find(path);
               if(si != sm.end())
               {
                  // servicer found
                  rval = si->second;
               }
               else if(strlen(path) > 1)
               {
                  // try to find servicer at parent path
                  char* end = strrchr(path, '/');
                  if(end != NULL)
                  {
                     // if parent is root (end == path), set path to "/"
                     // if parent is not root, clear last slash
                     end[(end == path) ? 1 : 0] = 0;
                  }
                  else
                  {
                     // no path left to search
                     path = NULL;
                  }
               }
               else
               {
                  // no path left to search
                  path = NULL;
               }
            }
         }
      }
   }
   mDomainLock.unlockShared();

   return rval;
}
