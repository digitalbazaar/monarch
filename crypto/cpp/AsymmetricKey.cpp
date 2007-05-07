/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "AsymmetricKey.h"

using namespace db::crypto;

AsymmetricKey::AsymmetricKey()
{
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
