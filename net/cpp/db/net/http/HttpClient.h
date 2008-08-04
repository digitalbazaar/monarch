/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_net_http_HttpClient_H
#define db_net_http_HttpClient_H

#include "db/net/http/HttpConnection.h"
#include "db/net/http/HttpRequest.h"
#include "db/net/http/HttpResponse.h"
#include "db/net/SslContext.h"
#include "db/net/SslSessionCache.h"
#include "db/net/Url.h"

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
protected:
   /**
    * The current connection for this client.
    */
   HttpConnection* mConnection;
   
   /**
    * The current request for this client.
    */
   HttpRequest* mRequest;
   
   /**
    * The current response for this client.
    */
   HttpResponse* mResponse;
   
   /**
    * An SSL context for handling SSL connections.
    */
   db::net::SslContext* mSslContext;
   
   /**
    * Store the previous SSL session.
    */
   SslSession mSslSession;
   
   /**
    * Sets the headers fields in the request header.
    * 
    * @param h the HttpHeader to update.
    * @param headers the header fields to set.
    */
   virtual void setHeaders(HttpHeader* h, const char** headers);
   
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
    * Connects this client to the passed url if it isn't already connected. If
    * this client is already connected, this method will not check to make
    * sure it is connected to the passed url, it will just return true.
    * 
    * A call to disconnect should be made if the client wishes to switch
    * urls.
    * 
    * @param url the url to connect to.
    * 
    * @return true if this client is connected, false if not.
    */
   virtual bool connect(db::net::Url* url);
   
   /**
    * Sends an HTTP GET request and receives the response header. This
    * method will not receive the response content. The caller of this
    * method must not free the memory associated with the returned
    * HttpResponse.
    * 
    * If the passed headers variable is not NULL, then it should contain
    * an array of null-terminated header field "name: value" pairs. The last
    * entry in the array must be NULL.
    * 
    * Exception::getLast() may be checked if no response is received, but
    * it should be cleared prior to this method with Exception::clearLast();
    * 
    * @param url the url of the content to request.
    * @param headers any special headers to include in the request.
    * 
    * @return the HTTP response if one was received, NULL if not.
    */
   virtual HttpResponse* get(db::net::Url* url, const char** headers = NULL);
   
   /**
    * Sends an HTTP POST request and its content.
    * 
    * If the passed headers variable is not NULL, then it should contain
    * an array of null-terminated header field "name: value" pairs. The last
    * entry in the array must be NULL.
    * 
    * Exception::getLast() may be checked if no response is received, but
    * it should be cleared prior to this method with Exception::clearLast().
    * 
    * @param url the url to post to.
    * @param headers any special headers to include in the request.
    * @param is the InputStream to read the content to send from.
    * @param trailer used to store any trailer headers to send.
    * 
    * @return the HTTP response if one was received, NULL if not.
    */
   virtual HttpResponse* post(
      db::net::Url* url, const char** headers, db::io::InputStream* is,
      HttpTrailer* trailer = NULL);
   
   /**
    * Receives the content previously requested by get() or post() and
    * writes it to the passed output stream.
    * 
    * @param os the OutputStream to write the content to.
    * @param trailer used to store any trailer headers.
    * 
    * @return false if an IO exception occurred, true if not.
    */
   virtual bool receiveContent(
      db::io::OutputStream* os, HttpTrailer* trailer = NULL);
   
   /**
    * Disconnects this client, if it is connected.
    */
   virtual void disconnect();
   
   /**
    * Creates a connection to the passed url.
    * 
    * The caller of this method is responsible for deleting the returned
    * connection. If an exception occurs, it can be retrieved via
    * Exception::getLast().
    * 
    * @param url the url to connect to.
    * @param context an SslContext to use ssl, NULL not to.
    * @param session an SSL session to reuse, if any.
    * @param timeout the timeout in seconds (0 for indefinite).
    * 
    * @return the HttpConnection to the url or NULL if an exception
    *         occurred.
    */
   static HttpConnection* createConnection(
      Url* url,
      db::net::SslContext* sslContext = NULL,
      db::net::SslSession* session = NULL,
      unsigned int timeout = 30);
   
   /**
    * Creates an SSL connection to the passed url. This is the preferred
    * method for establishing an SSL connection because an SSL session cache
    * is used.
    * 
    * The caller of this method is responsible for deleting the returned
    * connection. If an exception occurs, it can be retrieved via
    * Exception::getLast().
    * 
    * @param url the url to connect to.
    * @param context an SslContext to use.
    * @param cache the SSL session cache to use.
    * @param timeout the timeout in seconds (0 for indefinite).
    * 
    * @return the HttpConnection to the url or NULL if an exception
    *         occurred.
    */
   static HttpConnection* createSslConnection(
      Url* url, db::net::SslContext& context,
      db::net::SslSessionCache& cache,
      unsigned int timeout = 30);
   
   /**
    * Creates a connection to the passed address.
    * 
    * The caller of this method is responsible for deleting the returned
    * connection. If an exception occurs, it can be retrieved via
    * Exception::getLast().
    * 
    * @param address the address to connect to.
    * @param context an SslContext to use ssl, NULL not to.
    * @param session an SSL session to reuse, if any.
    * @param timeout the timeout in seconds (0 for indefinite).
    * 
    * @return the HttpConnection to the address or NULL if an exception
    *         occurred.
    */
   static HttpConnection* createConnection(
      db::net::InternetAddress* address,
      db::net::SslContext* context = NULL,
      db::net::SslSession* session = NULL,
      unsigned int timeout = 30);
};

} // end namespace http
} // end namespace net
} // end namespace db
#endif
