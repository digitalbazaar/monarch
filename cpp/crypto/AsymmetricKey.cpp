/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/AsymmetricKey.h"

using namespace std;
using namespace monarch::crypto;

AsymmetricKey::AsymmetricKey(EVP_PKEY* pkey) :
   mKey(pkey)
{
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

const char* AsymmetricKey::getAlgorithm()
{
   const char* rval;

   switch(EVP_PKEY_type(getPKEY()->type))
   {
      case EVP_PKEY_DSA:
         rval = "DSA";
         break;
      case EVP_PKEY_RSA:
         rval = "RSA";
         break;
      default:
         rval = "UNKNOWN";
   }

   return rval;
}

unsigned int AsymmetricKey::getOutputSize()
{
   return EVP_PKEY_size(getPKEY());
}
