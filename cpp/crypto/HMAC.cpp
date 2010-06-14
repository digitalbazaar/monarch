/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/HMAC.h"

#include "monarch/util/Convert.h"
#include "monarch/rt/Exception.h"

#include <cstring>

using namespace std;
using namespace monarch::crypto;
using namespace monarch::rt;
using namespace monarch::util;

HMAC::HMAC() :
   mHashFunction(NULL),
   mAlgorithm(NULL)
{
   // initialize the HMAC context
   HMAC_CTX_init(&mContext);
}

HMAC::~HMAC()
{
   // clean up the HMAC context
   HMAC_CTX_cleanup(&mContext);
}

bool HMAC::start(const char* algorithm, SymmetricKeyRef& key)
{
   bool rval = true;

   if(strcmp(algorithm, "SHA1") == 0)
   {
      mAlgorithm = algorithm;
   }
   else if(strcmp(algorithm, "MD5") == 0)
   {
      mAlgorithm = algorithm;
   }
   else
   {
      // unsupported algorithm
      ExceptionRef e = new Exception(
         "Unsupported hash algorithm.", "monarch.crypto.UnsupportedAlgorithm");
      e->getDetails()["algorithm"] = algorithm;
      Exception::set(e);
      rval = false;
   }

   if(rval)
   {
      // set key, initialize the HMAC context (NULL uses the default engine)
      mKey = key;
      mHashFunction = getHashFunction();
      HMAC_Init_ex(&mContext, key->data(), key->length(), mHashFunction, NULL);
   }

   return rval;
}

void HMAC::update(const char* str)
{
   update(str, strlen(str));
}

void HMAC::update(const char* b, unsigned int length)
{
   // update MAC context
   HMAC_Update(&mContext, (unsigned const char*)b, length);
}

void HMAC::getValue(char* b, unsigned int& length)
{
   // get the final value
   HMAC_Final(&mContext, (unsigned char*)b, &length);
}

unsigned int HMAC::getValueLength()
{
   return EVP_MD_size(mHashFunction);
}

string HMAC::getMAC()
{
   // get the value
   unsigned int length = getValueLength();
   char hashValue[length];
   getValue(hashValue, length);

   // convert the value into hexadecimal
   return Convert::bytesToHex(hashValue, length);
}

const EVP_MD* HMAC::getHashFunction()
{
   const EVP_MD* rval = NULL;

   if(strcmp(mAlgorithm, "SHA1") == 0)
   {
      rval = EVP_sha1();
   }
   else if(strcmp(mAlgorithm, "MD5") == 0)
   {
      rval = EVP_md5();
   }

   return rval;
}
