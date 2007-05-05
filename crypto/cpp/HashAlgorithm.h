/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef HashAlgorithm_H
#define HashAlgorithm_H

#include <string>

namespace db
{
namespace crypto
{

/**
 * The HashAlgorithm class provides an interface for hash algorithms.
 * 
 * A hash algorithm converts data into a relatively smaller number that can be
 * used as a "digital fingerprint" for the data. A hash algorithm always
 * produces the same output fingerprint for the same input data. The output
 * of the hash algorithm is often referred to as a hash sum, hash value,
 * checksum, or message digest. However, other terms also exist.
 * 
 * A good hash algorithm produces very few, if any, collisions where two
 * different sets of input data results in the same hash sum and runs quickly.
 * 
 * A good cryptographic hash algorithm is one-way, meaning that it is very
 * difficult to invert (determine the input data from the hash sum).
 *  
 * Examples of good cryptographic hash algorithms include the Secure Hash
 * Algorithm (SHA) and the Message Digest Algorithm 5 (MD5).
 * 
 * @author Dave Longley
 */
class HashAlgorithm
{
public:
   /**
    * Creates a new HashAlgorithm.
    */
   HashAlgorithm() {};
   
   /**
    * Destructs this HashAlgorithm.
    */
   virtual ~HashAlgorithm() {};
   
   /**
    * Resets this HashAlgorithm so it can be used again with new input.
    */ 
   virtual void reset() = 0;
   
   /**
    * Updates the data to hash. This method can be called repeatedly with
    * chunks of the data that is to be hashed.
    * 
    * @param b a buffer with data to hash.
    * @param offset the offset at which the data begins.
    * @param length the length of the data.
    */
   virtual void update(
      const char* b, unsigned int offset, unsigned int length) = 0;
   
   /**
    * Puts the hash value into an array of bytes. The length of the hash value
    * is dependent on the specific algorithm.
    * 
    * @param b a buffer to fill with the hash value bytes.
    */
   virtual void getValue(unsigned char* b) = 0;
   
   /**
    * Gets the length of the hash value in bytes.
    * 
    * @return the length of the hash value in bytes.
    */
   virtual unsigned int getValueLength();
   
   /**
    * Gets the hash value as a hexadecimal string.
    * 
    * @return the hash value as a hexadecimal string. 
    */
   virtual std::string getHexValue();
};

} // end namespace crypto
} // end namespace db
#endif
