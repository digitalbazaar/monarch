/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef MessageDigest_H
#define MessageDigest_H

#include "CryptoHashAlgorithm.h"
#include "UnsupportedAlgorithmException.h"

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
   std::string mAlgorithm;
   
   /**
    * Gets the hash function for this algorithm.
    *
    * @return the hash function to use.
    */
   virtual const EVP_MD* getHashFunction();

public:
   /**
    * Creates a new MessageDigest that uses the passed hash algorithm.
    * 
    * @param algorithm the hash algorithm to use.
    * 
    * @exception UnsupportedAlgorithmException thrown if the passed algorithm
    *            is not recognized or implemented.
    */
   MessageDigest(const std::string& algorithm)
   throw(UnsupportedAlgorithmException);
   
   /**
    * Destructs this MessageDigest.
    */
   virtual ~MessageDigest();
   
   /**
    * Updates the message to digest with a string. This method can be called
    * repeatedly with chunks of the message that is to be digested.
    * 
    * @param str the string to digest.
    */
   virtual void updateMessage(const std::string& str);   
   
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
