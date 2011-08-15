/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/oauth1/OAuth1Authenticator.h"

using namespace std;
using namespace monarch::oauth1;
using namespace monarch::rt;
using namespace monarch::ws;

#define MO_OAUTH1       "monarch.oauth1"
#define OAUTH1_METHOD   "oauth1"

OAuth1Authenticator::OAuth1Authenticator(
   OAuth1Provider* provider,
   OAuth1::TokenType tokenType,
   bool requireSecureConnection, bool allowPlainText):
   mTokenType(tokenType),
   mRequireSecureConnection(requireSecureConnection),
   mAllowPlainText(allowPlainText)
{
   mOAuth1.setProvider(provider);
}

OAuth1Authenticator::~OAuth1Authenticator()
{
}

static bool _checkConnectionSecurity(ServiceChannel* ch, bool require)
{
   bool rval = true;

   // secure connection required
   if(require && !ch->getRequest()->getConnection()->isSecure())
   {
      ExceptionRef e = new Exception(
         "All OAuth calls to the service must be performed over a secure "
         "channel. Please use Transaction Layer Security (TLS) to make all "
         "OAuth-related calls to the service.",
         MO_OAUTH1 ".NonSecureConnection");
      Exception::push(e);
      rval = false;
   }

   return rval;
}

static bool _checkSignatureSecurity(OAuth1Params& params, bool allow)
{
   bool rval = true;

   // plain text signatures not permitted
   if(!allow)
   {
      string method;
      rval = OAuth1::getParameter(
         params, "oauth_signature_method", method, false);
      if(rval && strcmp(method.c_str(), "PLAINTEXT") == 0)
      {
         ExceptionRef e = new Exception(
            "The service does not accept PLAINTEXT OAuth signatures. The use "
            "of PLAINTEXT is considered a very weak cryptographic practice "
            "where secrets are openly shared when transmitting messages. "
            "You must use the more cryptographically secure HMAC-SHA1 "
            "signature mechanism instead.",
            MO_OAUTH1 ".SignatureMethodTooWeak");
         Exception::push(e);
         rval = false;
      }
   }

   return rval;
}

RequestAuthenticator::Result
   OAuth1Authenticator::checkAuthentication(ServiceChannel* ch)
{
   // default to client did not attempt to use oauth1
   RequestAuthenticator::Result rval = NotChecked;

   // check to see if the OAuth parameters were specified, if they were
   // check to see if they are valid and return more information that is
   // associated with the OAuth token
   string client;
   OAuth1Params params = mOAuth1.getParameters(ch);
   if(params->hasMember("oauth_consumer_key"))
   {
      bool pass =
         _checkConnectionSecurity(ch, mRequireSecureConnection) &&
         _checkSignatureSecurity(params, mAllowPlainText) &&
         mOAuth1.verify(ch->getRequest(), params, mTokenType);
      if(pass)
      {
         // set authentication method
         DynamicObject data;
         data["params"] = params;
         ch->setAuthenticationMethod(OAUTH1_METHOD, data);
         rval = Success;
      }
      else
      {
         ExceptionRef e = new Exception(
            "There was an issue verifying the OAuth information that was "
            "presented to the service.",
            MO_OAUTH1 ".InvalidOAuthCredentials");
         Exception::push(e);

         // client attempted to authenticate, save exception w/channel
         ch->setAuthenticationException(OAUTH1_METHOD, e);

         // client attempted oauth1, set to failed
         rval = Failure;
      }
   }

   return rval;
}
