/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_HttpConnectionServicer_H
#define db_net_HttpConnectionServicer_H

#include "ConnectionServicer.h"
#include "HttpConnection.h"

namespace db
{
namespace net
{
namespace http
{

/**
 * A HttpConnectionServicer services HttpConnections. It maintains a list
 * of HttpRequestServicers that service HttpRequests received over the
 * HttpConnections. One HttpRequestServicer can be assigned per available
 * request path.
 * 
 * @author Dave Longley
 */
class HttpConnectionServicer : public db::net::ConnectionServicer
{
public:
   /**
    * Creates a new HttpConnectionServicer.
    */
   HttpConnectionServicer();
   
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
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
