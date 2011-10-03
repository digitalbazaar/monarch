/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/ws/ServiceChannel.h"

#include "monarch/logging/Logging.h"
#include "monarch/data/json/JsonWriter.h"

#include <cctype>
#include <algorithm>

using namespace std;
using namespace monarch::data::json;
using namespace monarch::http;
using namespace monarch::io;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::ws;

#define CONTENT_TYPE_ANY    "*/*"
#define CONTENT_TYPE_JSON   "application/json"
#define CONTENT_TYPE_JSONLD "application/ld+json"
#define CONTENT_TYPE_XML    "text/xml"
#define CONTENT_TYPE_FORM   "application/x-www-form-urlencoded"

ServiceChannel::ServiceChannel(const char* path) :
   mPath(strdup(path)),
   mBasePath(NULL),
   mInput(NULL),
   mOutput(NULL),
   mRequest(NULL),
   mResponse(NULL),
   mPathParams(NULL),
   mQueryVars(NULL),
   mArrayQueryVars(NULL),
   mContent(NULL),
   mHandlerInfo(NULL),
   mHandlerData(NULL),
   mAuthMethod(NULL),
   mContentReceived(false),
   mHasSent(NULL),
   mAutoContentEncode(true)
{
}

ServiceChannel::~ServiceChannel()
{
   free(mPath);
   free(mBasePath);
   free(mAuthMethod);

   // free method strings
   for(AuthDataMap::iterator i = mAuthDataMap.begin();
       i != mAuthDataMap.end(); ++i)
   {
      free((char*)i->first);
   }

   // free method strings
   for(AuthErrorMap::iterator i = mAuthErrorMap.begin();
       i != mAuthErrorMap.end(); ++i)
   {
      free((char*)i->first);
   }
}

void ServiceChannel::initialize()
{
   mInput = createMessage();
   mOutput = createMessage();
}

void ServiceChannel::cleanup()
{
   delete mInput;
   delete mOutput;

   // TODO: once void* handler data is implemented, call function to free
   // it here or will the handler take care of that?
}

Message* ServiceChannel::createMessage()
{
   return new Message();
}

void ServiceChannel::setHandlerInfo(DynamicObject& info)
{
   mHandlerInfo = info;
}

DynamicObject& ServiceChannel::getHandlerInfo()
{
   return mHandlerInfo;
}

void ServiceChannel::setAuthenticationMethod(
   const char* method, DynamicObject& data)
{
   if(method != NULL)
   {
      // only set first auth method once
      if(mAuthMethod == NULL)
      {
         mAuthMethod = strdup(method);
      }

      // add/replace auth data entry in map
      AuthDataMap::iterator i = mAuthDataMap.find(method);
      if(i != mAuthDataMap.end())
      {
         mAuthDataMap[method] = data;
      }
      else
      {
         mAuthDataMap.insert(make_pair(strdup(method), data));
      }
   }
}

void ServiceChannel::unsetAuthenticationMethod(const char* method)
{
   if(method != NULL)
   {
      // remove entry from map
      mAuthDataMap.erase(method);

      // set new "first" method, even if it wasn't technically "first", the
      // simple "first" API is typically used to avoid having to type the
      // authentication method for services that have only 1 method
      if(strcmp(mAuthMethod, method) == 0)
      {
         free(mAuthMethod);
         if(mAuthDataMap.size() == 0)
         {
            mAuthMethod = NULL;
         }
         else
         {
            mAuthMethod = strdup(mAuthDataMap.begin()->first);
         }
      }
   }
}

const char* ServiceChannel::getAuthenticationMethod()
{
   return mAuthMethod;
}

bool ServiceChannel::isAuthenticated()
{
   return mAuthMethod != NULL;
}

DynamicObject ServiceChannel::getAuthenticationData(const char* method)
{
   DynamicObject rval(NULL);

   // use first auth method set
   if(method == NULL)
   {
      method = mAuthMethod;
   }

   if(method != NULL)
   {
      AuthDataMap::iterator i = mAuthDataMap.find(method);
      if(i != mAuthDataMap.end())
      {
         rval = i->second;
      }
   }

   return rval;
}

void ServiceChannel::setAuthenticationException(
   const char* method, ExceptionRef& e)
{
   if(method != NULL)
   {
      // add/replace auth exception entry in map
      AuthErrorMap::iterator i = mAuthErrorMap.find(method);
      if(i != mAuthErrorMap.end())
      {
         mAuthErrorMap[method] = e;
      }
      else
      {
         mAuthErrorMap.insert(make_pair(strdup(method), e));
      }
   }
}

ExceptionRef ServiceChannel::getAuthenticationException(const char* method)
{
   ExceptionRef rval(NULL);

   if(method != NULL)
   {
      AuthErrorMap::iterator i = mAuthErrorMap.find(method);
      if(i != mAuthErrorMap.end())
      {
         rval = i->second;
      }
   }

   return rval;
}

