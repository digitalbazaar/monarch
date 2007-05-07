/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "BasicPrivateKey.h"

using namespace std;
using namespace db::crypto;

BasicPrivateKey::BasicPrivateKey(EVP_PKEY* pkey) : AsymmetricKey(pkey)
{
}

BasicPrivateKey::~BasicPrivateKey()
{
}

DigitalSignature* BasicPrivateKey::createSignature()
{
   DigitalSignature* rval = new DigitalSignature(this, true);
   return rval;
}

const string& BasicPrivateKey::getAlgorithm()
{
   return AsymmetricKey::getAlgorithm();
}
