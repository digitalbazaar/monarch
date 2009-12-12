/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_crypto_DigitalSignature_H
#define db_crypto_DigitalSignature_H

#include "monarch/crypto/CryptoHashAlgorithm.h"
#include "monarch/crypto/PrivateKey.h"
#include "monarch/crypto/PublicKey.h"

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
   AsymmetricKeyRef mKey;

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
    * Creates a new DigitalSignature that uses the given private key to sign.
    *
    * @param key the PrivateKey to sign with.
    */
   DigitalSignature(PrivateKeyRef& key);

   /**
    * Creates a new DigitalSignature that uses the given public key to verify.
    *
    * @param key the PublicKey to verify with.
    */
   DigitalSignature(PublicKeyRef& key);

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
    * @param length the length of the data.
    */
   virtual void update(const char* b, unsigned int length);

   /**
    * Puts the signature into an array of bytes. The length of the signature
    * is dependent on the specific algorithm.
    *
    * This method will only be effective if this DigitalSignature was created
    * with a PrivateKey. To verify a DigitalSignature it must be created with
    * a PublicKey and verify() must be called.
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
    * This method will only be effective when this DigitalSignature was
    * created with a PublicKey. To obtain the value of a DigitalSignature,
    * it must be created with a PrivateKey and getValue() must be called.
    *
    * @param b an array of bytes containing the signature.
    * @param length the length of the signature in bytes.
    *
    * @return true if the signature was verified, false if not.
    */
   virtual bool verify(const char* b, unsigned int length);

   /**
    * Gets the asymmetric key associated with this DigitalSignature.
    *
    * @return the asymmetric key associated with this DigitalSignature.
    */
   virtual AsymmetricKeyRef& getKey();

   /**
    * Gets the sign/verify mode for this DigitalSignature.
    *
    * @return true if in sign mode, false if in verify mode.
    */
   virtual bool getSignMode();
};

// typedef for a counted reference to a DigitalSignature
typedef monarch::rt::Collectable<DigitalSignature> DigitalSignatureRef;

} // end namespace crypto
} // end namespace db
#endif
