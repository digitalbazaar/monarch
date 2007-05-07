/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "PublicKey.h"

using namespace std;
using namespace db::crypto;

PublicKey::PublicKey(EVP_PKEY* pkey) : AsymmetricKey(pkey)
{
}

PublicKey::~PublicKey()
{
}

DigitalSignature* PublicKey::createSignature()
{
   DigitalSignature* rval = new DigitalSignature(this, false);
   return rval;
}
