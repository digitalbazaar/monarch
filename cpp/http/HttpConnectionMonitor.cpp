/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/HttpConnectionMonitor.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/logging/Logging.h"
#include "monarch/rt/Atomic.h"

#include <cstdio>

using namespace std;
using namespace monarch::io;
using namespace monarch::http;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;

// Add sufficient info for logging
// - info required for Apache "combined" type log:
//   - http://httpd.apache.org/docs/current/logs.html
// - also include extra information for stats and other analysis:
//   - http://blog.notdot.net/2011/06/Demystifying-the-App-Engine-request-logs
// Details are added where appropriate so as much information is available for
// event handlers.

// Many stats are also tracked on a per-request and overall level.

HttpConnectionMonitor::HttpConnectionMonitor() :
   mTotalTime(0),
   mTotalConnections(0),
   mActiveConnections(0),
   mTotalRequests(0),
   mActiveRequests(0),
   mTotalErrors(0),
   mTotalBytesRead(0),
   mTotalContentBytesRead(0),
   mTotalBytesWritten(0),
   mTotalContentBytesWritten(0),
   mTotalStatus1xx(0),
   mTotalStatus2xx(0),
   mTotalStatus3xx(0),
   mTotalStatus4xx(0),
   mTotalStatus5xx(0),
   mTotalStatusOther(0)
{
}

HttpConnectionMonitor::~HttpConnectionMonitor()
{
}

DynamicObject HttpConnectionMonitor::getStats()
{
   DynamicObject rval;

   rval["totalTime"] = Atomic::load(&mTotalTime);
   rval["totalConnections"] = Atomic::load(&mTotalConnections);
   rval["activeConnections"] = Atomic::load(&mActiveConnections);
   rval["totalRequests"] = Atomic::load(&mTotalRequests);
   rval["activeRequests"] = Atomic::load(&mActiveRequests);
   rval["totalErrors"] = Atomic::load(&mTotalErrors);
   rval["totalBytesRead"] = Atomic::load(&mTotalBytesRead);
   rval["totalContentBytesRead"] = Atomic::load(&mTotalContentBytesRead);
   rval["totalBytesWritten"] = Atomic::load(&mTotalBytesWritten);
   rval["totalContentBytesWritten"] = Atomic::load(&mTotalContentBytesWritten);
   rval["totalStatus1xx"] = Atomic::load(&mTotalStatus1xx);
   rval["totalStatus2xx"] = Atomic::load(&mTotalStatus2xx);
   rval["totalStatus3xx"] = Atomic::load(&mTotalStatus3xx);
   rval["totalStatus4xx"] = Atomic::load(&mTotalStatus4xx);
   rval["totalStatus5xx"] = Atomic::load(&mTotalStatus5xx);
   rval["totalStatusOther"] = Atomic::load(&mTotalStatusOther);

   return rval;
}

void HttpConnectionMonitor::beforeServicingConnection(
   HttpConnection* connection)
{
   MO_CAT_OBJECT_DEBUG_DETAIL(MO_HTTP_CAT, connection,
      "HttpConnection beforeServicingConnection");
   Atomic::incrementAndFetch(&mTotalConnections);
   Atomic::incrementAndFetch(&mActiveConnections);
}

void HttpConnectionMonitor::afterServicingConnection(
   HttpConnection* connection)
{
   MO_CAT_OBJECT_DEBUG_DETAIL(MO_HTTP_CAT, connection,
       "HttpConnection afterServicingConnection");
   Atomic::decrementAndFetch(&mActiveConnections);
}

void HttpConnectionMonitor::beforeRequest(HttpConnection* connection)
{
   MO_CAT_OBJECT_DEBUG_DETAIL(MO_HTTP_CAT, connection,
       "HttpConnection beforeRequest");
   Atomic::incrementAndFetch(&mTotalRequests);
   Atomic::incrementAndFetch(&mActiveRequests);
}

