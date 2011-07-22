/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_http_HttpConnectionMonitor_h
#define monarch_http_HttpConnectionMonitor_h

#include "monarch/http/HttpConnection.h"
#include "monarch/http/HttpRequest.h"
#include "monarch/http/HttpResponse.h"

namespace monarch
{
namespace http
{

/**
 * An HttpConnectionMonitor monitors HttpConnectionServicers.
 *
 * It contains a number of hooks that will be called for various servicer
 * events.
 *
 * @author David I. Lehn
 */
class HttpConnectionMonitor
{
public:
   // stats
   uint64_t mTotalTime;
   uint64_t mTotalConnections;
   uint64_t mActiveConnections;
   uint64_t mTotalRequests;
   uint64_t mActiveRequests;
   uint64_t mTotalErrors;
   uint64_t mTotalBytesRead;
   uint64_t mTotalContentBytesRead;
   uint64_t mTotalBytesWritten;
   uint64_t mTotalContentBytesWritten;
   // status code results
   uint64_t mTotalStatus1xx;
   uint64_t mTotalStatus2xx;
   uint64_t mTotalStatus3xx;
   uint64_t mTotalStatus4xx;
   uint64_t mTotalStatus5xx;
   uint64_t mTotalStatusOther;

public:
   /**
    * Creates a new HttpConnectionMonitor.
    */
   HttpConnectionMonitor();

   /**
    * Destructs this HttpConnectionMonitor.
    */
   virtual ~HttpConnectionMonitor();

   /**
    * Get a DynamicObject with connection monitor statistics that include all
    * connections and requests serviced or in progress.
    *
    * Available statistics:
    *    totalTime
    *    totalConnections
    *    activeConnections
    *    totalRequests
    *    activeRequests
    *    totalErrors
    *    totalBytesRead
    *    totalContentBytesRead
    *    totalBytesWritten
    *    totalContentBytesWritten
    *    totalStatus1xx
    *    totalStatus2xx
    *    totalStatus3xx
    *    totalStatus4xx
    *    totalStatus5xx
    *    totalStatusOther
    */
   virtual monarch::rt::DynamicObject getStats();

   /**
    * Called before a connection begins servicing requests.
    *
    * @param connection the HttpConnection that is being monitored.
    */
   virtual void beforeServicingConnection(HttpConnection* connection);

   /**
    * Called before a connection finishes servicing requests.
    *
    * @param connection the HttpConnection that is being monitored.
    */
   virtual void afterServicingConnection(HttpConnection* connection);

   /**
    * Called before a request is received.
    *
    * @param connection the HttpConnection that is being monitored.
    * @param request the HttpRequest that was serviced.
    * @param response the HttpResponse for the request.
    */
   virtual void beforeRequest(HttpConnection* connection);

   /**
    * Called after a successful request or request error.
    *
    * @param connection the HttpConnection that is being monitored.
    */
   virtual void afterRequest(HttpConnection* connection);

   /**
    * Called before a request is serviced.
    *
    * @param connection the HttpConnection that is being monitored.
    * @param request the HttpRequest that was serviced.
    * @param response the HttpResponse for the request.
    */
   virtual void beforeServicingRequest(
      HttpConnection* connection,
      HttpRequest* request,
      HttpResponse* response);

   /**
    * Called after a request was serviced.
    *
    * @param connection the HttpConnection that is being monitored.
    * @param request the HttpRequest that was serviced.
    * @param response the HttpResponse for the request.
    */
   virtual void afterServicingRequest(
      HttpConnection* connection,
      HttpRequest* request,
      HttpResponse* response);

   /**
    * Called before a request error is serviced.
    *
    * @param connection the HttpConnection that is being monitored.
    * @param request the HttpRequest that was serviced.
    * @param response the HttpResponse for the request.
    */
   virtual void beforeRequestError(
      HttpConnection* connection,
      HttpRequest* request,
      HttpResponse* response);

   /**
    * Called after a request has been serviced.
    *
    * @param connection the HttpConnection that is being monitored.
    * @param request the HttpRequest that was serviced.
    * @param response the HttpResponse for the request.
    * @param exception the Exception associated with this error.
    */
   virtual void afterRequestError(
      HttpConnection* connection,
      HttpRequest* request,
      HttpResponse* response,
      monarch::rt::ExceptionRef& exception);
};

// type definition for a reference counted HttpConnectionMonitor
typedef monarch::rt::Collectable<HttpConnectionMonitor>
   HttpConnectionMonitorRef;

} // end namespace http
} // end namespace monarch
#endif
