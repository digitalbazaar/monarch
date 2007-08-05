/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpClient_H
#define db_net_http_HttpClient_H

#include "HttpConnection.h"

namespace db
{
namespace net
{
namespace http
{

/**
 * An HttpClient is a web client that uses the HTTP protocol.
 * 
 * @author Dave Longley
 */
class HttpClient
{
public:
   /**
    * Creates a new HttpClient.
    */
   HttpClient();
   
   /**
    * Destructs this HttpClient.
    */
   virtual ~HttpClient();
   
   /**
    * Creates a connection to the passed address.
    * 
    * The caller of this method is responsible for deleting the returned
    * connection. If an exception occurs, it can be retrieved via
    * Exception::getLast().
    * 
    * @param address the address to connect to.
    * @param timeout the timeout in seconds (0 for indefinite).
    * 
    * @return the HttpConnection to the address or NULL if an exception
    *         occurred.
    */
   virtual HttpConnection* connect(
      db::net::InternetAddress* address, unsigned int timeout = 30);
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
