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
    * ServiceChannel.
    *
    * If an authentication attempt was made by the client and it was
    * unsuccessful, then setAuthenticationException() must be called on
    * the ServiceChannel.
    *
    * If no authentication attempt was made by the client, then this method
    * must return false. No calls to the channel are required.
    *
    * @param ch the communication channel with the client.
    *
    * @return true if the request is authenticated, false if not with
    *         exception set.
    */
   virtual bool checkAuthentication(ServiceChannel* ch);
};

// type definition for a reference counted RequestAuthenticator
typedef monarch::rt::Collectable<RequestAuthenticator> RequestAuthenticatorRef;

} // end namespace ws
} // end namespace monarch
#endif
