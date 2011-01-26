/*
 * Copyright (c) 2010-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_PathHandler_H
#define monarch_ws_PathHandler_H

#include "monarch/ws/RequestAuthenticator.h"

#include <vector>

namespace monarch
{
namespace ws
{

/**
 * A PathHandler handles a path for a WebService in some fashion. Its
 * operator() should receive the content from a client using the passed
 * ServiceChannel and then it should send an appropriate response.
 *
 * @author Dave Longley
 */
class PathHandler
{
protected:
   /**
    * True if a secure connection is required, false if not.
    */
   bool mSecureOnly;

   /**
    * A list of request authenticators.
    */
   typedef std::vector<RequestAuthenticatorRef> RequestAuthList;
   RequestAuthList mAuthMethods;

public:
   /**
    * Creates a new PathHandler.
    *
    * @param secureOnly true if this PathHandler should return a 404 if the
    *           connection is not secure.
    */
   PathHandler(bool secureOnly = false);

   /**
    * Destructs this PathHandler.
    */
   virtual ~PathHandler();

   /**
    * Checks to see if the handler can handle the client's request. If not,
    * an exception must be set that will be sent to the client. The default
    * implementation will call checkAuthentication().
    *
    * @param ch the communication channel with the client.
    *
    * @return true if the request can be handled.
    */
   virtual bool canHandleRequest(ServiceChannel* ch);

   /**
    * Checks to see if the request sent over the given channel is authenticated
    * according to one of the given authentication methods. Once a channel is
    * authenticated by one of the methods, the authentication method and data
    * will be set and this method will return.
    *
    * If no authentication methods were specified, then this method will set
    * the authentication method to NULL and return true.
    *
    * If no authentication methods are matched, then this method will return
    * false with an exception set.
    *
    * @return true if authenticated, false if not with exception set.
    */
   virtual bool checkAuthentication(ServiceChannel* ch);

   /**
    * Handles the client's request. Does whatever is necessary to handle the
    * client's request and sends a response.
    *
    * @param ch the communication channel with the client.
    */
   virtual void handleRequest(ServiceChannel* ch);

   /**
    * Handle's the client's request by receiving its content, if any, and
    * sending an appropriate response.
    *
    * The default implementation for this method will first call
    * canHandleRequest() and then if that returns true it will call
    * handleRequest().
    *
    * @param ch the communication channel with the client.
    */
   virtual void operator()(ServiceChannel* ch);

   /**
    * Returns true if this handler requires a secure connection (ie: SSL/TLS),
    * false if not.
    *
    * @return true if this handler requires a secure connection, false if not.
    */
   virtual bool secureConnectionRequired();

   /**
    * Adds a RequestAuthenticator to this handler. Authentication methods are
    * checked in the order that they are added.
    *
    * @param method the RequestAuthenticator to add.
    */
   virtual void addRequestAuthenticator(RequestAuthenticatorRef& method);
};

// type definition for a reference counted PathHandler
typedef monarch::rt::Collectable<PathHandler> PathHandlerRef;

} // end namespace ws
} // end namespace monarch
#endif
