/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef SymmetricKeyFactory_H
#define SymmetricKeyFactory_H

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
class SymmetricKeyFactory : public virtual db::rt::Object
{
protected:
   /**
    * Creates a random key using the passed algorithm.
    * 
    * @param algorithm the algorithm for the key.
    * @param key a pointer to point at the new SymmetricKey.
    * 
    * @exception UnsupportedAlgorithmException thrown if the passed algorithm
    *            is not supported.
    */
   void createRandomKey(const std::string& algorithm, SymmetricKey** key)
   throw(UnsupportedAlgorithmException);
   
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
    * @exception UnsupportedAlgorithmException thrown if the passed algorithm
    *            is not supported.
    */
   void createKey(std::string const& algorithm, SymmetricKey** key)
   throw(UnsupportedAlgorithmException);
};

} // end namespace crypto
} // end namespace db
#endif
