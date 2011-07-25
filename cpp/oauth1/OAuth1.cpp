/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/oauth1/OAuth1.h"

#include "monarch/crypto/BigInteger.h"
#include "monarch/crypto/HashMac.h"
#include "monarch/data/json/JsonWriter.h"
#include "monarch/logging/Logging.h"
#include "monarch/oauth1/OAuth1Provider.h"
#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/util/Base64Codec.h"

using namespace std;
using namespace monarch::crypto;
using namespace monarch::data::json;
using namespace monarch::http;
using namespace monarch::net;
using namespace monarch::oauth1;
using namespace monarch::rt;
using namespace monarch::util;
using namespace monarch::ws;

#define MO_OAUTH1 "monarch.oauth1"

// default character length of credentials and verifiers
// bits depends on encoding (eg: decimal, hex, base64, etc)
#define DEFAULT_CLIENT_KEY_LENGTH       16
#define DEFAULT_CLIENT_SECRET_LENGTH    16
#define DEFAULT_TOKEN_LENGTH            32
#define DEFAULT_TOKEN_SECRET_LENGTH     16
#define DEFAULT_VERIFIER_LENGTH         16
#define DEFAULT_SHORT_VERIFIER_LENGTH   6

OAuth1::OAuth1() :
   mProvider(NULL)
{
}

OAuth1::~OAuth1()
{
}

bool OAuth1::setProvider(OAuth1Provider* provider)
{
   bool rval = true;

   // set provider API
   mProvider = provider;

   // compile Authorization header regex (only needed for providers)
   mAuthRegex = Pattern::compile("([[:alnum:]_]+)=[[:space:]]?\"?([^\"]+)\"?");
   rval = !mAuthRegex.isNull();

   return rval;
}

inline static string _encode(const char* data)
{
   // use percent encoding for spaces
   return Url::encode(data, false);
}

inline static string _decode(const char* data)
{
   return Url::decode(data);
}

OAuth1Params OAuth1::getParameters(ServiceChannel* ch)
{
   OAuth1Params rval(Map);

   // get parameters from Authorization header
   HttpRequestHeader* header = ch->getRequest()->getHeader();
   if(header->hasField("Authorization"))
   {
      // use regex to parse header values
      DynamicObject values(Array);
      mAuthRegex->split(header->getFieldValue("Authorization").c_str(), values);

      // log authorization info
      MO_CAT_DEBUG_DATA(MO_OAUTH1_CAT,
         "Authorization Header Values:\n%s",
         JsonWriter::writeToString(values).c_str());

      // append to oauth1 params (eg: [key,value,key,value,key,value])
      for(int i = 0; i < values->length(); i += 2)
      {
         DynamicObject& value = rval[_decode(values[i]).c_str()];
         value->setType(Array);
         value->append(_decode(values[i + 1]).c_str());
      }
   }

   // if form encoding is used, get params from POST
   if(strncmp(
      header->getFieldValue("Content-Type").c_str(),
      "application/x-www-form-urlencoded", 33) == 0)
   {
      // get form input (will receive it if not yet received, otherwise will
      // just return already received form)
      DynamicObject form;
      if(ch->receiveContent(form) && !form.isNull())
      {
         MO_CAT_DEBUG_DATA(MO_OAUTH1_CAT,
            "POST body:\n%s",
            JsonWriter::writeToString(form).c_str());

         // merge in form, ensure all values are arrays
         DynamicObjectIterator i = form.getIterator();
         while(i->hasNext())
         {
            DynamicObject& value = i->next();
            const char* field = i->getName();
            if(value->getType() == Array)
            {
               rval[field].merge(value, true);
            }
            else
            {
               rval[field]->append(value);
            }
         }
      }
   }

   // parse the URL query parameters as arrays and append them
   DynamicObject queryParams;
   ch->getQuery(queryParams, true);
   rval.merge(queryParams, true);

   MO_CAT_DEBUG_DATA(MO_OAUTH1_CAT,
      "OAuth parameters:\n%s",
      JsonWriter::writeToString(rval).c_str());

   return rval;
}

