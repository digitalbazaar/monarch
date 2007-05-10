/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SymmetricKeyFactory.h"

#include <openssl/evp.h>

using namespace std;
using namespace db::crypto;

SymmetricKeyFactory::SymmetricKeyFactory()
{
}

SymmetricKeyFactory::~SymmetricKeyFactory()
{
}

void SymmetricKeyFactory::createAes256Key(SymmetricKey** key)
{
   //const EVP_CIPHER* type = EVP_aes_256_cbc();
   
//   // FIXME:
//   char* data = new char[16]{1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
//   
//   unsigned int ivLength = EVP_CIPHER_iv_length(type);
//   char* iv = new char[ivLength];
//   
//   key = new SymmetricKey("AES256");
//   (*key)->setData(data, 16, iv, false);
   
   //EVP_CIPHER_iv_length(cipherType);
   //EVP_aes_256_cbc
}

void SymmetricKeyFactory::createAes128Key(SymmetricKey** key)
{
   //EVP_aes_128_cbc
}

void SymmetricKeyFactory::createTripleDesKey(SymmetricKey** key)
{
   //EVP_des_ede3_cbc
}

void SymmetricKeyFactory::createKey(
   const std::string& algorithm, SymmetricKey** key)
throw(UnsupportedAlgorithmException)
{
   // set key to null
   *key = NULL;
   
   if(algorithm == "AES256")
   {
      // create AES 256-bit key
      createAes256Key(key);
   }
   else if(algorithm == "AES128")
   {
      // create AES 128-bit key
      createAes128Key(key);
   }
   else if(algorithm == "3DES")
   {
      // create Triple DES key
      createTripleDesKey(key);
   }
   else
   {
      // unknown algorithm
      throw UnsupportedAlgorithmException(
         "Key algorithm '" + algorithm + "' is not supported!");
   }
}
