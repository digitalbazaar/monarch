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

DigitalEnvelope* PrivateKey::createEnvelope(SymmetricKey* key)
{
   DigitalEnvelope* rval = new DigitalEnvelope();
   
   // start opening
   if(!rval->startOpening(this, key))
   {
      // open failed, delete envelope
      delete rval;
      rval = NULL;
   }
   
   return rval;
}

DigitalSignature* PrivateKey::createSignature()
{
   DigitalSignature* rval = new DigitalSignature(this);
   return rval;
}
