/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_Message_H
#define monarch_ws_Message_H

#include "monarch/io/InputStream.h"
#include "monarch/io/OutputStream.h"
#include "monarch/http/HttpConnection.h"
#include "monarch/http/HttpRequest.h"
#include "monarch/http/HttpResponse.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/util/Url.h"

namespace monarch
{
namespace ws
{

/**
 * A Message is a helper class for passing messages over HTTP. It is a
 * container for an object or stream that is to be transmitted or received. It
 * can automatically convert HTTP entity bodies to or from DynamicObjects
 * for the following MIME Content-Types:
 *
 * application/json
 * text/xml
 * application/x-www-form-urlencoded
 *
 * If the message is not serializable to a DynamicObject then an appropriate
 * custom stream can be used to transmit or receive the message.
 *
 * @author Dave Longley
 */
class Message
{
public:
   /**
    * HTTP methods.
    */
   enum MethodType
   {
      Undefined, Get, Put, Post, Delete, Head, Options, Trace, Connect
   };

   /**
    * The different acceptable content-types for DynamicObjects.
    */
   enum ContentType
   {
      Unknown, Json, JsonLd, Xml, Form
   };

protected:
   /**
    * An input stream to read content that will be sent.
    */
   monarch::io::InputStream* mContentSource;

   /**
    * An output stream to write received content to.
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

   /**
    * Options for receiving data.
    */
   monarch::rt::DynamicObject mOptions;

public:
   /**
    * Creates a new Message.
    */
   Message();

   /**
    * Destructs this Message.
    */
   virtual ~Message();

   /**
    * A helper function that automatically sets the path, version, user-agent,
    * and host for an http request. This method is not called automatically
    * and should be called, if desired, before sending a message using an
    * HttpRequest.
    *
    * @param url the Url the request will be made to.
    * @param header the HttpRequestHeader to update.
    */
   virtual void initializeRequestHeader(
      monarch::util::Url* url, monarch::http::HttpRequestHeader* header);

   /**
    * Adds any previously set custom headers and default transfer-encoding
    * based on the presence of content to be sent. Extending classes may
    * override this method to add their own headers.
    *
    * This method is automatically called before a message header is sent.
    *
    * @param header the header to update.
    */
   virtual void addCustomHeaders(monarch::http::HttpHeader* header);

   /**
    * Sends only the header for a message over the passed connection. This
    * method can be used to manually send an http header which is to be
    * followed by a manually sent content-body.
    *
    * The output stream for writing the body will be set to the passed
    * OutputStreamRef parameter and it must be closed once the body has been
    * written to it.
    *
    * If an HTTP trailer is set on the message then it will be sent when the
    * returned output stream is finished or closed. By default, no HTTP trailer
    * is set. The HTTP trailer must be set before sending the header, however,
    * additional fields can be set on the trailer before the output stream
    * is closed. Extending classes may set their own custom trailer.
    *
    * @param hc the connection to send the message over.
    * @param header the header to use with the message.
    * @param os the body OutputStreamRef to be set.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool sendHeader(
      monarch::http::HttpConnection* hc,
      monarch::http::HttpHeader* header,
      monarch::io::OutputStreamRef& os);

   /**
    * Convenience method for sending an HttpRequest header.
    *
    * See sendHeader().
    *
    * @param request the HttpRequest.
    * @param os the body OutputStreamRef to be set.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool sendRequestHeader(
      monarch::http::HttpRequest* request,
      monarch::io::OutputStreamRef& os);

   /**
    * Convenience method for sending an HttpResponse header.
    *
    * See sendHeader().
    *
    * @param response the HttpResponse.
    * @param os the body OutputStreamRef to be set.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool sendResponseHeader(
      monarch::http::HttpResponse* response,
      monarch::io::OutputStreamRef& os);

   /**
    * Sends a message in its entirety (header and content) over the given
    * HttpConnection using the given header. A content source input stream or
    * a DynamicObject must be set in order for content to be transmitted after
    * the header of the message. Otherwise, no content will be sent.
    *
    * Any custom headers will be added to the passed header and if content is
    * to be transmitted and "chunked" transfer-encoding can be used then any
    * supported compression methods (ie: deflate, gzip) will be performed
    * automatically. If "chunked" transfer-encoding is not available, then
    * compression must be performed manually.
    *
    * @param url the url to send the message to.
    * @param request the request to send the message with.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool send(
      monarch::http::HttpConnection* hc,
      monarch::http::HttpHeader* header);

   /**
    * A convenience method for sending a request message in its entirety.
    *
    * See send().
    *
    * @param request the HttpRequest.
    */
   virtual bool sendRequest(monarch::http::HttpRequest* request);

