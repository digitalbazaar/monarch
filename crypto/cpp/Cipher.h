/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef Cipher_H
#define Cipher_H

#include "Object.h"

namespace db
{
namespace crypto
{

/**
 * The Cipher is an algorithm for encryption and decryption.
 * 
 * When encrypting data, "plaintext" is transformed into "ciphertext". When
 * decrypting data "ciphertext" is transformed into "plaintext." A cipher is
 * used to keep data confidential.
 * 
 * This class uses OpenSSL's implementation of ciphers.
 * 
 * @author Dave Longley
 */
class Cipher : public virtual db::rt::Object
{
protected:
   /**
    * True to encrypt, false to decrypt.
    */
   bool mEncryptMode;
   
   /**
    * Gets the cipher function for this Cipher.
    *
    * @return the cipher function to use.
    */
   virtual const EVP_CIPHER* getCipherFunction();
   
public:
   /**
    * Creates a new Cipher for either encryption or decryption. This Cipher
    * may be reset at any time to switch encryption/decryption modes or to
    * encrypt/decrypt new data.
    * 
    * @param encrypt true to encrypt, false to decrypt.
    */
   Cipher(bool encrypt);
   
   /**
    * Destructs this Cipher.
    */
   virtual ~Cipher();
   
   /**
    * Resets this Cipher so it can be used again with new input.
    * 
    * @param encrypt true to encrypt, false to decrypt.
    */
   virtual void reset(bool encrypt) = 0;
   
   /**
    * Updates the data to encrypt or decrypt. This method can be called
    * repeatedly with chunks of the data that is to be encrypted or decrypted.
    * 
    * @param in a buffer with data to encrypt/decrypt.
    * @param inOffset the offset at which the data begins.
    * @param inLength the length of the data.
    */
   virtual void update(
      const char* in, unsigned int inOffset, unsigned int inLength) = 0;
   
   /**
    * Puts the final chunk of encrypted or decrypted data into an array of
    * bytes.
    * 
    * @param b a buffer to fill with the data.
    * @param length the length of the data.
    */
   virtual void final(char* b, unsigned int& length) = 0;
};

} // end namespace crypto
} // end namespace db
#endif