void ServiceChannel::selectContentEncoding()
{
   // check user-agent (MSIE barfs on deflate because it assumes it is raw
   // DEFLATE, not zlib+DEFLATE which is what the HTTP spec calls for...
   // zlib adds a 2 byte header that it dies on)
   // Safari has the same issue.

   // FIXME: could alternatively turn raw deflate off/on instead of gzipping
   bool mustGzip = true;
   string userAgent;
   if(mRequest->getHeader()->getField("User-Agent", userAgent))
   {
      // convert user agent to lower-case to normalize comparisons
      transform(
         userAgent.begin(), userAgent.end(), userAgent.begin(), ::tolower);
      mustGzip =
         (strstr(userAgent.c_str(), "msie") != NULL) ||
         (strstr(userAgent.c_str(), "webkit") != NULL) ||
         (strstr(userAgent.c_str(), "konqueror") != NULL);
   }

   // use accept content-encoding
   string contentEncoding;
   if(mRequest->getHeader()->getField("Accept-Encoding", contentEncoding))
   {
      // gzip gets precendence because not everyone handles deflate properly
      if(!mustGzip && strstr(contentEncoding.c_str(), "deflate") != NULL)
      {
         mResponse->getHeader()->setField("Content-Encoding", "deflate");
      }
      else if(strstr(contentEncoding.c_str(), "gzip") != NULL)
      {
         mResponse->getHeader()->setField("Content-Encoding", "gzip");
      }
   }
}

void ServiceChannel::setAutoContentEncode(bool on)
{
   mAutoContentEncode = on;
}

bool ServiceChannel::receiveContent(OutputStream* os, bool close)
{
   // set content sink, receive content
   mInput->setContentSink(os, close);
   return mInput->receiveContent(mRequest);
}

bool ServiceChannel::receiveContent(DynamicObject& dyno)
{
   bool rval = true;

   // use cached content if already received
   if(mContentReceived)
   {
      dyno = mContent;
   }
   else
   {
      // use previously set content object if available
      if(!mContent.isNull())
      {
         dyno = mContent;
      }

      // check to see if there is content to receive
      if(mRequest->getHeader()->hasContent())
      {
         // set content object, receive content
         mInput->setDynamicObject(dyno);
         rval = mInput->receiveContent(mRequest);
      }
      else
      {
         // no content
         dyno.setNull();
      }

      // content now received (Note: dyno is not cloned here, so changes
      // will affect the cached received value)
      mContent = dyno;
      mContentReceived = true;
   }

   return rval;
}

bool ServiceChannel::sendNoContent()
{
   bool rval = true;

   if(!hasSent())
   {
      // set no content code if code not set
      if(mResponse->getHeader()->getStatusCode() == 0)
      {
         mResponse->getHeader()->setStatus(204, "No Content");
      }
      else if(mResponse->getHeader()->getStatusCode() != 204)
      {
         // ensure content-length is set to none
         mResponse->getHeader()->setField("Content-Length", 0);
      }

      // send
      mOutput->setContentSource(NULL);
      rval = mOutput->sendResponse(mResponse);
      if(rval)
      {
         setSent();
      }
   }

   return rval;
}

bool ServiceChannel::sendContent(InputStream* is)
{
   bool rval = true;

   if(!hasSent())
   {
      // set response code if not set
      if(mResponse->getHeader()->getStatusCode() == 0)
      {
         // send 200 OK
         mResponse->getHeader()->setStatus(200, "OK");
      }

      // set content source
      mOutput->setContentSource(is);

      // select content encoding if auto-mode and not set
      if(mAutoContentEncode &&
         !mResponse->getHeader()->hasField("Content-Encoding"))
      {
         selectContentEncoding();
      }

      // send
      rval = mOutput->sendResponse(mResponse);
      if(rval)
      {
         setSent();
      }
   }

   return rval;
}

static void _setDynoContentType(HttpRequest* request, HttpResponse* response)
{
   // use accept content-type if not already set
   string ct = response->getHeader()->getFieldValue("Content-Type");
   if(ct.length() == 0)
   {
      ct = request->getHeader()->getFieldValue("Accept");

      // prefer JSON
      if(ct.length() == 0 ||
         strstr(ct.c_str(), CONTENT_TYPE_ANY) != NULL ||
         strstr(ct.c_str(), CONTENT_TYPE_JSON) != NULL)
      {
         ct = CONTENT_TYPE_JSON;
      }
      else if(strstr(ct.c_str(), CONTENT_TYPE_JSONLD) != NULL)
      {
         // FIXME: support form param
         ct = CONTENT_TYPE_JSONLD;
      }
      else if(strstr(ct.c_str(), CONTENT_TYPE_XML) != NULL)
      {
         ct = CONTENT_TYPE_XML;
      }
      else if(strstr(ct.c_str(), CONTENT_TYPE_FORM) != NULL)
      {
         ct = CONTENT_TYPE_FORM;
      }
      else
      {
         // none supported, don't care, use json
         ct = CONTENT_TYPE_JSON;
      }
      response->getHeader()->setField("Content-Type", ct.c_str());
   }
}

