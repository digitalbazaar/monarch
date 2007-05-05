/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef AbstractHashAlgorithm_H
#define AbstractHashAlgorithm_H

#include "HashAlgorithm.h"
#include "Object.h"

#include <openssl/evp.h>

namespace db
{
namespace crypto
{

/**
 * The AbstractHashAlgorithm class provides an abstract base class for
 * HashAlgorithms that uses OpenSSL's implementations for hash algorithms.
 * 
 * @author Dave Longley
 */
class AbstractHashAlgorithm : public virtual db::rt::Object
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
    * Creates a new AbstractHashAlgorithm.
    */
   AbstractHashAlgorithm();
   
   /**
    * Destructs this AbstractHashAlgorithm.
    */
   virtual ~AbstractHashAlgorithm();
   
   /**
    * Resets this HashAlgorithm so it can be used again with new input.
    */ 
   virtual void reset();
   
   /**
    * Updates the data to hash. This method can be called repeatedly with
    * chunks of the data that is to be hashed.
    * 
    * @param b a buffer with data to hash.
    * @param offset the offset at which the data begins.
    * @param length the length of the data.
    */
   virtual void update(const char* b, unsigned int offset, unsigned int length);
   
   /**
    * Puts the hash value into an array of bytes. The length of the hash value
    * is dependent on the specific algorithm.
    * 
    * @param b a buffer to fill with the hash value bytes.
    */
   virtual void getValue(char* b);
   
   /**
    * Gets the length of the hash value in bytes.
    * 
    * @return the length of the hash value in bytes.
    */
   virtual unsigned int getValueLength();
};

} // end namespace crypto
} // end namespace db
#endif
