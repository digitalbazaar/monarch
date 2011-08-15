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
    * Possible return codes for checkAuthentication
    */
   enum Result
   {
      /**
       * The request is denied.
       */
      Deny = -2,
      /*
       * The request failed to be authenticated but others may be checked.
       */
      Failure = -1,
      /**
       * This request could not be checked by this authenticator.
       */
      NotChecked = 0,
      /*
       * The request was successfully authenticated.
       */
      Success = 1
   };

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
    * ServiceChannel. This method must return Success.
    *
    * If an authentication attempt was made by the client and it was
    * unsuccessful, then setAuthenticationException() must be called on
    * the ServiceChannel. If other authenticators should be checked then this
    * method should return Failure. If other authenticators should not be
    * checked then this method should return Deny.
    *
    * If no authentication attempt was made by the client, then this method
    * must return NotChecked. No calls to the channel are required.
    *
    * @param ch the communication channel with the client.
    *
    * @return Success if the request is authenticated, Failure if the client
    *         attempted to authorize the request but failed, NoCheck if the
    *         client did not attempt to authenticate the request (and,
    *         therefore, it is not authenticated), and Denied if a request
    *         should be explicitly denied.
    */
   virtual Result checkAuthentication(ServiceChannel* ch);
};

// type definition for a reference counted RequestAuthenticator
typedef monarch::rt::Collectable<RequestAuthenticator> RequestAuthenticatorRef;

} // end namespace ws
} // end namespace monarch
#endif
