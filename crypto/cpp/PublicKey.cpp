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

DigitalEnvelope* PublicKey::createEnvelope(
   const std::string& algorithm, SymmetricKey** key)
throw(IOException, UnsupportedAlgorithmException)
{
   DigitalEnvelope* rval = new DigitalEnvelope();
   
   try
   {
      // start sealing
      rval->startSealing(algorithm, this, key);
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

DigitalSignature* PublicKey::createSignature()
{
   DigitalSignature* rval = new DigitalSignature(this);
   return rval;
}
