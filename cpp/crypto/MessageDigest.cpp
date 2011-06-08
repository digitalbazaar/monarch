/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/MessageDigest.h"

#include "monarch/io/FileInputStream.h"
#include "monarch/util/Convert.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/Exception.h"

#include <cstring>

using namespace std;
using namespace monarch::crypto;
using namespace monarch::io;
using namespace monarch::rt;
using namespace monarch::util;

MessageDigest::MessageDigest() :
   mAlgorithm(NULL),
   mPersistent(false)
{
}

MessageDigest::~MessageDigest()
{
}

bool MessageDigest::start(const char* algorithm, bool persistent)
{
   bool rval = true;

   const char* algorithms[] =
      {"SHA1", "SHA256", "SHA384", "SHA512", "MD5", NULL};

   mPersistent = persistent;

   // check if algorithm is supported
   mAlgorithm = NULL;
   for(int i = 0; mAlgorithm == NULL && algorithms[i] != NULL; ++i)
   {
      if(strcasecmp(algorithms[i], algorithm) == 0)
      {
         // normalize to static upper case string
         mAlgorithm = algorithms[i];
      }
   }
   if(mAlgorithm == NULL)
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
      // get the hash function for the algorithm, do reset
      mHashFunction = getHashFunction();
      reset();
   }

   return rval;
}

void MessageDigest::reset()
{
   // initialize the message digest context (NULL uses the default engine)
   EVP_DigestInit_ex(&mMessageDigestContext, mHashFunction, NULL);
}

bool MessageDigest::update(const char* str)
{
   update(str, strlen(str));
   return true;
}

bool MessageDigest::update(const char* b, unsigned int length)
{
   // update message digest context
   EVP_DigestUpdate(&mMessageDigestContext, b, length);
   return true;
}

void MessageDigest::getValue(char* b, unsigned int& length)
{
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
   else if(strcmp(mAlgorithm, "SHA256") == 0)
   {
      rval = EVP_sha256();
   }
   else if(strcmp(mAlgorithm, "SHA384") == 0)
   {
      rval = EVP_sha384();
   }
   else if(strcmp(mAlgorithm, "SHA512") == 0)
   {
      rval = EVP_sha512();
   }
   else if(strcmp(mAlgorithm, "MD5") == 0)
   {
      rval = EVP_md5();
   }

   return rval;
}