static string _generateSignatureKey(
   const char* clientSecret, const char* tokenSecret)
{
   string rval;

   // 1. The client shared-secret, after being encoded (Section 3.6).
   if(clientSecret != NULL)
   {
      rval.append(_encode(clientSecret));
   }

   // 2. An "&" character (ASCII code 38), which MUST be included even
   //    when either secret is empty.
   rval.append("&");

   // 3. The token shared-secret, after being encoded (Section 3.6).
   if(tokenSecret != NULL)
   {
      rval.append(_encode(tokenSecret));
   }

   return rval;
}

static string _normalizeUri(HttpRequest* request)
{
   string rval;

   // use https for secure, http for non-secure
   string uri = (
      request->getConnection() != NULL &&
      request->getConnection()->isSecure()) ? "https://" : "http://";

   // get the host and path for the URL
   HttpRequestHeader* header = request->getHeader();
   uri.append(header->getFieldValue("Host").c_str());
   uri.append(header->getPath());

   /* Now normalize the URI:

      1.  The scheme and host MUST be in lowercase.
      2.  The host and port values MUST match the content of the HTTP
          request "Host" header field.
      3.  The port MUST be included if it is not the default port for the
          scheme, and MUST be excluded if it is the default.  Specifically,
          the port MUST be excluded when making an HTTP request [RFC2616]
          to port 80 or when making an HTTPS request [RFC2818] to port 443.
          All other non-default port numbers MUST be included.
   */
   Url url(uri);
   string hostPort = (url.getPort() != url.getDefaultPort()) ?
      url.getHostAndPort().c_str() : url.getHost().c_str();
   rval = StringTools::format("%s://%s%s",
      StringTools::toLower(url.getScheme().c_str()).c_str(),
      StringTools::toLower(hostPort.c_str()).c_str(),
      url.getPath().c_str());

   MO_CAT_DEBUG_DATA(MO_OAUTH1_CAT, "OAuth URI: '%s'", rval.c_str());

   return rval;
}

static string _normalizeParams(OAuth1Params& params)
{
   string rval;

   // build a map of encoded keys to arrays of encoded values (it will be
   // automatically sorted on encoded keys)
   DynamicObject encodedParams(Map);

   // 1. First, the name and value of each parameter are encoded (Section 3.6).
   DynamicObjectIterator pi = params.getIterator();
   while(pi->hasNext())
   {
      DynamicObject& values = pi->next();

      // used to sort encoded values
      DynamicObject sorted(Array);

      // 2. The parameters are sorted by name, using ascending byte value
      //    ordering. If two or more parameters share the same name, they
      //    are sorted by their value.
      DynamicObjectIterator vi = values.getIterator();
      while(vi->hasNext())
      {
         sorted->append(_encode(vi->next()).c_str());
      }
      sorted.sort();

      // encode the key and insert the kv-pair into the sorted map
      encodedParams[_encode(pi->getName()).c_str()] = sorted;
   }

   // 3. The name of each parameter is concatenated to its corresponding
   //    value using an "=" character (ASCII code 61) as a separator, even
   //    if the value is empty.
   pi = encodedParams.getIterator();
   bool firstItem = true;
   while(pi->hasNext())
   {
      DynamicObject& values = pi->next();
      DynamicObjectIterator vi = values.getIterator();
      while(vi->hasNext())
      {
         // 4. The sorted name/value pairs are concatenated together into a
         //    single string by using an "&" character (ASCII code 38) as
         //    separator.
         if(!firstItem)
         {
            rval.push_back('&');
         }
         else
         {
            firstItem = false;
         }

         // append the parameter to the end of the string.
         rval.append(pi->getName());
         rval.push_back('=');
         rval.append(vi->next());
      }
   }

   return rval;
}

