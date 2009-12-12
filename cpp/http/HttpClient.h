/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_http_HttpClient_H
#define monarch_http_HttpClient_H

#include "monarch/http/HttpConnection.h"
#include "monarch/http/HttpRequest.h"
#include "monarch/http/HttpResponse.h"
#include "monarch/net/SslContext.h"
#include "monarch/net/SslSessionCache.h"
#include "monarch/net/Url.h"

namespace monarch
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
   monarch::net::SslContext* mSslContext;

   /**
    * Set to true if this client created its own ssl context.
    */
   bool mCleanupSslContext;

   /**
    * Store the previous SSL session.
    */
   monarch::net::SslSession mSslSession;

public:
   /**
    * Creates a new HttpClient.
    *
    * @param sc the SslContext to use for https connections, NULL to create
    *           one.
    */
   HttpClient(monarch::net::SslContext* sc = NULL);

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
   virtual bool connect(monarch::net::Url* url);

   /**
    * Gets the local address for this Connection. This address can be
    * up-cast to an InternetAddress or Internet6Address based on
    * the communication domain of the returned SocketAddress.
    *
    * @return the local address.
    */
   virtual monarch::net::SocketAddress* getLocalAddress();

   /**
    * Gets the remote address for this Connection. This address can be
    * up-cast to an InternetAddress or Internet6Address based on
    * the communication domain of the returned SocketAddress.
    *
    * @return the remote address.
    */
   virtual monarch::net::SocketAddress* getRemoteAddress();

   /**
    * Sends an HTTP GET request and receives the response header. This
    * method will not receive the response content. The caller of this
    * method *MUST NOT* free the memory associated with the returned
    * HttpResponse.
    *
    * If the passed headers variable is not NULL, then it should contain
    * a map with keys that are field names and values that are either arrays
    * or non-maps.
    *
    * @param url the url of the content to request.
    * @param headers any special headers to include in the request.
    * @param follow true to follow redirects (default), false not to.
    *
    * @return the HTTP response if one was received, NULL if not.
    */
   virtual HttpResponse* get(
      monarch::net::Url* url, monarch::rt::DynamicObject* headers = NULL,
      bool follow = true);

   /**
    * Sends an HTTP POST request and its content. The caller of this
    * method *MUST NOT* free the memory associated with the returned
    * HttpResponse.
    *
    * If the passed headers variable is not NULL, then it should contain
    * a map with keys that are field names and values that are either arrays
    * or non-maps.
    *
    * @param url the url to post to.
    * @param headers any special headers to include in the request.
    * @param is the InputStream to read the content to send from.
    * @param trailer used to store any trailer headers to send.
    * @param skipContinue true to skip 100 continue response.
    *
    * @return the HTTP response if one was received, NULL if not.
    */
   virtual HttpResponse* post(
      monarch::net::Url* url, monarch::rt::DynamicObject* headers,
      monarch::io::InputStream* is,
      HttpTrailer* trailer = NULL, bool skipContinue = true);

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
      monarch::io::OutputStream* os, HttpTrailer* trailer = NULL);

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
    * @param commonNames a list of special X.509 subject common names to allow
    *                    in addition to the url's host, a blank list indicates
    *                    that the url's host common name should be ignored and
    *                    any common name can be accepted, a NULL list adds
    *                    no special common names and only the url's host will
    *                    be used.
    * @param includeHost true to include the host in any special list of
    *                    common names, false not to.
    *
    * @return the HttpConnection to the url or NULL if an exception
    *         occurred.
    */
   static HttpConnection* createConnection(
      monarch::net::Url* url,
      monarch::net::SslContext* sslContext = NULL,
      monarch::net::SslSession* session = NULL,
      unsigned int timeout = 30,
      monarch::rt::DynamicObject* commonNames = NULL,
      bool includeHost = true);

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
    * @param commonName the X.509 subject common
    * @param timeout the timeout in seconds (0 for indefinite).
    * @param commonNames a list of special X.509 subject common names to allow
    *                    in addition to the url's host, a blank list indicates
    *                    that the url's host common name should be ignored and
    *                    any common name can be accepted, a NULL list adds
    *                    no special common names and only the url's host will
    *                    be used.
    * @param includeHost true to include the host in any special list of
    *                    common names, false not to.
    *
    * @return the HttpConnection to the url or NULL if an exception
    *         occurred.
    */
   static HttpConnection* createSslConnection(
      monarch::net::Url* url, monarch::net::SslContext& context,
      monarch::net::SslSessionCache& cache,
      unsigned int timeout = 30,
      monarch::rt::DynamicObject* commonNames = NULL,
      bool includeHost = true);

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
    * @param commonNames a list of special X.509 subject common names to allow
    *                    in addition to the url's host, a blank list indicates
    *                    that the url's host common name should be ignored and
    *                    any common name can be accepted, a NULL list adds
    *                    no special common names and only the url's host will
    *                    be used.
    * @param includeHost true to include the host in any special list of
    *                    common names, false not to.
    *
    * @return the HttpConnection to the address or NULL if an exception
    *         occurred.
    */
   static HttpConnection* createConnection(
      monarch::net::InternetAddress* address,
      monarch::net::SslContext* context = NULL,
      monarch::net::SslSession* session = NULL,
      unsigned int timeout = 30,
      monarch::rt::DynamicObject* commonNames = NULL,
      bool includeHost = true);

protected:
   /**
    * Sets the custom headers fields in the request header.
    *
    * @param h the HttpHeader to update.
    * @param headers the header fields to set.
    */
   virtual void setCustomHeaders(HttpHeader* h, monarch::rt::DynamicObject& headers);
};

} // end namespace http
} // end namespace monarch
#endif
