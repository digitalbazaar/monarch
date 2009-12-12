/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_crypto_DigitalEnvelope_H
#define monarch_crypto_DigitalEnvelope_H

#include "monarch/crypto/AbstractBlockCipher.h"
#include "monarch/crypto/PrivateKey.h"
#include "monarch/crypto/PublicKey.h"
#include "monarch/crypto/SymmetricKey.h"

namespace monarch
{
namespace crypto
{

// forward declare private and public keys
class PrivateKey;
class PublicKey;

/**
 * A DigitalEnvelope is used in cryptography to transport a confidential
 * message.
 *
 * The following describes how a DigitalEnvelope is used:
 *
 * A message to be transported is encrypted using a randomly-generated
 * symmetric key because symmetric key algorithms result in much faster
 * encryption and decryption than asymmetric key algorithms. Once the
 * message is encrypted with the symmetric key, the recipient's public key
 * is used to encrypt the symmetric key.
 *
 * Upon receiving the message, the recipient uses their private key to
 * decrypt the symmetric key and which they use to decrypt the message.
 *
 * The message to be transported may, optionally, be first digitally signed
 * to ensure non-repudiation and authenticity.
 *
 * This class uses OpenSSL's digital envelope implementation.
 *
 * @author Dave Longley
 */
class DigitalEnvelope : public AbstractBlockCipher
{
protected:
   /**
    * Stores a copy of the key used to seal/open this envelope.
    */
   AsymmetricKeyRef mKey;

public:
   /**
    * Creates a new DigitalEnvelope.
    */
   DigitalEnvelope();

   /**
    * Destructs this DigitalEnvelope.
    */
   virtual ~DigitalEnvelope();

   /**
    * Starts sealing this DigitalEnvelope by using the given public key to
    * encrypt a randomly generated symmetric key that will be used to seal
    * the envelope.
    *
    * This method must be called before calling update() or finish() or
    * else those methods will have no effect.
    *
    * This method can be called multiple times to seal multiple messages.
    *
    * @param algorithm the algorithm to use for the encryption.
    * @param publicKey the PublicKey to encrypt the symmetric key with.
    * @param symmetricKey to store the encrypted SymmetricKey.
    *
    * @return true if no exception occurred, false if not.
    */
   virtual bool startSealing(
      const char* algorithm,
      PublicKeyRef& publicKey, SymmetricKey* symmetricKey);

   /**
    * Starts sealing this DigitalEnvelope by using the given array of public
    * keys to encrypt a randomly generated symmetric key that will be that
    * will be used to seal the envelope.
    *
    * Each public key in the passed array of PublicKey pointers encrypts a
    * a generated symmetric key and then assigns its encrypted data to
    * the SymmetricKey pointed at in the passed array of SymmetricKey pointers.
    *
    * The passed array of symmetric keys must have pointers that point at
    * valid SymmetricKeys to assign data to.
    *
    * This allows for a single message to be quickly encrypted and enveloped
    * for transport to multiple recipients. Each recipient receives their
    * own copy of the encrypted symmetric key which only they can decrypt
    * with their private key.
    *
    * This method must be called before calling update() or finish() or
    * else those methods will have no effect.
    *
    * This method can be called multiple times to seal multiple messages.
    *
    * @param algorithm the algorithm to use for the encryption.
    * @param publicKey the PublicKey to encrypt the symmetric key with.
    * @param symmetricKeys to store the encrypted SymmetricKeys.
    * @param keys the number of keys.
    *
    * @return true if no exception occurred, false if not.
    */
   virtual bool startSealing(
      const char* algorithm,
      PublicKey** publicKeys, SymmetricKey** symmetricKeys, int keys);

   /**
    * Starts opening this DigitalEnvelope by using the given private key to
    * decrypt the given symmetric key that will be used to open the envelope.
    *
    * This method must be called before calling update() or finish() or
    * else those methods will have no effect.
    *
    * This method can be called multiple times to open multiple messages.
    *
    * @param privateKey the PrivateKey to decrypt the symmetric key with.
    * @param symmetricKey the SymmetricKey to open the envelope with.
    *
    * @return true if no exception occurred, false if not.
    */
   virtual bool startOpening(
      PrivateKeyRef& privateKey, SymmetricKey* symmetricKey);

   /**
    * Updates the data that is being sealed or opened. This method can be
    * called repeatedly with chunks of the data that is to be sealed or opened.
    *
    * The out buffer must be at least inLength + getBlockSize().
    *
    * @param in a buffer with data to seal/open.
    * @param inLength the length of the data.
    * @param out a buffer to fill with sealed/opened data.
    * @param outLength to store the number of bytes put into the output buffer.
    *
    * @return true if no exception occurred, false if not.
    */
   virtual bool update(const char* in, int inLength, char* out, int& outLength);

   /**
    * Puts the final chunk of sealed or opened data into an array of bytes.
    *
    * The out buffer must be at least getBlockSize() bytes long.
    *
    * @param out a buffer to fill with the data.
    * @param length to store the number of bytes put into the output buffer.
    *
    * @return true if no exception occurred, false if not.
    */
   virtual bool finish(char* out, int& length);

   // use update/finish from AbstractBlockCipher
   using AbstractBlockCipher::update;
   using AbstractBlockCipher::finish;

   /**
    * Gets the number of input bytes. If sealing, this is the total size
    * of the unsealed data. If opening, this is the total size of the sealed
    * data. This number is reset when a new seal or open is started.
    *
    * @return the total number of input bytes.
    */
   virtual uint64_t getTotalInput();

   /**
    * Gets the number of output bytes. If sealing, this is the total size
    * of the sealed data. If opening, this is the total size of the opened
    * data. This number is reset when a new seal or open is started.
    *
    * @return the total number of output bytes.
    */
   virtual uint64_t getTotalOutput();
};

} // end namespace crypto
} // end namespace monarch
#endif