static string _generateSignatureBaseString(
   HttpRequest* request, DynamicObject& params)
{
   string rval;

   /* Algorithm to generate OAuth 1.0 "Signature Base String":

      1. The HTTP request method in uppercase. For example: "HEAD",
         "GET", "POST", etc. If the request uses a custom HTTP method, it
         MUST be encoded (Section 3.6).
      2. An "&" character (ASCII code 38).
      3. The base string URI from Section 3.4.1.2, after being encoded
         (Section 3.6).
      4. An "&" character (ASCII code 38).
      5. The request parameters as normalized in Section 3.4.1.3.2,
         after being encoded (Section 3.6).
   */
   HttpRequestHeader* header = request->getHeader();
   rval.append(StringTools::toUpper(header->getMethod()));
   rval.push_back('&');
   rval.append(_encode(_normalizeUri(request).c_str()));
   rval.push_back('&');
   rval.append(_encode(_normalizeParams(params).c_str()));
   return rval;
}

static string _generateHmacSha1Signature(
   const char* signatureBaseString,
   const char* clientSecret, const char* tokenSecret)
{
   string rval;

   // generate the HMAC-SHA1 digest
   SymmetricKeyRef skey = new SymmetricKey();
   string key = _generateSignatureKey(clientSecret, tokenSecret);
   skey->setData(key.c_str(), key.length());
   HashMac hmac;
   hmac.start("SHA1", skey);
   hmac.update(signatureBaseString);

   // base-64 encode the HMAC-SHA1 byte stream
   unsigned int length = hmac.getValueLength();
   char macValue[length];
   hmac.getValue(macValue, length);
   rval = Base64Codec::encode(macValue, length);

   return rval;
}

bool OAuth1::sign(
   HttpRequest* request, OAuth1Params& params,
   const char* method, OAuth1::TokenType tokenType)
{
   bool rval = true;

   string signature;

   // set method in parameters, remove and old signature
   params->removeMember("oauth_signature_method");
   params["oauth_signature_method"]->append(method);
   params->removeMember("oauth_signature");

   // get the token secret from the provider, if applicable
   string tokenSecret;
   if(tokenType != OAuth1::NullToken)
   {
      rval = mProvider->getTokenSecret(params, tokenType, tokenSecret);
   }

   // get client secret from the provider
   string clientSecret;
   rval = rval && mProvider->getClientSecret(params, clientSecret);
   if(rval)
   {
      // generate signature key
      string key = _generateSignatureKey(
         clientSecret.c_str(), tokenSecret.c_str());

      if(strcmp(method, "PLAINTEXT") == 0)
      {
         // PLAINTEXT simply uses the key that the other methods use
         signature = key;
      }
      else
      {
         // generate "Signature Base String" according to spec
         string signatureBaseString =
            _generateSignatureBaseString(request, params);

         // hmac-sha1 method
         if(strcmp(method, "HMAC-SHA1") == 0)
         {
            signature = _generateHmacSha1Signature(
               signatureBaseString.c_str(),
               clientSecret.c_str(), tokenSecret.c_str());
         }
         // unknown method
         else
         {
            ExceptionRef e = new Exception(
               "OAuth signature method is not supported. "
               "Supported signature methods: PLAINTEXT and HMAC-SHA1.",
                MO_OAUTH1 ".SignatureMethodNotSupported");
            e->getDetails()["method"] = method;
            Exception::set(e);
            rval = false;
         }

         MO_CAT_DEBUG_DATA(MO_OAUTH1_CAT,
            "Signature Base String:\n   %s", signatureBaseString.c_str());
      }
   }

   // update signature in params
   if(rval)
   {
      params["oauth_signature"][0] = signature.c_str();
   }

   return rval;
}

