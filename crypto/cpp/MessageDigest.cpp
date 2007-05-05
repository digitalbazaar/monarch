/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "MessageDigest.h"
#include "Convert.h"

using namespace std;
using namespace db::crypto;
using namespace db::util;

MessageDigest::MessageDigest(const string& algorithm)
throw(UnsupportedAlgorithmException)
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
      throw UnsupportedAlgorithmException(
         "Unsupported algorithm '" + algorithm + "'"); 
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

void MessageDigest::updateMessage(const std::string& str)
{
   update(str.c_str(), 0, str.length());
}

string MessageDigest::getDigest()
{
   // get the hash value
   unsigned int length = getValueLength();
   char hashValue[length];
   getValue(hashValue);
   
   // convert the hash value into hexadecimal
   return Convert::bytesToHex(hashValue, 0, length);
}
