/*
 * Copyright (c) 2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_crypto_HashMac_H
#define monarch_crypto_HashMac_H

#include <openssl/hmac.h>

#include "monarch/crypto/SymmetricKey.h"

namespace monarch
{
namespace crypto
{

/**
 * A HashMac (Hash-based Message Authentication Code) calculates a MAC (message
 * authentication code) using a cryptographic hash algorithm and secret key.
 *
 * A MAC is used to simultaneously verify both the data integrity and
 * authenticity of a message.
 *
 * This implementation is a wrapper for OpenSSL's implementations for HMAC.
 *
 * @author Dave Longley
 */
class HashMac
{
protected:
   /**
    * The hmac context.
    */
   HMAC_CTX mContext;

   /**
    * A pointer to the hash function.
    */
   const EVP_MD* mHashFunction;

   /**
    * The algorithm for this HMAC.
    */
   const char* mAlgorithm;

   /**
    * The secret key to use.
    */
   SymmetricKeyRef mKey;

public:
   /**
    * Creates a new HashMac.
    */
   HashMac();

   /**
    * Destructs this HashMac.
    */
   virtual ~HashMac();

   /**
    * Starts calculating a new MAC using the passed hash algorithm and secret
    * key. An UnsupportedAlgorithm exception may be set if the given algorithm
    * is not supported. Call this method each time you want to calculate
    * a new MAC.
    *
    * @param algorithm the hash algorithm to use.
    * @param key the secret key to use.
    *
    * @return true on success, false on failure with exception set.
    */
   virtual bool start(const char* algorithm, SymmetricKeyRef& key);

   /**
    * Updates the MAC with a null-terminated string. This method can be
    * called repeatedly with chunks of the message that is to be processed.
    *
    * @param str the string to process.
    */
   virtual void update(const char* str);

   /**
    * Updates the MAC with data. This method can be called repeatedly with
    * chunks of the data that is to be hashed.
    *
    * @param b a buffer with data.
    * @param length the length of the data.
    */
   virtual void update(const char* b, unsigned int length);

   /**
    * Puts the MAC value into an array of bytes. The length of the MAC value
    * is dependent on the specific algorithm.
    *
    * @param b a buffer to fill with the MAC value bytes.
    * @param length the length of the value.
    */
   virtual void getValue(char* b, unsigned int& length);

   /**
    * Gets the maximum length of the MAC value in bytes.
    *
    * @return the maximum length of the MAC value in bytes.
    */
   virtual unsigned int getValueLength();

   /**
    * Gets the MAC as a hexadecimal string.
    *
    * @return the MAC as a hexadecimal string.
    */
   virtual std::string getMac();

protected:
   /**
    * Gets the hash function for this algorithm.
    *
    * @return the hash function to use.
    */
   virtual const EVP_MD* getHashFunction();
};

} // end namespace crypto
} // end namespace monarch
#endif
