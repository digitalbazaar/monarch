/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_ws_RequestAuthenticator_H
#define monarch_ws_RequestAuthenticator_H

#include "monarch/ws/ServiceChannel.h"

namespace monarch
{
namespace ws
{

/**
 * A RequestAuthenticator determines if a request made to a WebService is
 * authenticated.
 *
 * This class is the base class for all RequestAuthenticators. It may be
 * used as an anonymous request authenticator.
 *
 * @author Dave Longley
 */
class RequestAuthenticator
{
public:
   /**
    * Creates a new RequestAuthenticator.
    */
   RequestAuthenticator();

   /**
    * Destructs this RequestAuthenticator.
    */
   virtual ~RequestAuthenticator();

   /**
    * Checks to see if a request made over the given channel is authenticated.
    *
    * If an authentication attempt was made by the client and it was
    * successful, then setAuthenticationMethod() must be called on the
    * ServiceChannel. This method must return 1.
    *
    * If an authentication attempt was made by the client and it was
    * unsuccessful, then setAuthenticationException() must be called on
    * the ServiceChannel. This method must return -1.
    *
    * If no authentication attempt was made by the client, then this method
    * must return 0. No calls to the channel are required.
    *
    * @param ch the communication channel with the client.
    *
    * @return 1 if the request is authenticated, -1 if the client attempted
    *         to authorize the request but failed, and 0 if the client did
    *         not attempt to authenticate the request (and, therefore, it
    *         is not authenticated).
    */
   virtual int checkAuthentication(ServiceChannel* ch);
};

// type definition for a reference counted RequestAuthenticator
typedef monarch::rt::Collectable<RequestAuthenticator> RequestAuthenticatorRef;

} // end namespace ws
} // end namespace monarch
#endif
