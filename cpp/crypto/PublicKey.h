/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_crypto_PublicKey_H
#define monarch_crypto_PublicKey_H

#include "monarch/crypto/AsymmetricKey.h"
#include "monarch/io/ByteBuffer.h"

namespace monarch
{
namespace crypto
{

/**
 * A PublicKey is a non-secret cryptographic asymmetric key.
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
 * This class uses OpenSSL's implementation of public keys.
 *
 * @author Dave Longley
 */
class PublicKey : public AsymmetricKey
{
public:
   /**
    * Creates a new PublicKey from a PKEY structure.
    *
    * @param pkey the PKEY structure with the data for the key.
    */
   PublicKey(EVP_PKEY* pkey);

   /**
    * Destructs this PublicKey.
    */
   virtual ~PublicKey();

   /**
    * Encrypts using this PublicKey (only applicable to RSA keys).
    *
    * @param data the data to encrypt.
    * @param length the length of the data to encrypt.
    * @param out the output buffer to write the encrypted data to, which must
    *           be getOutputSize() in length.
    *
    * @return the number of encrypted bytes, -1 on failure.
    */
   virtual int encrypt(const char* data, int length, char* out);

   /**
    * Encrypts using this PublicKey (only applicable to RSA keys).
    *
    * @param data the data to encrypt.
    * @param length the length of the data to encrypt.
    * @param out the output buffer to write the encrypted data to.
    * @param resize true to resize the output buffer if necessary.
    *
    * @return the number of encrypted bytes, -1 on failure.
    */
   virtual int encrypt(
      const char* data, int length, monarch::io::ByteBuffer* out, bool resize);
};

class PublicKeyRef : public AsymmetricKeyRef
{
public:
   PublicKeyRef(PublicKey* ptr = NULL) : AsymmetricKeyRef(ptr) {};
   virtual ~PublicKeyRef() {};

   /**
    * Returns a reference to the PublicKey.
    *
    * @return a reference to the PublicKey.
    */
   virtual PublicKey& operator*()
   {
      return static_cast<PublicKey&>(AsymmetricKeyRef::operator*());
   }

   /**
    * Returns a pointer to the PublicKey.
    *
    * @return a pointer to the PublicKey.
    */
   virtual PublicKey* operator->()
   {
      return static_cast<PublicKey*>(AsymmetricKeyRef::operator->());
   }
};

} // end namespace crypto
} // end namespace monarch
#endif