bool OAuth1::verify(
   HttpRequest* request, OAuth1Params& params, OAuth1::TokenType tokenType)
{
   bool rval = false;

   // get the signature
   string method;
   string givenSignature;
   getSignature(params, method, givenSignature);

   // sign the request
   rval = sign(request, params, method.c_str(), tokenType);
   if(rval)
   {
      // compare the signature results
      const char* generatedSignature = params["oauth_signature"][0];
      rval = (strcmp(givenSignature.c_str(), generatedSignature) == 0);
      if(!rval)
      {
         // copy generated signature, restore old signature in params
         string gs = generatedSignature;
         params["oauth_signature"][0] = givenSignature.c_str();
         ExceptionRef e = new Exception(
            "The OAuth signature is invalid.",
             MO_OAUTH1 ".InvalidSignature");
         Exception::set(e);

         MO_CAT_DEBUG_DATA(MO_OAUTH1_CAT,
            "Given Signature: %s\nGenerated Signature: %s",
            givenSignature.c_str(), gs.c_str());
      }
   }

   // if signature method is not PLAINTEXT, then additional checks are needed
   if(rval && strcmp(method.c_str(), "PLAINTEXT") != 0)
   {
      // required parameters
      if(!(params->hasMember("oauth_consumer_key") &&
         params->hasMember("oauth_timestamp") &&
         params->hasMember("oauth_nonce")))
      {
         ExceptionRef e = new Exception(
            "An oauth_token, oauth_timestamp, and an oauth_nonce parameter "
            "must be included when signing OAuth requests using HMAC-SHA1 "
            "or RSA-SHA1.",
             MO_OAUTH1 ".MissingSigningParameters");
         Exception::set(e);
         rval = false;
      }
      else
      {
         // do custom provider validation
         rval = mProvider->validateTimestampAndNonce(params, true);
      }
   }

   return rval;
}

static string _generateUrlSafeRandom(int length)
{
   // url-safe-encode and trim to length
   char bytes[length];
   BigInteger::randomBytes(bytes, length);
   return Base64Codec::urlSafeEncode(bytes, length).erase(length);
}

static void _generateCredentials(
   string& id, string& secret, int idLength, int secretLength)
{
   // generate id and secret
   id = _generateUrlSafeRandom(idLength);
   secret = _generateUrlSafeRandom(secretLength);
}

void OAuth1::generateClientCredentials(
   string& key, string& secret, int keyLength, int secretLength)
{
   if(keyLength == 0)
   {
      keyLength = DEFAULT_CLIENT_KEY_LENGTH;
   }
   if(secretLength == 0)
   {
      secretLength = DEFAULT_CLIENT_SECRET_LENGTH;
   }
   _generateCredentials(key, secret, keyLength, secretLength);
}

void OAuth1::generateTokenCredentials(
   string& token, string& secret, int tokenLength, int secretLength)
{
   if(tokenLength == 0)
   {
      tokenLength = DEFAULT_TOKEN_LENGTH;
   }
   if(secretLength == 0)
   {
      secretLength = DEFAULT_TOKEN_SECRET_LENGTH;
   }
   _generateCredentials(token, secret, tokenLength, secretLength);
}

string OAuth1::generateVerifier(bool manualEntry, int length)
{
   // FIXME: use simpler char set for simple mode? [0-9] or easy to read/type?
   if(length == 0)
   {
      length = manualEntry ?
         DEFAULT_SHORT_VERIFIER_LENGTH : DEFAULT_VERIFIER_LENGTH;
   }
   return _generateUrlSafeRandom(length);
}

bool OAuth1::getParameter(
   OAuth1Params& params, const char* name, string& value, bool optional)
{
   bool rval = true;

   // param exists, retrieve it
   if(params->hasMember(name))
   {
      value = params[name][0]->getString();
   }
   // param does not exist and is required, set exception
   else if(!optional)
   {
      ExceptionRef e = new Exception(
         "A required OAuth parameter is missing.",
         MO_OAUTH1 ".MissingRequiredParameter");
      e->getDetails()["missingParameter"] = name;
      Exception::set(e);
      rval = false;
   }
   // param does not exist, but is optional, so just clear value
   else
   {
      value.clear();
   }

   return rval;
}

void OAuth1::getSignature(
   OAuth1Params& params, string& method, string& signature)
{
   // check for signature in params
   if(!params->hasMember("oauth_signature"))
   {
      // no signature in params
      signature.clear();
   }
   else
   {
      // get signature from params, remove if requested
      signature = params["oauth_signature"][0]->getString();
   }

   // check for signature method
   if(!params->hasMember("oauth_signature_method"))
   {
      // no method in params
      method.clear();
   }
   else
   {
      // get method
      method = params["oauth_signature_method"][0]->getString();
   }
}
