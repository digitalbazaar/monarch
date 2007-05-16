/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "AsymmetricKey.h"

using namespace std;
using namespace db::crypto;

AsymmetricKey::AsymmetricKey(EVP_PKEY* pkey)
{
   // set the public/private key structure
   mKey = pkey;
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
   if(mAlgorithm.length() == 0)
   {
      switch(EVP_PKEY_type(getPKEY()->type))
      {
         case EVP_PKEY_DSA:
            mAlgorithm = "DSA";
            break;
         case EVP_PKEY_RSA:
            mAlgorithm = "RSA";
            break;
         default:
            mAlgorithm = "NONE";
      }
   }
   
   return mAlgorithm;
}

unsigned int AsymmetricKey::getOutputSize()
{
   return EVP_PKEY_size(getPKEY());
}
