/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "AsymmetricKey.h"

using namespace std;
using namespace db::crypto;

AsymmetricKey::AsymmetricKey(const string& algorithm)
throw(UnsupportedAlgorithmException)
{
   // make sure the passed algorithm is supported
   if(algorithm == "DSA")
   {
      mAlgorithm = algorithm;
   }
   else if(algorithm == "RSA")
   {
      mAlgorithm = algorithm;
   }
   else
   {
      // unsupported algorithm
      throw UnsupportedAlgorithmException(
         "Unsupported key algorithm '" + algorithm + "'"); 
   }
   
   // allocate the public/private key structure
   mKey = EVP_PKEY_new();
}

AsymmetricKey::~AsymmetricKey()
{
   // free the public/private key structure
   EVP_PKEY_free(mKey);
}

EVP_PKEY* AsymmetricKey::getPKEY()
{
   return mKey;
}

const string& AsymmetricKey::getAlgorithm()
{
   return mAlgorithm;
}
