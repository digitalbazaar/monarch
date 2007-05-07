/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "BasicPublicKey.h"

using namespace std;
using namespace db::crypto;

BasicPublicKey::BasicPublicKey(EVP_PKEY* pkey) : AsymmetricKey(pkey)
{
}

BasicPublicKey::~BasicPublicKey()
{
}

DigitalSignature* BasicPublicKey::createSignature()
{
   DigitalSignature* rval = new DigitalSignature(this, false);
   return rval;
}

const string& BasicPublicKey::getAlgorithm()
{
   return AsymmetricKey::getAlgorithm();
}
