/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/MessageDigest.h"

#include "db/io/FileInputStream.h"
#include "db/util/Convert.h"
#include "db/rt/DynamicObject.h"
#include "db/rt/Exception.h"

#include <cstring>

using namespace std;
using namespace db::crypto;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

MessageDigest::MessageDigest(const char* algorithm, bool persistent)
{
   mPersistent = persistent;
   
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
         "Unsupported hash algorithm.", "db.crypto.UnsupportedAlgorithm");
      e->getDetails()["algorithm"] = algorithm;
      Exception::setLast(e, false);
   }
}

MessageDigest::~MessageDigest()
{
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
   
   if(mPersistent)
   {
      // get the final value from a copy so the context can continue to be used
      EVP_MD_CTX copy;
      EVP_MD_CTX_init(&copy);
      EVP_MD_CTX_copy_ex(&copy, &mMessageDigestContext);
      EVP_DigestFinal_ex(&copy, (unsigned char*)b, &length);
      EVP_MD_CTX_cleanup(&copy);
   }
   else
   {
      // get the final value directly from the message digest context
      EVP_DigestFinal_ex(&mMessageDigestContext, (unsigned char*)b, &length);
   }
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

bool MessageDigest::digestFile(File& file)
{
   FileInputStream fis(file);
   const unsigned int bufsize = 2048;
   char* buf = (char*)malloc(bufsize);
   int numBytes;
   while((numBytes = fis.read(buf, bufsize)) > 0)
   {
      update(buf, numBytes);
   }
   fis.close();
   free(buf);
   return (numBytes == 0);
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
