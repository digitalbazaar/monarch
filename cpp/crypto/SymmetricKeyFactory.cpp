/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/SymmetricKeyFactory.h"

#include "monarch/rt/System.h"
#include "monarch/rt/DynamicObject.h"
#include "monarch/rt/Exception.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#include <cstring>

using namespace monarch::crypto;
using namespace monarch::rt;

SymmetricKeyFactory::SymmetricKeyFactory()
{
}

SymmetricKeyFactory::~SymmetricKeyFactory()
{
}

bool SymmetricKeyFactory::createRandomKey(
   const char* algorithm, SymmetricKey* key)
{
   bool rval = true;

   const EVP_CIPHER* cipherType = NULL;

   if(strcmp(algorithm, "AES") == 0 || strcmp(algorithm, "AES256") == 0)
   {
      cipherType = EVP_aes_256_cbc();
   }
   else if(strcmp(algorithm, "AES128") == 0)
   {
      cipherType = EVP_aes_128_cbc();
   }
   else if(strcmp(algorithm, "3DES") == 0)
   {
      cipherType = EVP_des_ede3_cbc();
   }
   else
   {
      // unknown algorithm
      ExceptionRef e = new Exception(
         "Key algorithm is not supported.",
         "monarch.crypto.UnsupportedAlgorithm");
      e->getDetails()["algorithm"] = algorithm;
      Exception::set(e);
      rval = false;
   }

   if(rval)
   {
      // get random bytes for data
      unsigned int keyLength = EVP_CIPHER_key_length(cipherType);
      char *data = (char*)malloc(keyLength);
      RAND_bytes((unsigned char*)data, keyLength);

      // get random bytes for IV
      char* iv = NULL;
      unsigned int ivLength = EVP_CIPHER_iv_length(cipherType);
      if(ivLength > 0)
      {
         iv = (char*)malloc(ivLength);
         RAND_bytes((unsigned char*)iv, ivLength);
      }

      // create symmetric key and assign key data/IV
      key->setAlgorithm("AES256");
      key->assignData(data, keyLength, iv, ivLength, false);
   }

   return rval;
}

bool SymmetricKeyFactory::createKey(const char* algorithm, SymmetricKey* key)
{
   // add random bytes from the time
   struct timeval tv;
   gettimeofday(&tv, 0);
   RAND_add(&tv, sizeof(tv), 0.0);

   // create random key
   return createRandomKey(algorithm, key);
}
