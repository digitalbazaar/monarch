/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SymmetricKeyFactory.h"
#include "System.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

using namespace db::crypto;
using namespace db::rt;

SymmetricKeyFactory::SymmetricKeyFactory()
{
}

SymmetricKeyFactory::~SymmetricKeyFactory()
{
}

bool SymmetricKeyFactory::createRandomKey(
   const char* algorithm, SymmetricKey** key)
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
      rval = false;
      char* msg = new char[15 + strlen(algorithm) + 19 + 1];
      sprintf(msg, "Key algorithm '%s' is not supported!", algorithm);
      Exception::setLast(new UnsupportedAlgorithmException(msg));
      delete msg;
   }
   
   if(rval)
   {
      // get random bytes for data
      unsigned int keyLength = EVP_CIPHER_key_length(cipherType);
      char *data = new char[keyLength];
      RAND_bytes((unsigned char*)data, keyLength);
      
      // get random bytes for IV
      char* iv = NULL;
      unsigned int ivLength = EVP_CIPHER_iv_length(cipherType);
      if(ivLength > 0)
      {
         iv = new char[ivLength];
         RAND_bytes((unsigned char*)iv, ivLength);
      }
      
      // create symmetric key and assign key data/IV
      *key = new SymmetricKey("AES256");
      (*key)->assignData(data, keyLength, iv, ivLength, false);
   }
   
   return rval;
}

bool SymmetricKeyFactory::createKey(const char* algorithm, SymmetricKey** key)
{
   // set key to null
   *key = NULL;
   
   // add random bytes from the time
   struct timeval tv;
   gettimeofday(&tv, 0);
   RAND_add(&tv, sizeof(tv), 0.0);
   
   // create random key
   return createRandomKey(algorithm, key);
}
