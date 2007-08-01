/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "MessageDigest.h"
#include "UnsupportedAlgorithmException.h"
#include "Convert.h"

using namespace std;
using namespace db::crypto;
using namespace db::rt;
using namespace db::util;

MessageDigest::MessageDigest(const string& algorithm)
{
   if(algorithm == "SHA1")
   {
      mAlgorithm = algorithm;
   }
   else if(algorithm == "MD5")
   {
      mAlgorithm = algorithm;
   }
   else
   {
      // unsupported algorithm
      string msg = "Unsupported hash algorithm '" + algorithm + "'";
      Exception::setLast(new UnsupportedAlgorithmException(msg.c_str()));
   }
}

MessageDigest::~MessageDigest()
{
}

const EVP_MD* MessageDigest::getHashFunction()
{
   const EVP_MD* rval = NULL;
   
   if(mAlgorithm == "SHA1")
   {
      rval = EVP_sha1();
   }
   else if(mAlgorithm == "MD5")
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

void MessageDigest::update(const std::string& str)
{
   update(str.c_str(), str.length());
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
