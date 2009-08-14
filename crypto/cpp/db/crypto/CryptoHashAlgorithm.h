/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_crypto_CryptoHashAlgorithm_H
#define db_crypto_CryptoHashAlgorithm_H

#include "db/util/HashAlgorithm.h"

#include <openssl/evp.h>

namespace db
{
namespace crypto
{

/**
 * The CryptoHashAlgorithm class provides an abstract base class for
 * cryptographic HashAlgorithms. It uses OpenSSL's implementations for
 * crypographic hash algorithms.
 *
 * @author Dave Longley
 */
class CryptoHashAlgorithm : public db::util::HashAlgorithm
{
protected:
   /**
    * The message digest context.
    */
   EVP_MD_CTX mMessageDigestContext;

   /**
    * A pointer to the hash function.
    */
   const EVP_MD* mHashFunction;

   /**
    * Gets the hash function for this algorithm.
    *
    * @return the hash function to use.
    */
   virtual const EVP_MD* getHashFunction() = 0;

public:
   /**
    * Creates a new CryptoHashAlgorithm.
    */
   CryptoHashAlgorithm();

   /**
    * Destructs this CryptoHashAlgorithm.
    */
   virtual ~CryptoHashAlgorithm();
};

} // end namespace crypto
} // end namespace db
#endif
