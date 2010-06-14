/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/CryptoHashAlgorithm.h"

using namespace monarch::crypto;

CryptoHashAlgorithm::CryptoHashAlgorithm() :
   mHashFunction(NULL)
{
   // initialize the message digest context
   EVP_MD_CTX_init(&mMessageDigestContext);
}

CryptoHashAlgorithm::~CryptoHashAlgorithm()
{
   // clean up the message digest context
   EVP_MD_CTX_cleanup(&mMessageDigestContext);
}
