/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "PrivateKey.h"

using namespace std;
using namespace db::crypto;

PrivateKey::PrivateKey(EVP_PKEY* pkey) : AsymmetricKey(pkey)
{
}

PrivateKey::~PrivateKey()
{
}

DigitalSignature* PrivateKey::createSignature()
{
   DigitalSignature* rval = new DigitalSignature(this, true);
   return rval;
}
