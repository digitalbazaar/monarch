/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/DigitalSignature.h"

#include "monarch/crypto/PrivateKey.h"
#include "monarch/crypto/PublicKey.h"

#include <cstring>

using namespace monarch::crypto;

DigitalSignature::DigitalSignature(PrivateKeyRef& key)
{
   // store key
   mKey = key;

   // set sign mode to true
   mSignMode = true;
}

DigitalSignature::DigitalSignature(PublicKeyRef& key)
{
   // store key
   mKey = key;

   // set sign mode to false
   mSignMode = false;
}

DigitalSignature::~DigitalSignature()
{
}

const EVP_MD* DigitalSignature::getHashFunction()
{
   const EVP_MD* rval = NULL;

   if(strcmp(mKey->getAlgorithm(), "DSA") == 0)
   {
      // dss1 is the same as sha1 but there's some weird algorithm
      // linking thing with OpenSSL that currently requires this
      // to be used -- it is supposed to go away eventually
      rval = EVP_dss1();
   }
   else if(strcmp(mKey->getAlgorithm(), "RSA") == 0)
   {
      rval = EVP_sha1();
   }
   else
   {
      // default to SHA1
      rval = EVP_sha1();
   }

   return rval;
}

void DigitalSignature::reset()
{
   // get the hash function for this algorithm
   mHashFunction = getHashFunction();

   // initialize the message digest context (NULL uses the default engine)
   // according to sign mode
   if(mSignMode)
   {
      EVP_SignInit_ex(&mMessageDigestContext, mHashFunction, NULL);
   }
   else
   {
      EVP_VerifyInit_ex(&mMessageDigestContext, mHashFunction, NULL);
   }
}

void DigitalSignature::update(const char* b, unsigned int length)
{
   // if the hash function hasn't been set, then call reset to set it
   if(mHashFunction == NULL)
   {
      reset();
   }

   // update message digest context according to sign mode
   if(mSignMode)
   {
      EVP_SignUpdate(&mMessageDigestContext, b, length);
   }
   else
   {
      EVP_VerifyUpdate(&mMessageDigestContext, b, length);
   }
}

void DigitalSignature::getValue(char* b, unsigned int& length)
{
   // get the final value from the message digest context according
   // to sign mode
   if(mSignMode)
   {
      // if the hash function hasn't been set, then call reset to set it
      if(mHashFunction == NULL)
      {
         reset();
      }

      EVP_SignFinal(
         &mMessageDigestContext, (unsigned char*)b, &length, mKey->getPKEY());
   }
}

unsigned int DigitalSignature::getValueLength()
{
   // get the maximum signature size
   return mKey->getOutputSize();
}

bool DigitalSignature::verify(const char* b, unsigned int length)
{
   bool rval = false;

   if(!mSignMode)
   {
      // if the hash function hasn't been set, then call reset to set it
      if(mHashFunction == NULL)
      {
         reset();
      }

      int error = EVP_VerifyFinal(
         &mMessageDigestContext, (unsigned char*)b, length, mKey->getPKEY());
      if(error == 1)
      {
         rval = true;
      }
   }

   return rval;
}

AsymmetricKeyRef& DigitalSignature::getKey()
{
   return mKey;
}

bool DigitalSignature::getSignMode()
{
   return mSignMode;
}
