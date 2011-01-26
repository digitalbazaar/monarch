/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_ServiceChannel_H
#define monarch_ws_ServiceChannel_H

#include "monarch/ws/Message.h"

namespace monarch
{
namespace ws
{

/**
 * A ServiceChannel is the channel used by a WebService to communicate with
 * a client. It contains a Message for receiving data from the client,
 * positioned after having read the client's request but before reading an
 * optional entity body, and a Message for sending data to the client.
 *
 * The Message objects are provided as a convenience interface for
 * communication. Direct access to the HttpRequest, HttpResponse, and
 * underlying HttpConnection objects is available if needed.
 *
 * @author Dave Longley
 */
class ServiceChannel
{
protected:
   /**
    * The normalized path received from the client.
    */
   char* mPath;

   /**
    * Stores the base path (excluding parameters) from the PathHandler that
    * created this ServiceChannel.
    */
   char* mBasePath;

   /**
    * The "in" Message from the client to the service.
    */
   Message* mInput;

   /**
    * The "out" Message from the service to the client.
    */
   Message* mOutput;

   /**
    * The HttpRequest used to receive from the client.
    */
   monarch::http::HttpRequest* mRequest;

   /**
    * The HttpResponse to use to send from the server.
    */
   monarch::http::HttpResponse* mResponse;

   /**
    * Stores any parsed path parameters.
    */
   monarch::rt::DynamicObject mPathParams;

   /**
    * Cache of parsed query variables in non-array mode.
    */
   monarch::rt::DynamicObject mQueryVars;

   /**
    * Cache of parsed query variables in array mode.
    */
   monarch::rt::DynamicObject mArrayQueryVars;

   /**
    * Stores a reference to any received content from
    * receiveContent(monarch::rt::DynamicObject& dyno).
    */
   monarch::rt::DynamicObject mContent;

   /**
    * Stores any handler-specific information.
    */
   monarch::rt::DynamicObject mHandlerInfo;

   /**
    * Stores any handler-specific data.
    */
   // TODO: here for future implementation of opaque user-data for handlers
   void* mHandlerData;

   /**
    * Stores the authentication method used. NULL for anonymous.
    */
   char* mAuthenticationMethod;

   /**
    * Stores the authentication data.
    */
   monarch::rt::DynamicObject mAuthenticationData;

   /**
    * Flag if content has already been received.
    */
   bool mContentReceived;

   /**
    * Flag if data has already been sent to the client.
    */
   bool mHasSent;

   /**
    * Flag for automatic selection of content-encoding for sending content.
    */
   bool mAutoContentEncode;

public:
   /**
    * Creates a new ServiceChannel for the passed path.
    *
    * @param path the full path from the client.
    */
   ServiceChannel(const char* path);

   /**
    * Destructs this ServiceChannel.
    */
   virtual ~ServiceChannel();

   /**
    * Initializes this ServiceChannel, creating its input and output Messages
    * and performing any other custom initialization necessary.
    */
   virtual void initialize();

   /**
    * Cleans up this ServiceChannel. By default the input and output Messages
    * are deleted.
    */
   virtual void cleanup();

   /**
    * Sets handler-specific information. A handler should know the context
    * underwhich it was added such that it knows how to interpret the object
    * that is set by this call. This object is typically used by extended
    * PathHandlers to store state to pass onto sub-handlers via customized
    * filters.
    *
    * @param info the handler info to set.
    */
   virtual void setHandlerInfo(monarch::rt::DynamicObject& info);

   /**
    * Gets handler-specific information. A PathHandler should know how to
    * interpret this data based on how it was attached to a WebService.
    *
    * @return the handler info.
    */
   virtual monarch::rt::DynamicObject& getHandlerInfo();

   /**
    * Sets the authentication method used and any associated data.
    *
    * @param method the method used.
    * @param data any associated data.
    */
   virtual void setAuthenticationMethod(
      const char* method, monarch::rt::DynamicObject* data = NULL);

   /**
    * Gets the authentication method, NULL indicates anonymous.
    *
    * @return the authentication method.
    */
   virtual const char* getAuthenticationMethod();

   /**
    * Gets the authentication data.
    *
    * @return the authentication data.
    */
   virtual monarch::rt::DynamicObject& getAuthenticationData();

   /**
    * Adds a Content-Encoding header if Accept-Encoding includes a supported
    * method.
    */
   virtual void selectContentEncoding();

   /**
    * Sets whether content-encoding will be automatically selected based on
    * the received Accept-Encoding header when sending content, if the
    * content-encoding header hasn't been set yet.
    *
    * @param on true to enable auto-selection of content-encoding, false to
    *           disable it.
    */
   virtual void setAutoContentEncode(bool on);

