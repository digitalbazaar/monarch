/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef AsymmetricKey_H
#define AsymmetricKey_H

#include "IOException.h"
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
    * Loads this key from a PEM formatted key. A PEM formatted key is just
    * the base64-encoded version of an ASN.1 DER-encoded key structure
    * that has a header and footer.
    * 
    * @param pem the PEM string to load this key from.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual void loadFromPem(const std::string& pem)
   throw(db::io::IOException) = 0;
   
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
