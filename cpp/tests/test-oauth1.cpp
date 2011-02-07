/*
 * Copyright (c) 2011 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_CONSTANT_MACROS

#include "monarch/test/Test.h"
#include "monarch/test/TestModule.h"
#include "monarch/oauth1/OAuth1Provider.h"

#include <cstdio>

using namespace std;
using namespace monarch::http;
using namespace monarch::oauth1;
using namespace monarch::rt;
using namespace monarch::test;

namespace mo_test_oauth1
{

class TestProvider : public OAuth1Provider
{
public:
   virtual bool validateTimestampAndNonce(
      OAuth1Params& params, bool update)
   {
      return true;
   }

   virtual bool getClientSecret(OAuth1Params& params, string& secret)
   {
      if(strcmp(params["oauth_consumer_key"][0], "9djdj82h48djs9d2") == 0)
      {
         secret = "j289fwua2l3kjfw803";
      }
      return true;
   }

   virtual bool getTokenSecret(
      OAuth1Params& params, OAuth1::TokenType tokenType, string& secret)
   {
      if(tokenType == OAuth1::RequestToken)
      {
         secret = "";
      }
      else if(tokenType == OAuth1::AccessToken)
      {
         secret = "38fjkjewif83j2iuwc983jh";
      }
      return true;
   }
};

static void runOAuth1Tests(TestRunner& tr)
{
   // main group
   tr.group("OAuth");

   OAuth1 oauth1;
   TestProvider provider;
   assertNoException(
      oauth1.setProvider(&provider));

   // signatures group
   tr.group("Signatures");

   tr.test("HMAC-SHA1");
   {
      HttpRequest request(NULL);
      request.getHeader()->setMethod("POST");
      request.getHeader()->setField("Host", "example.com");
      request.getHeader()->setPath("/request");

      OAuth1Params params;
      params->setType(Map);

      // URL Parameters:
      // b5=%3D%253D
      // a3=a
      // c%40=
      // a2=r%20b
      params["b5"]->append("=%3D");
      params["a3"]->append("a");
      params["c@"]->append("");
      params["a2"]->append("r b");

      // Authorization Parameters
      // oauth_consumer_key="9djdj82h48djs9d2"
      // oauth_token="kkk9d7dh3k39sjv7"
      // oauth_signature_method="HMAC-SHA1"
      // oauth_timestamp="137131201"
      // oauth_nonce="7d8f3e4a"
      // oauth_signature="bYT5CMsGcbgUdFHObYMEfcx6bsw%3D"
      params["oauth_consumer_key"]->append("9djdj82h48djs9d2");
      params["oauth_token"]->append("kkk9d7dh3k39sjv7");
      params["oauth_signature_method"]->append("HMAC-SHA1");
      params["oauth_timestamp"]->append("137131201");
      params["oauth_nonce"]->append("7d8f3e4a");

      // POST Parameters
      // c2=
      // a3=2+q
      params["c2"]->append("");
      params["a3"]->append("2 q");

      assertNoException(
         oauth1.sign(&request, params, "HMAC-SHA1", OAuth1::AccessToken));

      string signature = params["oauth_signature"][0]->getString();
      assertStrCmp(signature.c_str(), "SeBklM1Jx9YiZUfs6hzjHzdpd9U=");
   }
   tr.passIfNoException();

   // end signatures group
   tr.ungroup();

   // end main group
   tr.ungroup();
}

static bool run(TestRunner& tr)
{
   if(tr.isDefaultEnabled())
   {
      runOAuth1Tests(tr);
   }
   return true;
}

} // end namespace

MO_TEST_MODULE_FN("monarch.tests.oauth1.test", "1.0", mo_test_oauth1::run)
