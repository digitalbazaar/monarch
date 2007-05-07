/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef AsymmetricKey_H
#define AsymmetricKey_H

#include "Object.h"
#include "UnsupportedAlgorithmException.h"

#include <openssl/evp.h>
#include <string>

namespace db
{
namespace crypto
{

/**
 * An AsymmetricKey is an abstract base class for asymmetric cryptographic
 * keys. It uses OpenSSL's implementation for public and private keys.
 * 
 * @author Dave Longley
 */
class AsymmetricKey : public virtual db::rt::Object
{
protected:
   /**
    * The key data structure used to store the public or private key.
    */
   EVP_PKEY* mKey;
   
public:
   /**
    * Creates a new AsymmetricKey.
    */
   AsymmetricKey();
   
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
   virtual const std::string& getAlgorithm() = 0;
};

} // end namespace crypto
} // end namespace db
#endif
