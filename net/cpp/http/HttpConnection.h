/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef HttpConnection_H
#define HttpConnection_H

#include "WebConnection.h"

#include <string>

namespace db
{
namespace net
{
namespace http
{

/**
 * An HttpConnection is a WebConnection that uses the HTTP protocol.
 * 
 * @author Dave Longley
 */
class HttpConnection : public db::net::WebConnection
{
public:
   /**
    * Creates a new HttpConnection that wraps the passed Connection.
    * 
    * @param c the Connection to wrap.
    * @param cleanup true to clean up the Connection when this HttpConnection
    *                is destructed, false to leave it alone. 
    */
   HttpConnection(Connection* c, bool cleanup);
   
   /**
    * Destructs this HttpConnection.
    */
   virtual ~HttpConnection();
   
   /**
    * Creates a new HttpRequest.
    * 
    * The caller of this method is responsible for freeing the created request.
    * 
    * @return the new HttpRequest.
    */
   virtual db::net::WebRequest* createRequest();
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
