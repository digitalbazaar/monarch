/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_Channel_H
#define monarch_ws_Channel_H

#include "monarch/io/InputStream.h"
#include "monarch/io/OutputStream.h"
#include "monarch/net/Url.h"
#include "monarch/http/HttpConnection.h"
#include "monarch/http/HttpRequest.h"
#include "monarch/http/HttpResponse.h"
#include "monarch/rt/DynamicObject.h"

namespace monarch
{
namespace ws
{

/**
 * A Channel is a helper class that is used on top of an HttpConnection to
 * transmit or receive messages over HTTP. It can automatically convert HTTP
 * entity bodies to or from monarch DynamicObjects for the following MIME
 * Content-Types:
 *
 * application/json
 * text/xml
 * application/x-www-form-urlencoded
 *
 * If the associated message is not an object, the channel can also be used
 * to transmit or receive streams of data over HTTP.
 *
 * @author Dave Longley
 */
class Channel
{
public:
   /**
    * The different acceptable content-types for DynamicObjects.
    */
   enum ContentType
   {
      Invalid, Json, Xml, Form
   };

protected:
   /**
    * An input stream to read content will be sent.
    */
   monarch::io::InputStream* mContentSource;

   /**
    * An output stream to write receive content to.
    */
   monarch::io::OutputStream* mContentSink;

   /**
    * Set to true if the associated content sink should be closed
    * once it has been written to, false if not.
    */
   bool mCloseSink;

   /**
    * A DynamicObject to be sent/received.
    */
   monarch::rt::DynamicObject mDynamicObject;

   /**
    * Any extra http headers to include in an outgoing message.
    */
   monarch::rt::DynamicObject mCustomHeaders;

   /**
    * An HttpTrailer used in communication.
    */
   monarch::http::HttpTrailerRef mTrailer;

public:
   /**
    * Creates a new Channel.
    */
   Channel();

   /**
    * Destructs this Channel.
    */
   virtual ~Channel();

   /**
    * A helper function that automatically sets the path, version, user-agent,
    * and host for an http request.
    *
    * @param url the Url the request will be made to.
    * @param header the HttpRequestHeader to update.
    */
   virtual void setupRequestHeader(
      monarch::net::Url* url, monarch::http::HttpRequestHeader* header);

   /**
    * Adds any previously set custom headers and default transfer-encoding
    * based on the presence of content to be sent.
    *
    * @param header the header to update.
    */
   virtual void addCustomHeaders(monarch::http::HttpHeader* header);

   /**
    * Sends the header for a message over the passed connection. This method
    * can be used to manually send an http header which is to be followed
    * by a manually sent content-body.
    *
    * The output stream for writing the body will be set to the passed
    * OutputStreamRef parameter and it must be closed once the body has been
    * written to it.
    *
    * Any http trailer that will be sent when the stream is closed will be
    * set to the passed HttpTrailerRef parameter. Additional fields can be set
    * on the trailer to be sent after the body output stream is closed.
    *
    * @param hc the connection to send the message over.
    * @param header the header to use with the message.
    * @param os the body OutputStreamRef to be set.
    * @param trailer the HttpTrailerRef to be set or used.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool sendHeader(
      monarch::http::HttpConnection* hc, monarch::http::HttpHeader* header,
      monarch::io::OutputStreamRef& os,
      monarch::http::HttpTrailerRef& trailer);

   /**
    * Sends a message in its entirety (header and content) to the passed
    * url using the passed HttpRequest and pre-set content source input stream
    * or DynamicObject.
    *
    * The request header's method, host, accept, and other basic fields
    * will be automatically set.
    *
    * @param url the url to send the message to.
    * @param request the request to send the message with.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool send(
      monarch::net::Url* url,
      monarch::http::HttpRequest* request);

   /**
    * Sends a message in its entirety (header and content) using
    * the passed HttpResponse and pre-set content source input stream or
    * DynamicObject.
    *
    * @param response the response to send the message with.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool send(monarch::http::HttpResponse* response);

   /**
    * Receives the content of a message using the passed request and writes
    * it to the pre-set content sink output stream or DynamicObject. It is
    * assumed that the header for the passed request has already been received.
    *
    * @param request the request to receive the message content with.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool receiveContent(monarch::http::HttpRequest* request);

   /**
    * Receives the content of a message using the passed response and writes
    * it to the pre-set content sink output stream or DynamicObject. It is
    * assumed that the header for the passed response has already been received.
    *
    * @param response the response to receive the message content with.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool receiveContent(monarch::http::HttpResponse* response);

   /**
    * Gets a stream to manually receive the content of a message after the
    * header has already been received.
    *
    * It is safe to close the returned input stream (this will not shutdown
    * the underlying connection input) or to leave it open.
    *
    * @param request the http request.
    * @param is the InputStream to be set to the content stream to read.
    * @param trailer the HttpTrailer to be set to the trailers following
    *           the body.
    */
   virtual void getContentReceiveStream(
      monarch::http::HttpRequest* request,
      monarch::io::InputStreamRef& is, monarch::http::HttpTrailerRef& trailer);

