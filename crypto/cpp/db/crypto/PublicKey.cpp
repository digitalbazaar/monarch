/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/PublicKey.h"

#include <cstring>

using namespace std;
using namespace db::crypto;

PublicKey::PublicKey(EVP_PKEY* pkey) : AsymmetricKey(pkey)
{
}

PublicKey::PublicKey(const PublicKey& copy) : AsymmetricKey(NULL)
{
   // create new PKEY structure
   mKey = EVP_PKEY_new();
   
   // copy algorithm
   PublicKey* pkey = (PublicKey*)&copy;
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

PublicKey::~PublicKey()
{
}

DigitalEnvelope* PublicKey::createEnvelope(
   const char* algorithm, SymmetricKey* key)
{
   DigitalEnvelope* rval = new DigitalEnvelope();
   
   // start sealing
   if(!rval->startSealing(algorithm, this, key))
   {
      // seal failed, delete envelope
      delete rval;
      rval = NULL;
   }
   
   return rval;
}

DigitalSignature* PublicKey::createSignature()
{
   DigitalSignature* rval = new DigitalSignature(this);
   return rval;
}
