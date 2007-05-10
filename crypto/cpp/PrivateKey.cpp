/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "PrivateKey.h"

using namespace std;
using namespace db::crypto;
using namespace db::io;

PrivateKey::PrivateKey(EVP_PKEY* pkey) : AsymmetricKey(pkey)
{
}

PrivateKey::~PrivateKey()
{
}

DigitalEnvelope* PrivateKey::createEnvelope(SymmetricKey* key)
throw(IOException)
{
   DigitalEnvelope* rval = new DigitalEnvelope();
   rval->startOpening(this, key);
   return rval;
}

DigitalSignature* PrivateKey::createSignature()
{
   DigitalSignature* rval = new DigitalSignature(this);
   return rval;
}
