/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_crypto_SymmetricKeyFactory_H
#define monarch_crypto_SymmetricKeyFactory_H

#include "monarch/crypto/SymmetricKey.h"

namespace monarch
{
namespace crypto
{

/**
 * A SymmetricKeyFactory is used to create symmetric cryptographic keys.
 *
 * @author Dave Longley
 */
class SymmetricKeyFactory
{
protected:
   /**
    * Creates a random key using the passed algorithm.
    *
    * @param algorithm the algorithm for the key.
    * @param key a pointer to a SymmetricKey to populate.
    *
    * @return true if no exception occurred, false if not.
    */
   bool createRandomKey(const char* algorithm, SymmetricKey* key);

public:
   /**
    * Creates a new SymmetricKeyFactory.
    */
   SymmetricKeyFactory();

   /**
    * Destructs this SymmetricKeyFactory.
    */
   virtual ~SymmetricKeyFactory();

   /**
    * Creates a new random key using the given algorithm.
    *
    * @param algorithm the algorithm to use.
    * @param key a pointer to a SymmetricKey to populate.
    *
    * @return true if no exception occurred, false if not.
    */
   bool createKey(const char* algorithm, SymmetricKey* key);
};

} // end namespace crypto
} // end namespace monarch
#endif
