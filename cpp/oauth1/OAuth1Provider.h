/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_oauth1_OAuth1Provider_H
#define monarch_oauth1_OAuth1Provider_H

#include "monarch/oauth1/OAuth1.h"

namespace monarch
{
namespace oauth1
{

/**
 * An OAuth1Provider is an interface used by an OAuth1 object to perform its
 * duties. The implementing class of OAuth1Provider provides storage for
 * OAuth tokens and other customizations that are necessary to authenticate
 * clients of the service provided.
 *
 * @author Dave Longley
 */
class OAuth1Provider
{
public:
   /**
    * Creates a new OAuth1Provider.
    */
   OAuth1Provider() {};

   /**
    * Destructs this OAuth1Provider.
    */
   virtual ~OAuth1Provider() {};

   /**
    * Validates a timestamp and nonce against an optional token. All values
    * are provided in the given OAuth1Params object.
    *
    * oauth_consumer_key: The client ID.
    * outh_nonce: The nonce.
    * oauth_timestamp: The timestamp.
    * oauth_token: The optional token, may not be present.
    *
    * @param params the OAuth1Params.
    * @param update if true, updates the timestamp and nonce values in storage
    *           so that the same token-timestamp-nonce combination cannot
    *           be re-used.
    *
    * @return true if the timestamp and nonce were validated, false if not
    *         with exception set.
    */
   virtual bool validateTimestampAndNonce(
      OAuth1Params& params, bool update = true) = 0;

   /**
    * Gets the client secret associated with the given OAuth1Params object.
    *
    * @param params the OAuth1Params.
    * @param secret a string to fill with the client secret if found.
    *
    * @return true if successful, false if not with exception set.
    */
   virtual bool getClientSecret(OAuth1Params& params, std::string& secret) = 0;

   /**
    * Gets the token secret associated with the given OAuth1Params object and
    * token type.
    *
    * @param params the OAuth1Params.
    * @param tokenType the token type (AccessToken or RequestToken only).
    * @param secret a string to fill with the client secret if found.
    *
    * @return true if successful, false if not with exception set.
    */
   virtual bool getTokenSecret(
      OAuth1Params& params, OAuth1::TokenType tokenType,
      std::string& secret) = 0;
};

} // end namespace oauth1
} // end namespace monarch
#endif
