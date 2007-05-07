/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "BasicPrivateKey.h"

using namespace std;
using namespace db::crypto;
using namespace db::io;

BasicPrivateKey::BasicPrivateKey()
{
   // no algorithm set yet
   mAlgorithm = "";
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
