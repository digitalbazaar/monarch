/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/AsymmetricKey.h"

#include <cstring>

using namespace std;
using namespace db::crypto;

AsymmetricKey::AsymmetricKey(EVP_PKEY* pkey)
{
   // set the public/private key structure
   mKey = pkey;
   mAlgorithm = NULL;
}

AsymmetricKey::~AsymmetricKey()
{
   // free the public/private key structure
   EVP_PKEY_free(mKey);
   
   if(mAlgorithm != NULL)
   {
      free(mAlgorithm);
   }
}

EVP_PKEY* AsymmetricKey::getPKEY()
{
   return mKey;
}

const char* AsymmetricKey::getAlgorithm()
{
   if(mAlgorithm == NULL)
   {
      switch(EVP_PKEY_type(getPKEY()->type))
      {
         case EVP_PKEY_DSA:
            mAlgorithm = strdup("DSA");
            break;
         case EVP_PKEY_RSA:
            mAlgorithm = strdup("RSA");
            break;
         default:
            mAlgorithm = strdup("NONE");
      }
   }
   
   return mAlgorithm;
}

unsigned int AsymmetricKey::getOutputSize()
{
   return EVP_PKEY_size(getPKEY());
}
