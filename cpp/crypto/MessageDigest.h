/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_crypto_MessageDigest_H
#define monarch_crypto_MessageDigest_H

#include "monarch/crypto/CryptoHashAlgorithm.h"
#include "monarch/io/File.h"

#include <string>

namespace monarch
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
    * True if this MessageDigest is persistent, false if not.
    */
   bool mPersistent;

public:
   /**
    * Creates a new MessageDigest that uses the passed hash algorithm. An
    * UnsupportedAlgorithm exception may be set if the given algorithm
    * is not supported.
    *
    * By default, a call to getDigest() or getValue() will finalize the
    * digest and subsequent calls will be considered invalid. This behavior
    * is faster than allowing the digest to persist and be further modified.
    *
    * To allow further modifications to the digest after getting it, this
    * MessageDigest must be initialized as persistent.
    *
    * @param algorithm the hash algorithm to use.
    * @param persistent true if calls to getDigest() or getValue() should not
    *                   finalize the digest, false if they should.
    */
   MessageDigest(const char* algorithm, bool persistent = false);

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

   /**
    * A convenience method for digesting an entire File. This method will
    * update the message digest with the contents from the given file.
    *
    * @param file the file to digest.
    *
    * @return true if successful, false if an error occured.
    */
   virtual bool digestFile(monarch::io::File& file);

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