void HttpConnectionMonitor::afterRequest(HttpConnection* connection)
{
   MO_CAT_OBJECT_DEBUG_DETAIL(MO_HTTP_CAT, connection,
       "HttpConnection afterRequest");
   Atomic::decrementAndFetch(&mActiveRequests);

   HttpRequestState* state = connection->getRequestState();
   DynamicObject& d = state->getDetails();

   // client's IP address/port
   d["remoteAddress"] = connection->getRemoteAddress()->getAddress();
   d["remotePort"] = connection->getRemoteAddress()->getPort();

   // RFC1413 identity of the client (in practice, always '-')
   d["identity"] = "-";

   // Set default if userId not set in connection state.
   if(!d->hasMember("userId"))
   {
      d["userId"] = "-";
   }

   // timestamp of the request
   d["startTime"] = state->getTimer()->getStartTime();

   // wallclock milliseconds required to fulfill the request
   d["elapsedTime"] = state->getTimer()->getElapsedMilliseconds();

   // request and response full and content size in bytes
   d["bytesRead"] = connection->getBytesRead();
   d["contentBytesRead"] = connection->getContentBytesRead();
   d["bytesWritten"] = connection->getBytesWritten();
   d["contentBytesWritten"] = connection->getContentBytesWritten();

   // security mode
   // TODO: add other variations like "SSL+OAuth"?
   d["security"] = connection->isSecure() ? "SSL" : "none";

   // update stats
   Atomic::addAndFetch(
      &mTotalTime, d["elapsedTime"]->getUInt64());
   Atomic::addAndFetch(
      &mTotalBytesRead, d["bytesRead"]->getUInt64());
   Atomic::addAndFetch(
      &mTotalContentBytesRead, d["contentBytesRead"]->getUInt64());
   Atomic::addAndFetch(
      &mTotalBytesWritten, d["bytesWritten"]->getUInt64());
   Atomic::addAndFetch(
      &mTotalContentBytesWritten, d["contentBytesWritten"]->getUInt64());

   // debug
   //d["stats"] = getStats();
   //monarch::data::json::JsonWriter::writeToStdOut(d["stats"]);
}

void HttpConnectionMonitor::beforeServicingRequest(
   HttpConnection* connection,
   HttpRequest* request,
   HttpResponse* response)
{
   MO_CAT_OBJECT_DEBUG_DETAIL(MO_HTTP_CAT, connection,
       "HttpConnection beforeServicingRequest");
   connection->getRequestState()->getDetails()["isError"] = false;
}

void HttpConnectionMonitor::afterServicingRequest(
   HttpConnection* connection,
   HttpRequest* request,
   HttpResponse* response)
{
   MO_CAT_OBJECT_DEBUG_DETAIL(MO_HTTP_CAT, connection,
       "HttpConnection afterServicingRequest");

   HttpRequestState* state = connection->getRequestState();
   DynamicObject& d = state->getDetails();

   // HTTP method
   d["method"] = request->getHeader()->getMethod();

   // request path
   d["path"] = request->getHeader()->getPath();

   // HTTP version
   d["version"] = request->getHeader()->getVersion();

   // The status code returned by the server
   d["statusCode"] = response->getHeader()->getStatusCode();

   // update status code stats
   {
      int code = d["statusCode"];
      if(code < 100)
      {
         Atomic::incrementAndFetch(&mTotalStatusOther);
      }
      if(code < 200)
      {
         Atomic::incrementAndFetch(&mTotalStatus1xx);
      }
      else if(code < 300)
      {
         Atomic::incrementAndFetch(&mTotalStatus2xx);
      }
      else if(code < 400)
      {
         Atomic::incrementAndFetch(&mTotalStatus3xx);
      }
      else if(code < 500)
      {
         Atomic::incrementAndFetch(&mTotalStatus4xx);
      }
      else if(code < 600)
      {
         Atomic::incrementAndFetch(&mTotalStatus5xx);
      }
      else
      {
         Atomic::incrementAndFetch(&mTotalStatusOther);
      }
   }

   // referrer path
   {
      std::string field;
      bool hasField = request->getHeader()->getField("referer", field);
      d["referer"] = hasField ? field.c_str() : "-";
   }

   // The user-agent
   {
      std::string field;
      bool hasField = request->getHeader()->getField("user-agent", field);
      d["userAgent"] = hasField ? field.c_str() : "-";
   }

   // server hostname
   {
      // FIXME: may want to get this from something other than the request
      std::string field;
      bool hasField = request->getHeader()->getField("host", field);
      d["host"] = hasField ? field.c_str() : "-";
   }
}

void HttpConnectionMonitor::beforeRequestError(
   HttpConnection* connection,
   HttpRequest* request,
   HttpResponse* response)
{
   MO_CAT_OBJECT_DEBUG_DETAIL(MO_HTTP_CAT, connection,
       "HttpConnection beforeRequestError");
   Atomic::incrementAndFetch(&mTotalErrors);
   connection->getRequestState()->getDetails()["isError"] = true;
}

void HttpConnectionMonitor::afterRequestError(
   HttpConnection* connection,
   HttpRequest* request,
   HttpResponse* response,
   ExceptionRef& exception)
{
   MO_CAT_OBJECT_DEBUG_DETAIL(MO_HTTP_CAT, connection,
       "HttpConnection afterRequestError");
   // debug
   //monarch::data::json::JsonWriter::writeToStdOut(
   //   Exception::convertToDynamicObject(exception));
}