bool ServiceChannel::sendContent(DynamicObject& dyno)
{
   bool rval = true;

   if(!hasSent())
   {
      // set response code if not set
      if(mResponse->getHeader()->getStatusCode() == 0)
      {
         // send 200 OK
         mResponse->getHeader()->setStatus(200, "OK");
      }

      // set dyno content-type if not already set
      _setDynoContentType(mRequest, mResponse);

      // set content object
      mOutput->setDynamicObject(dyno);

      // auto-select content-encoding if specified and not set
      if(mAutoContentEncode &&
         !mResponse->getHeader()->hasField("Content-Encoding"))
      {
         selectContentEncoding();
      }

      // send
      rval = mOutput->sendResponse(mResponse);
      if(rval)
      {
         setSent();
      }
   }

   return rval;
}

bool ServiceChannel::sendException(ExceptionRef& e, bool client)
{
   bool rval = true;

   if(!hasSent())
   {
      // set status code if necessary
      if(mResponse->getHeader()->getStatusCode() == 0)
      {
         if(e->getDetails()->hasMember("httpStatusCode"))
         {
            mResponse->getHeader()->setStatus(
               e->getDetails()["httpStatusCode"]);
         }
         else if(client)
         {
            // set 400 Bad Request
            mResponse->getHeader()->setStatus(400, "Bad Request");
         }
         else
         {
            // set 500 Internal Server Error
            mResponse->getHeader()->setStatus(500, "Internal Server Error");
         }
      }

      // set dyno content-type if not already set
      _setDynoContentType(mRequest, mResponse);

      // convert exception to dyno
      DynamicObject dyno = Exception::convertToDynamicObject(e);
      mOutput->setDynamicObject(dyno);

      // auto-select content-encoding if specified and not set
      if(mAutoContentEncode &&
         !mResponse->getHeader()->hasField("Content-Encoding"))
      {
         selectContentEncoding();
      }

      // log exception
      MO_CAT_DEBUG(MO_WS_CAT,
         "Exception while servicing path '%s':\nException %s",
         getPath(), JsonWriter::writeToString(dyno).c_str());

      // send
      rval = mOutput->sendResponse(mResponse);
      if(rval)
      {
         setSent();
      }
   }

   return rval;
}

inline const char* ServiceChannel::getPath()
{
   return mPath;
}

bool ServiceChannel::getPathParams(DynamicObject& params)
{
   bool rval;

   if(mPathParams.isNull())
   {
      // parse params
      Url url;
      rval = url.setRelativeUrl(mPath);
      if(rval)
      {
         mPathParams = DynamicObject();
         rval = url.getTokenizedPath(mPathParams, mBasePath);
      }
   }
   else
   {
      // check stored params (previously parsed)
      rval = (mPathParams->length() > 0);
   }

   // clone stored params
   params = mPathParams.clone();

   return rval;
}

bool ServiceChannel::getQuery(DynamicObject& vars, bool asArrays)
{
   bool rval;

   // choose which cache ivar to use
   DynamicObject& qvars = asArrays ? mArrayQueryVars : mQueryVars;

   if(qvars.isNull())
   {
      // parse query
      Url url(mPath);
      qvars = DynamicObject();
      rval = url.getQueryVariables(qvars, asArrays);
   }
   else
   {
      // check stored vars (previously parsed)
      rval = (qvars->length() > 0);
   }

   // clone stored vars
   vars = qvars.clone();

   return rval;
}

Message* ServiceChannel::getInput()
{
   return mInput;
}

Message* ServiceChannel::getOutput()
{
   return mOutput;
}

void ServiceChannel::setRequest(HttpRequest* request)
{
   mRequest = request;
}

HttpRequest* ServiceChannel::getRequest()
{
   return mRequest;
}

void ServiceChannel::setResponse(HttpResponse* response)
{
   mResponse = response;
}

HttpResponse* ServiceChannel::getResponse()
{
   return mResponse;
}

monarch::http::HttpConnection* ServiceChannel::getConnection()
{
   return mRequest->getConnection();
}

bool ServiceChannel::isSecure()
{
   return getRequest()->getConnection()->isSecure();
}

void ServiceChannel::setBasePath(const char* path)
{
   free(mBasePath);

   // append slash to passed path as needed
   int length = strlen(path);
   if(path[length - 1] != '/')
   {
      mBasePath = (char*)malloc(length + 2);
      strcpy(mBasePath, path);
      mBasePath[length] = '/';
      mBasePath[length + 1] = 0;
   }
   else
   {
      mBasePath = strdup(path);
   }
}

const char* ServiceChannel::getBasePath()
{
   return mBasePath;
}

bool ServiceChannel::hasSent()
{
   return mHasSent;
}

void ServiceChannel::setSent(bool sent)
{
   mHasSent = sent;
}

void ServiceChannel::setContentObject(DynamicObject& content)
{
   mContent = content;
}

bool ServiceChannel::getClientAddress(InternetAddress* address)
{
   return getConnection()->writeRemoteAddress(address);
}

Message::MethodType ServiceChannel::getRequestMethod()
{
   return Message::stringToMethod(getRequest()->getHeader()->getMethod());
}