   /**
    * Receives the client's content and writes it to the passed output stream.
    *
    * @param os the stream to write the content to.
    * @param true to close the output stream when finished writing, false
    *           not to.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool receiveContent(monarch::io::OutputStream* os, bool close);

   /**
    * Receives the client's content and writes it to the passed DynamicObject.
    * This method will only be successful if the content-type is a supported
    * serialization for a DynamicObject.
    *
    * @param dyno the DynamicObject to write to.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool receiveContent(monarch::rt::DynamicObject& dyno);

   /**
    * Sends only the response header to the client with no content.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool sendNoContent();

   /**
    * Sends the response header and content in the passed input stream to the
    * client.
    *
    * @param is the content source input stream.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool sendContent(monarch::io::InputStream* is);

   /**
    * Sends the response header and passed DynamicObject to the client using
    * the content-type specified in the response header or an acceptable one
    * from the client. If the http response code is set to zero, this method
    * will automatically set it.
    *
    * @param dyno the DynamicObject to send.
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool sendContent(monarch::rt::DynamicObject& dyno);

   /**
    * Sends the response header and an exception as a DynamicObject to the
    * client using the content-type specified in the response header or an
    * acceptable one from the client. If the http response code is set to
    * zero, this method will automatically set it.
    *
    * @param e a reference to the Exception to send.
    * @param client true if the exception was due to the client, false if it
    *           was due to the server (this is used to set the response code
    *           if it is set to 0).
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool sendException(monarch::rt::ExceptionRef& e, bool client);

   /**
    * Gets the full normalized path.
    *
    * @return the full normalized path.
    */
   virtual const char* getPath();

   /**
    * Gets the path parameters that occur after the base path for the
    * PathHandler that created this ServiceChannel.
    *
    * @param params the DynamicObject to store the parameters in.
    *
    * @return true if there are parameters, false if there aren't any.
    */
   virtual bool getPathParams(monarch::rt::DynamicObject& params);

   /**
    * Gets the path's query variables.
    *
    * @param vars the DynamicObject to store the variables in.
    * @param asArrays true to create an array to hold all values for each key,
    *           false to use only the last value for each key.
    *
    * @return true if there are variables, false if there aren't any.
    */
   virtual bool getQuery(
      monarch::rt::DynamicObject& vars, bool asArrays = false);

   /**
    * Gets the "in" Message. This is the Message that is received from the
    * client.
    *
    * Useful for customized input.
    *
    * @return the "in" Message.
    */
   virtual Message* getInput();

   /**
    * Gets the "out" Message. This is the Message that is sent to the client.
    *
    * Useful for customized output.
    *
    * @return the "out" Message.
    */
   virtual Message* getOutput();

   /**
    * Sets the HttpRequest to use to receive content from the client.
    *
    * @param request the HttpRequest to use.
    */
   virtual void setRequest(monarch::http::HttpRequest* request);

   /**
    * Gets the HttpRequest associated with this channel.
    *
    * @return the HttpRequest associated with this channel.
    */
   virtual monarch::http::HttpRequest* getRequest();

   /**
    * Sets the HttpResponse to use to send to the client.
    *
    * @param response the HttpResponse to use to send to the client.
    */
   virtual void setResponse(monarch::http::HttpResponse* response);

   /**
    * Gets the HttpResponse associated with this channel.
    *
    * @return the HttpResponse associated with this channel.
    */
   virtual monarch::http::HttpResponse* getResponse();

   /**
    * Gets the underlying HttpConnection.
    *
    * @return the underlying HttpConnection.
    */
   virtual monarch::http::HttpConnection* getConnection();

   /**
    * Called by the PathHandler that created this channel to set the base
    * path (excluding parameters).
    *
    * @param path the base path (excluding parameters).
    */
   virtual void setBasePath(const char* res);

   /**
    * Gets the base path as set by the PathHandler.
    *
    * @return the base path as set by the PathHandler.
    */
   virtual const char* getBasePath();

   /**
    * Checks to see if a header or content has been sent to the client yet. If
    * something has already been sent to the client then further send() calls
    * will do nothing and succeed.
    *
    * @return true if the client has been sent data, false if not.
    */
   virtual bool hasSent();

   /**
    * Set whether or not the client has been sent data.
    *
    * @param sent true if the client has been sent data, false if not.
    */
   virtual void setSent(bool sent = true);

   /**
    * Overrides the content object with the given one. If receiveContent() is
    * called, this object will be written to and returned.
    *
    * @param content the content object to use.
    */
   virtual void setContentObject(monarch::rt::DynamicObject& content);

   /**
    * Gets the client's internet address.
    *
    * @param address the client's internet address to populate.
    *
    * @return true if successful, false if an error occurred.
    */
   virtual bool getClientAddress(monarch::net::InternetAddress* address);

   /**
    * A helper method for getting the request method.
    *
    * @return the request method.
    */
   virtual Message::MethodType getRequestMethod();
};

} // end namespace ws
} // end namespace monarch
#endif
