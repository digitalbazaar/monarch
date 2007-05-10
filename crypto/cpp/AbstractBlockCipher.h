/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef AbstractBlockCipher_H
#define AbstractBlockCipher_H

#include "BlockCipher.h"
#include "Object.h"
#include "UnsupportedAlgorithmException.h"

#include <openssl/evp.h>
#include <string>

namespace db
{
namespace crypto
{

/**
 * The AbstractBlockCipher is an abstract base class for BlockCiphers that use
 * OpenSSL's implementation of block ciphers.
 * 
 * @author Dave Longley
 */
class AbstractBlockCipher : public virtual db::rt::Object, public BlockCipher
{
protected:
   /**
    * True to encrypt, false to decrypt.
    */
   bool mEncryptMode;
   
   /**
    * The cipher context.
    */
   EVP_CIPHER_CTX mCipherContext;
   
   /**
    * A pointer to the cipher function.
    */
   const EVP_CIPHER* mCipherFunction;   
   
   /**
    * Gets the cipher function for this Cipher.
    * 
    * @param algorithm the cipher algorithm. 
    * 
    * @return the cipher function to use.
    */
   virtual const EVP_CIPHER* getCipherFunction(const std::string& algorithm)
   throw(UnsupportedAlgorithmException);
   
public:
   /**
    * Creates a new AbstractBlockCipher for either encryption or decryption.
    * 
    * @param encrypt true to encrypt, false to decrypt.
    */
   AbstractBlockCipher(bool encrypt);
   
   /**
    * Destructs this AbstractBlockCipher.
    */
   virtual ~AbstractBlockCipher();
   
   /**
    * Gets the cipher block size.
    * 
    * @return the cipher block size.
    */
   virtual unsigned int getBlockSize();
   
   /**
    * Gets whether this Cipher is in encrypt or decrypt mode.
    * 
    * @return true if encryption mode is enabled, false if decryption mode is.
    */
   virtual bool isEncryptEnabled();
};

} // end namespace crypto
} // end namespace db
#endif
