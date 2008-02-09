/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/MessageDigest.h"
#include "db/util/Convert.h"
#include "db/rt/Exception.h"

using namespace std;
using namespace db::crypto;
using namespace db::rt;
using namespace db::util;

MessageDigest::MessageDigest(const char* algorithm)
{
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
      int length = 50 + strlen(algorithm);
      char msg[length];
      snprintf(msg, length, "Unsupported hash algorithm '%s'", algorithm);
      ExceptionRef e = new Exception(msg, "db.crypto.UnsupportedAlgorithm");
      Exception::setLast(e, false);
   }
}

MessageDigest::~MessageDigest()
{
}

const EVP_MD* MessageDigest::getHashFunction()
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

void MessageDigest::reset()
{
   // get the hash function for this algorithm
   mHashFunction = getHashFunction();
   
   // initialize the message digest context (NULL uses the default engine)
   EVP_DigestInit_ex(&mMessageDigestContext, mHashFunction, NULL);
}

void MessageDigest::update(const char* str)
{
   update(str, strlen(str));
}

void MessageDigest::update(const char* b, unsigned int length)
{
   // if the hash function hasn't been set, then call reset to set it
   if(mHashFunction == NULL)
   {
      reset();
   }
   
   // update message digest context
   EVP_DigestUpdate(&mMessageDigestContext, b, length);
}

void MessageDigest::getValue(char* b, unsigned int& length)
{
   // if the hash function hasn't been set, then call reset to set it
   if(mHashFunction == NULL)
   {
      reset();
   }
   
   // get the final value from the message digest context
   EVP_DigestFinal_ex(&mMessageDigestContext, (unsigned char*)b, &length);
}

unsigned int MessageDigest::getValueLength()
{
   // if the hash function hasn't been set, then call reset to set it
   if(mHashFunction == NULL)
   {
      reset();
   }
   
   return EVP_MD_size(mHashFunction);
}

string MessageDigest::getDigest()
{
   // get the hash value
   unsigned int length = getValueLength();
   char hashValue[length];
   getValue(hashValue, length);
   
   // convert the hash value into hexadecimal
   return Convert::bytesToHex(hashValue, length);
}
