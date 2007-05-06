/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "CryptoHashAlgorithm.h"

using namespace db::crypto;

CryptoHashAlgorithm::CryptoHashAlgorithm()
{
   // initialize the message digest context
   EVP_MD_CTX_init(&mMessageDigestContext);
   
   // set the hash function to null
   mHashFunction = NULL;
}

CryptoHashAlgorithm::~CryptoHashAlgorithm()
{
   // clean up the message digest context
   EVP_MD_CTX_cleanup(&mMessageDigestContext);
}

void CryptoHashAlgorithm::reset()
{
   // get the hash function for this algorithm
   mHashFunction = getHashFunction();
   
   // initialize the message digest context (NULL uses the default engine)
   EVP_DigestInit_ex(&mMessageDigestContext, mHashFunction, NULL);
}

void CryptoHashAlgorithm::update(
   const char* b, unsigned int offset, unsigned int length)
{
   // if the hash function hasn't been set, then call reset to set it
   if(mHashFunction == NULL)
   {
      reset();
   }
   
   // update message digest context
   EVP_DigestUpdate(&mMessageDigestContext, b + offset, length);
}

void CryptoHashAlgorithm::getValue(char* b)
{
   // if the hash function hasn't been set, then call reset to set it
   if(mHashFunction == NULL)
   {
      reset();
   }
   
   // get the final value from the message digest context
   unsigned int length;
   EVP_DigestFinal_ex(&mMessageDigestContext, (unsigned char*)b, &length);
}

unsigned int CryptoHashAlgorithm::getValueLength()
{
   // if the hash function hasn't been set, then call reset to set it
   if(mHashFunction == NULL)
   {
      reset();
   }
   
   return EVP_MD_size(mHashFunction);
}
