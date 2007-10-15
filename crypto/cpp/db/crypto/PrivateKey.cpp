/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/PrivateKey.h"

using namespace std;
using namespace db::crypto;

PrivateKey::PrivateKey(EVP_PKEY* pkey) : AsymmetricKey(pkey)
{
}

PrivateKey::PrivateKey(const PrivateKey& copy) : AsymmetricKey(NULL)
{
   // create new PKEY structure
   mKey = EVP_PKEY_new();
   
   // copy algorithm
   PrivateKey* pkey = (PrivateKey*)&copy;
   mAlgorithm = strdup(pkey->getAlgorithm());
   
   if(strcmp(mAlgorithm, "DSA") == 0)
   {
      // set dsa key
      EVP_PKEY_set1_DSA(mKey, EVP_PKEY_get1_DSA(pkey->mKey));
   }
   else if(strcmp(mAlgorithm, "RSA") == 0)
   {
      // set rsa key
      EVP_PKEY_set1_RSA(mKey, EVP_PKEY_get1_RSA(pkey->mKey));
   }
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