   /**
    * A convenience method for sending a response message in its entirety.
    *
    * See send().
    *
    * @param responsethe HttpResponse.
    */
   virtual bool sendResponse(monarch::http::HttpResponse* response);

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
    * assumed that the header for the passed response has already been
    * received. If the status code for the response indicates that there was
    * an error, then an attempt will be made to receive the content as a
    * DynamicObject (regardless of the content sink setting) and convert it
    * to an Exception. If an Exception can be read from the message, it will
    * be set on the current thread and the return value of this method will be
    * set to false.
    *
    * @param response the response to receive the message content with.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool receiveContent(monarch::http::HttpResponse* response);

   /**
    * Receives the content of a message using the passed connection and
    * header and writes it to the pre-set content sink output stream or
    * DynamicObject. If the header is an HttpResponseHeader with an error
    * status code, no automatic conversion of a DynamicObject to an Exception
    * will be performed.
    *
    * @param hc the connection to receive the message over.
    * @param header the header for the message.
    *
    * @return true if successful, false if an Exception occurred.
    */
   virtual bool receiveContent(
      monarch::http::HttpConnection* hc, monarch::http::HttpHeader* header);

   /**
    * Gets a stream to manually receive the content of a message after the
    * header has already been received.
    *
    * It is safe to close the returned input stream (this will not shutdown
    * the underlying connection input) or to leave it open.
    *
    * If a trailer is set on this message then it will be populated with any
    * trailers received after the input stream is read. Extending classes may
    * set their own custom trailer.
    *
    * If a supported content-encoding is set (ie: deflate, gzip) then
    * content-decoding will be performed automatically as the data is read
    * from the input stream.
    *
    * @param hc the http connection.
    * @param header the header for the message.
    * @param is the InputStream to be set to the content stream to read.
    */
   virtual void getContentInputStream(
      monarch::http::HttpConnection* hc,
      monarch::http::HttpHeader* header,
      monarch::io::InputStreamRef& is);

   /**
    * Gets the custom headers object. Any headers in this object will be added
    * to an outgoing header. The object is a map, where values can either
    * be arrays of non-arrays and non-maps, or where values are non-maps.
    *
    * @return the custom headers object for modification.
    */
   virtual monarch::rt::DynamicObject& getCustomHeaders();

   /**
    * Sets the content source to use with a message. This will also set
    * the DynamicObject to null indicating the message is not a DynamicObject.
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
    * Sets the content sink to use with a message. This will also set
    * the DynamicObject to null indicating the message is not a DynamicObject.
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
    * Sets the DynamicObject to use when sending or receiving. This will also
    * set the ContentSource and ContentSink to null indicating the message
    * will not be sent or received using custom streams.
    *
    * @param dyno the DynamicObject to use.
    */
   virtual void setDynamicObject(monarch::rt::DynamicObject& dyno);

   /**
    * Gets the DynamicObject sent or received.
    *
    * @return the DynamicObject associated with this Message.
    */
   virtual monarch::rt::DynamicObject& getDynamicObject();

   /**
    * Sets the HttpTrailer sent/received during communication.
    *
    * @param trailer the HttpTrailer used in communication.
    */
   virtual void setTrailer(monarch::http::HttpTrailerRef& trailer);

   /**
    * Gets the HttpTrailer sent/received during communication.
    *
    * @return the HttpTrailer used in communication.
    */
   virtual monarch::http::HttpTrailerRef& getTrailer();

   /**
    * Gets the options object. Allows special options to be set for
    * receiving data, etc.
    *
    * @return the options object.
    */
   virtual monarch::rt::DynamicObject& getOptions();

   /**
    * Gets the Content-Type for the given header.
    *
    * @param header the HttpHeader to check.
    *
    * @return the detected Content-Type of the header.
    */
   static ContentType getContentType(monarch::http::HttpHeader* header);

   /**
    * Conversion from string to MethodType.
    *
    * @param str the string to convert.
    *
    * @return the Type or Undefined.
    */
   static MethodType stringToMethod(const char* str);

   /**
    * Conversion from MethodType to string.
    *
    * @param method the MethodType to convert.
    *
    * @return the string or NULL.
    */
   static const char* methodToString(MethodType method);

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
    * @param hc the connection over which to receive the content.
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
    * @param hc the connection over which to receive the content.
    * @param header the previously received header.
    * @param dyno the DynamicObject to write the received content to.
    *
    * @return true if successful, false if an Exception occurred or was
    *         received.
    */
   virtual bool receiveContentObject(
      monarch::http::HttpConnection* hc, monarch::http::HttpHeader* header,
      monarch::rt::DynamicObject& dyno);
};

} // end namespace ws
} // end namespace monarch

#endif
