/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef MessageDigest_H
#define MessageDigest_H

#include "AbstractHashAlgorithm.h"
#include "UnsupportedAlgorithmException.h"

#include <string>

namespace db
{
namespace crypto
{

/**
 * A MessageDigest is a type of HashAlgorithm that "digests" a message and
 * produces a "message digest".
 * 
 * @author Dave Longley
 */
class MessageDigest : public AbstractHashAlgorithm
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
};

} // end namespace crypto
} // end namespace db
#endif
