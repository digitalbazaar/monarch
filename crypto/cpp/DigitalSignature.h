/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef DigitalSignature_H
#define DigitalSignature_H

#include "CryptoHashAlgorithm.h"
#include "AsymmetricKey.h"

namespace db
{
namespace crypto
{

/**
 * A DigitalSignature is used in asymmetric cryptography to authenticate a
 * particular entity. It is a type of cryptographic hash algorithm.
 * 
 * A message that is digitally signed with the message sender's private key
 * can be verified by anyone who has the sender's public key. This ensures
 * that any verified message was sent from the sender and has not been altered.
 * 
 * This class uses OpenSSL's digital signature implementation.
 * 
 * @author Dave Longley
 */
class DigitalSignature : public CryptoHashAlgorithm
{
protected:
   /**
    * The asymmetric key to sign or verify with.
    */
   AsymmetricKey* mKey;
   
   /**
    * Set to true to sign data, false to verify it.
    */
   bool mSignMode;
   
   /**
    * Gets the hash function for this algorithm.
    *
    * @return the hash function to use.
    */
   virtual const EVP_MD* getHashFunction();   
   
public:
   /**
    * Creates a new DigitalSignature that uses the given asymmetric key to
    * sign or verify with.
    * 
    * @param key the AsymmetricKey to use.
    * @param sign true to sign data, false to use verify data. 
    */
   DigitalSignature(AsymmetricKey* key, bool sign);
   
   /**
    * Destructs this DigitalSignature.
    */
   virtual ~DigitalSignature();
   
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
    * Puts the signature into an array of bytes. The length of the signature
    * is dependent on the specific algorithm.
    * 
    * This method will only be effective when this DigitalSignature is in sign
    * mode. To verify a DigitalSignature that is in verify mode, call verify().
    * 
    * @param b a buffer to fill with the hash value bytes.
    * @param length the length of the value.
    */
   virtual void getValue(char* b, unsigned int& length);
   
   /**
    * Gets the maximum length of the signature in bytes.
    * 
    * @return the maximum length of the signature in bytes.
    */
   virtual unsigned int getValueLength();
   
   /**
    * Verifies the passed signature against the generated hash value.
    * 
    * This method will only be effective when this DigitalSignature is in
    * verify mode. To obtain the signature for a DigitalSignature in sign
    * mode, call getValue().
    * 
    * @param b an array of bytes containing the signature.
    * @param length the length of the signature in bytes.
    * 
    * @return true if the signature was verified, false if not.
    */
   virtual bool verify(const char* b, unsigned int length); 
};

} // end namespace crypto
} // end namespace db
#endif
