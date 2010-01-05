/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/CryptoHashAlgorithm.h"

using namespace monarch::crypto;

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
