/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_crypto_PrivateKey_H
#define monarch_crypto_PrivateKey_H

#include "monarch/crypto/AsymmetricKey.h"
#include "monarch/io/ByteBuffer.h"

namespace monarch
{
namespace crypto
{

/**
 * A PrivateKey is a secret cryptographic asymmetric key.
 *
 * In asymmetric cryptography a pair of keys, one public and one private
 * are used. An entity's public key, as the name suggests, is public and does
 * not contain any confidential data. The entity's private key, however, is
 * confidential and must be kept secret.
 *
 * There are two ways in which asymmetric cryptography can be used to aid
 * in secure communication.
 *
 * 1. Public key encryption - a message can be encrypted using the message
 * recipient's public key so that only the entity may decrypt it using their
 * private key. This ensures that only the recipient can read the secret
 * message.
 *
 * 2. Digital signatures - a message signed with the message sender's private
 * key can be verified by anyone who has the sender's public key. This ensures
 * that any verified message was sent from the sender and has not been altered.
 *
 * This class uses OpenSSL's implementation of private keys.
 *
 * @author Dave Longley
 */
class PrivateKey : public AsymmetricKey
{
public:
   /**
    * Creates a new PrivateKey from a PKEY structure.
    *
    * @param pkey the PKEY structure with the data for the key.
    */
   PrivateKey(EVP_PKEY* pkey);

   /**
    * Destructs this PrivateKey.
    */
   virtual ~PrivateKey();

   /**
    * Dencrypts using this PrivateKey (only applicable to RSA keys).
    *
    * @param data the data to decrypt.
    * @param length the length of the data to decrypt.
    * @param out the output buffer to write the decrypted data to, which must
    *           be getOutputSize() in length.
    *
    * @return the number of decrypted bytes, -1 on failure.
    */
   virtual int decrypt(const char* data, int length, char* out);

   /**
    * Dencrypts using this PrivateKey (only applicable to RSA keys).
    *
    * @param data the data to decrypt.
    * @param length the length of the data to decrypt.
    * @param out the output buffer to write the decrypted data to.
    * @param resize true to resize the output buffer if necessary.
    *
    * @return the number of decrypted bytes, -1 on failure.
    */
   virtual int decrypt(
      const char* data, int length, monarch::io::ByteBuffer* out, bool resize);
};

// define reference counted PrivateKey
class PrivateKeyRef : public AsymmetricKeyRef
{
public:
   PrivateKeyRef(PrivateKey* ptr = NULL) : AsymmetricKeyRef(ptr) {};
   virtual ~PrivateKeyRef() {};

   /**
    * Returns a reference to the PrivateKey.
    *
    * @return a reference to the PrivateKey.
    */
   virtual PrivateKey& operator*()
   {
      return static_cast<PrivateKey&>(AsymmetricKeyRef::operator*());
   }

   /**
    * Returns a pointer to the PrivateKey.
    *
    * @return a pointer to the PrivateKey.
    */
   virtual PrivateKey* operator->()
   {
      return static_cast<PrivateKey*>(AsymmetricKeyRef::operator->());
   }
};

} // end namespace crypto
} // end namespace monarch
#endif
