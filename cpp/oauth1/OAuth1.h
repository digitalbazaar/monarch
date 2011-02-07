/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_oauth1_OAuth1_H
#define monarch_oauth1_OAuth1_H

#include "monarch/rt/DynamicObject.h"
#include "monarch/util/Pattern.h"
#include "monarch/ws/ServiceChannel.h"

namespace monarch
{
namespace oauth1
{

// forward declarations
class OAuth1Provider;

/**
 * OAuth1Params is a DynamicObject map of key => values. Each key
 * is a named oauth parameter and each value is an array of all of the
 * values that were presented in an oauth request with that key.
 *
 * The reason that the values are arrays is because the keys may appear
 * multiple times in a query parameter, an Authorization header, and/or
 * a POST body.
 *
 * OAuth1Params are extracted from an oauth request by combining all
 * parameters from the query, the POST data, and the Authorization header.
 */
typedef monarch::rt::DynamicObject OAuth1Params;

/**
 * An OAuth1 object implements the OAuth v1.0 protocol.
 *
 * @author Dave Longley
 * @author Manu Sporny
 * @author David I. Lehn
 */
class OAuth1
{
public:
   /**
    * Token types.
    */
   enum TokenType
   {
      NullToken = 0,
      AccessToken,
      RequestToken
   };

protected:
   /**
    * The OAuth1Provider to use.
    */
   // FIXME: add client support, ability to set a client interface or
   // consolidate common parts (getting secrets) into another interface
   OAuth1Provider* mProvider;

   /**
    * The regex pattern to split apart the Authorization HTTP header.
    */
   monarch::util::PatternRef mAuthRegex;

public:
   /**
    * Creates a new OAuth1.
    */
   OAuth1();

   /**
    * Destructs this OAuth1.
    */
   virtual ~OAuth1();

   /**
    * Sets the OAuth1Provider to use with this object.
    *
    * @param provider the OAuth1Provider API to use.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool setProvider(OAuth1Provider* provider);

   /**
    * Gets the OAuth parameters from a ServiceChannel and returns them as
    * an OAuth1Params object. If the content for the request has not yet been
    * received, it will be by calling this method.
    *
    * @param ch the ServiceChannel that contains some of the OAuth parameters.
    *
    * @return a map containing all OAuth parameters that were extracted from
    *         the query parameters, Authorization header, and POST body.
    */
   virtual OAuth1Params getParameters(monarch::ws::ServiceChannel* ch);

   /**
    * Creates an OAuth signature for an HttpRequest. Any old signature value
    * will be removed from the OAuth1Params. The method will be set in the
    * given OAuth1Params and, on success, the new signature value will also be
    * set in the OAuth1Params.
    *
    * @param request the HttpRequest to be signed.
    * @param params the OAuth1Params to use.
    * @param method the method to use.
    * @param tokenType the token type the signature is for.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool sign(
      monarch::http::HttpRequest* request, OAuth1Params& params,
      const char* method, OAuth1::TokenType tokenType);

   /**
    * Verifies the OAuth signature on an HttpRequest.
    *
    * @param request the HttpRequest to verify the signature for.
    * @param params the OAuth1Params associated with the request.
    * @param tokenType the expected token type, use NullToken to verify
    *           an HTTP request to generate a request token (there is no
    *           token yet).
    *
    * @return true if the signature verifies, false if not with exception set.
    */
   virtual bool verify(
      monarch::http::HttpRequest* request, OAuth1Params& params,
      OAuth1::TokenType tokenType);

   /**
    * Generates an OAuth verifier. An OAuth verifier is used to grant
    * Resource Owner Authorization. When a client wants access to a particular
    * resource, it must direct the resource owner to the server for them
    * to grant access to the client. If access is granted, an unguessable
    * verifier code is created by the server.
    *
    * The verifier code is usually returned in the query of the callback
    * request but it may be displayed to the resource owner so that they can
    * manually enter it. If this is the case, and the resource owner is on a
    * limited (eg: mobile) device, then the verifier code should be a short
    * enough value for them to manually enter.
    *
    * @param manualEntry true if the resource owner is on a limited device that
    *           requires manual entry, false for a more secure verifier.
    * @param length the length for the verifier, 0 to use the default.
    *
    * @return the verifier code string.
    */
   virtual std::string generateVerifier(
      bool manualEntry = false, int length = 0);

   /**
    * Generates an OAuth client key and shared-secret, aka "client credentials".
    *
    * @param key string to populate with a new client key.
    * @param secret string to populate with a new client shared-secret.
    * @param keyLength the key length to use, 0 to use the default.
    * @param secretLength the secret length to use, 0 to use the default.
    */
   virtual void generateClientCredentials(
      std::string& key, std::string& secret,
      int keyLength = 0, int secretLength = 0);

   /**
    * Generates an OAuth token and secret, aka "temporary credentials" or
    * "token credentials" (the former term is used with request tokens, the
    * latter term is used with access tokens).
    *
    * The client obtains a set of temporary credentials from the server by
    * making an authenticated HTTP "POST" request to the Temporary Credential
    * Request endpoint (unless the server advertises another HTTP request
    * method for the client to use).
    *
    * The client obtains a set of token credentials from the server by making
    * an authenticated HTTP "POST" request to the Token Request endpoint
    * (unless the server advertises another HTTP request method for the client
    * to use).
    *
    * @param token string to populate with a new token.
    * @param secret string to populate with a new secret.
    * @param tokenLength the token length to use, 0 to use the default.
    * @param secretLength the secret length to use, 0 to use the default.
    */
   virtual void generateTokenCredentials(
      std::string& token, std::string& secret,
      int tokenLength = 0, int secretLength = 0);

   /**
    * A helper method that retrieves the value of the first OAuth1 parameter
    * from an OAuth1Params object by name.
    *
    * @param params an OAuth1Params.
    * @param name the name of the parameter to fetch.
    * @param value the value of the parameter will be stored in this variable
    *           if it is found.
    * @param optional whether or not the parameter is optional, if true then
    *           no exception will be set if the parameter doesn't exist and
    *           the return value will always be true.
    *
    * @return true if the parameter exists or optional is true, false otherwise.
    */
   static bool getParameter(
      OAuth1Params& params, const char* name,
      std::string& value, bool optional = false);

   /**
    * Gets the signature and method from the given OAuth1Params.
    *
    * @param params an OAuth1Params.
    * @param method to be populated with the signature method if found in the
    *           params, cleared otherwise.
    * @param signature to be populated with the signature value if found in
    *           the params, cleared otherwise.
    */
   static void getSignature(
      OAuth1Params& params, std::string& method, std::string& signature);
};

} // end namespace oauth1
} // end namespace monarch
#endif
