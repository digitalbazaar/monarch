/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "AbstractPrivateKey.h"

using namespace db::crypto;

AbstractPrivateKey::AbstractPrivateKey()
{
   // allocate the private key structure
   mKey = EVP_PKEY_new();
}

AbstractPrivateKey::~AbstractPrivateKey()
{
   // free the private key structure
   EVP_PKEY_free(mKey);
}
