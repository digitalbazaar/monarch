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
throw(IOException, UnsupportedAlgorithmException)
{
   DigitalEnvelope* rval = new DigitalEnvelope();
   
   try
   {
      // start opening
      rval->startOpening(this, key);
   }
   catch(UnsupportedAlgorithmException &e)
   {
      // delete envelope
      delete rval;
      
      // throw exception
      throw e;
   }
   
   return rval;
}

DigitalSignature* PrivateKey::createSignature()
{
   DigitalSignature* rval = new DigitalSignature(this);
   return rval;
}
