/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/crypto/PublicKey.h"

using namespace db::crypto;

PublicKey::PublicKey(EVP_PKEY* pkey) : AsymmetricKey(pkey)
{
}

PublicKey::~PublicKey()
{
}
