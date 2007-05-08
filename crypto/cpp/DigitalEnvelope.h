/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef DigitalEnvelope_H
#define DigitalEnvelope_H

#include "Cipher.h"
#include "AsymmetricKey.h"

namespace db
{
namespace crypto
{

/**
 * A DigitalEnvelope is used in cryptography to transport a confidential
 * message.
 * 
 * The following describes how a DigitalEnvelope is used:
 * 
 * A message to be transported is encrypted using a randomly-generated
 * symmetric key because symmetric key algorithms result in much faster
 * encryption and decryption than asymmetric key algorithms. Once the
 * message is encrypted, the recipient's public key is used to encrypt
 * the symmetric key. Upon receiving the message, the recipient uses
 * their private key to decrypt the symmetric key and which they use
 * to decrypt the message. 
 * 
 * The message to be transported may, optionally, be first digitally signed
 * to ensure non-repudiation and authenticity.
 * 
 * This class uses OpenSSL's digital envelope implementation.
 * 
 * @author Dave Longley
 */
class DigitalEnvelope : public Cipher
{
protected:
   /**
    * The asymmetric key to seal or open the envelope with.
    */
   AsymmetricKey* mKey;
   
   /**
    * Gets the cipher function for this Cipher.
    *
    * @return the cipher function to use.
    */
   virtual const EVP_CIPHER* getCipherFunction();
   
public:
   /**
    * Creates a new DigitalEnvelope that uses the given asymmetric key to
    * seal or open with.
    * 
    * @param key the AsymmetricKey to use.
    * @param seal true to seal the envelope, false to use open the envelope. 
    */
   DigitalEnvelope(AsymmetricKey* key, bool seal);
   
   /**
    * Destructs this DigitalEnvelope.
    */
   virtual ~DigitalEnvelope();
   
   /**
    * Resets this DigitalEnvelope so it can be used again with new input.
    * 
    * @param seal true to seal, false to open.
    */
   virtual void reset(bool seal) = 0;
   
   /**
    * Updates the data to seal or open. This method can be called repeatedly
    * with chunks of the data that is to be sealed or opened.
    * 
    * @param in a buffer with data to seal/open.
    * @param inLength the length of the data.
    */
   virtual void update(const char* in, unsigned int inLength) = 0;
   
   /**
    * Puts the final chunk of sealed or opened data into an array of bytes.
    * 
    * @param b a buffer to fill with the data.
    * @param length the length of the data.
    */
   virtual void final(char* b, unsigned int& length) = 0;   
};

} // end namespace crypto
} // end namespace db
#endif
