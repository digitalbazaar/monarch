/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SymmetricKeyFactory.h"
#include "System.h"
#include "Thread.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

using namespace std;
using namespace db::crypto;
using namespace db::rt;

SymmetricKeyFactory::SymmetricKeyFactory()
{
}

SymmetricKeyFactory::~SymmetricKeyFactory()
{
}

bool SymmetricKeyFactory::createRandomKey(
   const std::string& algorithm, SymmetricKey** key)
{
   bool rval = true;
   
   const EVP_CIPHER* cipherType = NULL;
   
   if(algorithm == "AES" || algorithm == "AES256")
   {
      cipherType = EVP_aes_256_cbc();
   }
   else if(algorithm == "AES128")
   {
      cipherType = EVP_aes_128_cbc();
   }
   else if(algorithm == "3DES")   
   {
      cipherType = EVP_des_ede3_cbc();
   }
   else
   {
      // unknown algorithm
      rval = false;
      Thread::setException(new UnsupportedAlgorithmException(
         "Key algorithm '" + algorithm + "' is not supported!"));
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
      (*key)->assignData(data, keyLength, iv, false);
   }
   
   return rval;
}

bool SymmetricKeyFactory::createKey(
   const std::string& algorithm, SymmetricKey** key)
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
