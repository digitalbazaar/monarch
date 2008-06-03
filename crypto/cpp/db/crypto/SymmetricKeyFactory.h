/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_crypto_SymmetricKeyFactory_H
#define db_crypto_SymmetricKeyFactory_H

#include "db/crypto/SymmetricKey.h"

namespace db
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
} // end namespace db
#endif
