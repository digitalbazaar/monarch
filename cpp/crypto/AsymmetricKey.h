/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_crypto_AsymmetricKey_H
#define monarch_crypto_AsymmetricKey_H

#include "monarch/rt/Collectable.h"

#include <openssl/evp.h>
#include <string>

namespace monarch
{
namespace crypto
{

/**
 * An AsymmetricKey is an abstract base class for asymmetric cryptographic
 * keys. It uses OpenSSL's implementation for public and private keys.
 *
 * @author Dave Longley
 */
class AsymmetricKey
{
protected:
   /**
    * The key data structure used to store the public or private key.
    */
   EVP_PKEY* mKey;

public:
   /**
    * Creates a new AsymmetricKey from a PKEY structure.
    *
    * @param pkey the PKEY structure with the data for the key.
    */
   AsymmetricKey(EVP_PKEY* pkey);

   /**
    * Destructs this AsymmetricKey.
    */
   virtual ~AsymmetricKey();

   /**
    * Gets a pointer to the OpenSSL PKEY structure that stores the key data.
    *
    * @return a pointer to the OpenSSL PKEY structure that stores the key data.
    */
   virtual EVP_PKEY* getPKEY();

   /**
    * Gets the algorithm for this key.
    *
    * @return the algorithm for this key.
    */
   virtual const char* getAlgorithm();

   /**
    * Gets the maximum size, in bytes, required to hold a signature or single
    * encryption/decryption using this key.
    *
    * @return the maximum size, in bytes, required to hold a signature or
    *         single encryption/decryption using this key.
    */
   virtual unsigned int getOutputSize();
};

// typedef for a reference-counted AsymmetricKey
typedef monarch::rt::Collectable<AsymmetricKey> AsymmetricKeyRef;

} // end namespace crypto
} // end namespace monarch
#endif