   /**
    * Gets a stream to manually receive the content of a message after the
    * header has already been received.
    *
    * It is safe to close the returned input stream (this will not shutdown
    * the underlying connection input) or to leave it open.
    *
    * @param response the http response.
    * @param is the InputStream to be set to the content stream to read.
    * @param trailer the HttpTrailer to be set to the trailers following
    *           the body.
    */
   virtual void getContentReceiveStream(
      monarch::http::HttpResponse* response,
      monarch::io::InputStreamRef& is, monarch::http::HttpTrailerRef& trailer);

   /**
    * Gets the custom headers object. Any headers in this object will be added
    * to an outgoing header. The object is a map, where values can either
    * be arrays of non-arrays and non-maps, or where values are non-maps.
    *
    * @return the custom headers object for modification.
    */
   virtual monarch::rt::DynamicObject& getCustomHeaders();

   /**
    * Sets the content source to use with a message.
    *
    * @param is the InputStream with the content to send with a message.
    */
   virtual void setContentSource(monarch::io::InputStream* is);

   /**
    * Gets the content source to use with a message.
    *
    * @return the InputStream with the content to send with a message.
    */
   virtual monarch::io::InputStream* getContentSource();

   /**
    * Sets the content sink to use with a message.
    *
    * @param os the OutputStream to write the received message content to.
    * @param close close the OutputStream when done.
    */
   virtual void setContentSink(monarch::io::OutputStream* os, bool close);

   /**
    * Gets the content sink to use with a message.
    *
    * @return the OutputStream to write the received message content to.
    */
   virtual monarch::io::OutputStream* getContentSink();

   /**
    * Sets the DynamicObject to use when sending or receiving over this
    * Channel.
    *
    * @param dyno the DynamicObject to use.
    */
   virtual void setDynamicObject(monarch::rt::DynamicObject& dyno);

   /**
    * Gets the DynamicObject sent or received over this Channel.
    *
    * @return the DynamicObject associated with this Channel.
    */
   virtual monarch::rt::DynamicObject& getDynamicObject();

   /**
    * Gets the HttpTrailer sent/received during communication. The returned
    * trailer can also be set to a class instance that extends HttpTrailer.
    *
    * @return the HttpTrailer used in communication.
    */
   virtual monarch::http::HttpTrailerRef& getTrailer();

protected:
   /**
    * Checks the content-type specified in the given header to ensure that
    * it is a supported type for the transfer of a DynamicObject.
    *
    * @param header the HttpHeader to check.
    * @param type the ContentType detected.
    *
    * @return true if the content-type was appropriate, false if an Exception
    *         occurred.
    */
   virtual bool validateContentType(
      monarch::http::HttpHeader* header, ContentType& type);

   /**
    * Sends a message in its entirety (header and content) over the passed
    * connection, using the passed header and content source input stream.
    *
    * @param hc the connection to send the message over.
    * @param header the header to use with the message.
    * @param is the InputStream with content to read and send.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool sendHeaderAndStream(
      monarch::http::HttpConnection* hc,
      monarch::http::HttpHeader* header,
      monarch::io::InputStream* is);

   /**
    * Sends a message in its entirety (header and content) over the passed
    * connection, using the passed header and DynamicObject content.
    *
    * @param hc the connection to send the message over.
    * @param header the header to use with the message.
    * @param dyno the DynamicObject with content to send.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool sendHeaderAndObject(
      monarch::http::HttpConnection* hc, monarch::http::HttpHeader* header,
      monarch::rt::DynamicObject& dyno);

   /**
    * Receives the content of a message, if there is any, after the header
    * has already been received, and writes it to the passed OutputStream
    * content sink. The sink will not be closed by this method.
    *
    * @param hc the connection overwhich to receive the content.
    * @param header the previously received header.
    * @param os the output stream to write the received content to.
    *
    * @return true if successful, false if an Exception occurred or was
    *         received.
    */
   virtual bool receiveContentStream(
      monarch::http::HttpConnection* hc,
      monarch::http::HttpHeader* header, monarch::io::OutputStream* os);

   /**
    * Receives the content of a message, after the header has already been
    * received, and writes it to the passed DynamicObject.
    *
    * @param hc the connection overwhich to receive the content.
    * @param header the previously received header.
    * @param dyno the DynamicObject to write the received content to.
    *
    * @return true if successful, false if an Exception occurred or was
    *         received.
    */
   virtual bool receiveContentObject(
      monarch::http::HttpConnection* hc, monarch::http::HttpHeader* header,
      monarch::rt::DynamicObject& dyno);

   /**
    * Gets a stream to manually receive the content of a message after the
    * header has already been received.
    *
    * Closing the returned input stream will not shut down the connection's
    * input.
    *
    * @param hc the connection overwhich to receive the content.
    * @param header the previously received header.
    * @param is the InputStream to be set to the content stream to read.
    * @param trailer the HttpTrailer to be set or populated with the trailers
    *           following the body.
    */
   virtual void getContentReceiveStream(
      monarch::http::HttpConnection* hc, monarch::http::HttpHeader* header,
      monarch::io::InputStreamRef& is, monarch::http::HttpTrailerRef& trailer);
};

} // end namespace ws
} // end namespace monarch

#endif
