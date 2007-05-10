/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "PublicKey.h"

using namespace std;
using namespace db::crypto;
using namespace db::io;

PublicKey::PublicKey(EVP_PKEY* pkey) : AsymmetricKey(pkey)
{
}

PublicKey::~PublicKey()
{
}

DigitalEnvelope* PublicKey::createEnvelope(SymmetricKey** key)
throw(IOException)
{
   DigitalEnvelope* rval = new DigitalEnvelope();
   rval->startSealing(this, key);
   return rval;
}

DigitalSignature* PublicKey::createSignature()
{
   DigitalSignature* rval = new DigitalSignature(this);
   return rval;
}
