/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef AbstractPrivateKey_H
#define AbstractPrivateKey_H

#include "Object.h"
#include "PrivateKey.h"

#include <openssl/evp.h>

namespace db
{
namespace crypto
{

/**
 * An AbstractPrivateKey is an abstract base class for cryptographic
 * private keys. It uses OpenSSL's implementation for private keys.
 * 
 * @author Dave Longley
 */
class AbstractPrivateKey : public virtual db::rt::Object, public PrivateKey
{
protected:
   /**
    * The private key data structure used to store the private key.
    */
   EVP_PKEY* mKey;
   
public:
   /**
    * Creates a new AbstractPrivateKey.
    */
   AbstractPrivateKey();
   
   /**
    * Destructs this AbstractPrivateKey.
    */
   virtual ~AbstractPrivateKey();
};

} // end namespace crypto
} // end namespace db
#endif
