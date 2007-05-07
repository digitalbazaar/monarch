/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "BasicPublicKey.h"

using namespace std;
using namespace db::crypto;
using namespace db::io;

BasicPublicKey::BasicPublicKey()
{
   // no algorithm set yet
   mAlgorithm = "";
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
   if(mAlgorithm.length() == 0)
   {
      if(getPKEY()->type == EVP_PKEY_DSA)
      {
         mAlgorithm = "DSA";
      }
      else if(getPKEY()->type == EVP_PKEY_RSA)
      {
         mAlgorithm = "RSA";
      }
   }
   
   return mAlgorithm;
}
