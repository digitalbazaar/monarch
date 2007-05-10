/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SymmetricKeyFactory.h"

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
   // FIXME:
   //EVP_CIPHER_iv_length(cipherType);
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
   else
   {
      // unknown algorithm
      throw UnsupportedAlgorithmException(
         "Key algorithm '" + algorithm + "' is not supported!");
   }
}
