/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_oauth1_OAuth1Authenticator_H
#define monarch_oauth1_OAuth1Authenticator_H

#include "monarch/ws/RequestAuthenticator.h"
#include "monarch/oauth1/OAuth1Provider.h"

namespace monarch
{
namespace oauth1
{

/**
 * An Oauth1Authenticator determines if a request made to a WebService
 * is authenticated using OAuth1.
 *
 * @author Dave Longley
 */
class OAuth1Authenticator : public monarch::ws::RequestAuthenticator
{
protected:
   /**
    * The OAuth1 implementation.
    */
   OAuth1 mOAuth1;

   /**
    * The type of token to check for.
    */
   OAuth1::TokenType mTokenType;

   /**
    * True to require a secure connection.
    */
   bool mRequireSecureConnection;

   /**
    * True to allow plain text signatures.
    */
   bool mAllowPlainText;

public:
   /**
    * Creates a new OAuth1Authenticator.
    *
    * @param provider the OAuth1Provider to use.
    * @param tokenType the type of token to verify (NullToken for no token).
    * @param requireSecureConnection true to require a secure connection
    *           (eg: SSL), defaults to true.
    * @param allowPlainText true to allow non-plain text signatures,
    *           defaults to false.
    */
   OAuth1Authenticator(
      OAuth1Provider* provider, OAuth1::TokenType tokenType,
      bool requireSecureConnection = true, bool allowPlainText = false);

   /**
    * Destructs this OAuth1Authenticator.
    */
   virtual ~OAuth1Authenticator();

   /**
    * {@inheritDoc}
    */
   virtual monarch::ws::RequestAuthenticator::Result
      checkAuthentication(monarch::ws::ServiceChannel* ch);
};

} // end namespace oauth1
} // end namespace monarch
#endif
