/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_crypto_SymmetricKeyFactory_H
#define db_crypto_SymmetricKeyFactory_H

#include "SymmetricKey.h"
#include "UnsupportedAlgorithmException.h"

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
    * @param key a pointer to point at the new SymmetricKey.
    * 
    * @return true if no exception occurred, false if not.
    */
   bool createRandomKey(const std::string& algorithm, SymmetricKey** key);
   
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
    * The caller of this method is responsible for freeing the generated
    * SymmetricKey.
    * 
    * @param algorithm the algorithm to use.
    * @param key a pointer to point at the new SymmetricKey.
    * 
    * @return true if no exception occurred, false if not.
    */
   bool createKey(std::string const& algorithm, SymmetricKey** key);
};

} // end namespace crypto
} // end namespace db
#endif
