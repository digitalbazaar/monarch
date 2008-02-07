/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_crypto_MessageDigest_H
#define db_crypto_MessageDigest_H

#include "db/crypto/CryptoHashAlgorithm.h"

#include <string>

namespace db
{
namespace crypto
{

/**
 * A MessageDigest is a type of CryptoHashAlgorithm that "digests" a message
 * and produces a "message digest".
 * 
 * @author Dave Longley
 */
class MessageDigest : public CryptoHashAlgorithm
{
protected:
   /**
    * The algorithm for this MessageDigest.
    */
   const char* mAlgorithm;
   
   /**
    * Gets the hash function for this algorithm.
    *
    * @return the hash function to use.
    */
   virtual const EVP_MD* getHashFunction();

public:
   /**
    * Creates a new MessageDigest that uses the passed hash algorithm. An
    * UnsupportedAlgorithm exception may be set if the given algorithm
    * is not supported.
    * 
    * @param algorithm the hash algorithm to use.
    */
   MessageDigest(const char* algorithm);
   
   /**
    * Destructs this MessageDigest.
    */
   virtual ~MessageDigest();
   
   /**
    * Resets this HashAlgorithm so it can be used again with new input.
    */ 
   virtual void reset();
   
   /**
    * Updates the message to digest with a null-terminated string. This method
    * can be called repeatedly with chunks of the message that is to be
    * digested.
    * 
    * @param str the string to digest.
    */
   virtual void update(const char* str);
   
   /**
    * Updates the data to hash. This method can be called repeatedly with
    * chunks of the data that is to be hashed.
    * 
    * @param b a buffer with data to hash.
    * @param length the length of the data.
    */
   virtual void update(const char* b, unsigned int length);
   
   /**
    * Puts the hash value into an array of bytes. The length of the hash value
    * is dependent on the specific algorithm.
    * 
    * @param b a buffer to fill with the hash value bytes.
    * @param length the length of the value.
    */
   virtual void getValue(char* b, unsigned int& length);
   
   /**
    * Gets the maximum length of the hash value in bytes.
    * 
    * @return the maximum length of the hash value in bytes.
    */
   virtual unsigned int getValueLength();   
      
   /**
    * Gets the message digest as a hexadecimal string.
    * 
    * @return the message digest as a hexadecimal string. 
    */
   virtual std::string getDigest();   
};

} // end namespace crypto
} // end namespace db
#endif
