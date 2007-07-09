/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_crypto_DefaultBlockCipher_H
#define db_crypto_DefaultBlockCipher_H

#include "AbstractBlockCipher.h"
#include "SymmetricKey.h"

namespace db
{
namespace crypto
{

/**
 * A DefaultBlockCipher uses a block cipher algorithm to encrypt or
 * decrypt data.
 * 
 * This class uses OpenSSL's block cipher implementation.
 * 
 * @author Dave Longley
 */
class DefaultBlockCipher : public AbstractBlockCipher
{
public:
   /**
    * Creates a new DefaultBlockCipher.
    */
   DefaultBlockCipher();
   
   /**
    * Destructs this DefaultBlockCipher.
    */
   virtual ~DefaultBlockCipher();
   
   /**
    * Starts encrypting with a randomly generated SymmetricKey that is
    * stored in the passed parameter.
    * 
    * The caller of this method must free the generated SymmetricKey.
    * 
    * @param algorithm the algorithm to use to encrypt.
    * @param symmetricKey to store the generated SymmetricKey.
    *
    * @return true if no exception occurred, false if not. 
    */
   virtual bool startEncrypting(
      const std::string& algorithm, SymmetricKey** symmetricKey);
   
   /**
    * Starts encrypting with the given SymmetricKey.
    * 
    * @param symmetricKey to start encrypting with.
    *
    * @return true if no exception occurred, false if not. 
    */
   virtual bool startEncrypting(SymmetricKey* symmetricKey);
   
   /**
    * Starts decrypting with the given SymmetricKey.
    * 
    * @param symmetricKey to start decrypting with.
    * 
    * @return true if no exception occurred, false if not.
    */
   virtual bool startDecrypting(SymmetricKey* symmetricKey);
   
   /**
    * Updates the data that is being encrypted or decrypted. This method can be
    * called repeatedly with chunks of the data that is to be encrypted or
    * decrypted.
    * 
    * The out buffer must be at least inLength + getBlockSize().
    * 
    * @param in a buffer with data to encrypt/decrypt.
    * @param inLength the length of the data.
    * @param out a buffer to fill with encrypt/decrypt data.
    * @param outLength to store the number of bytes put into the output buffer.
    * 
    * @return true if no exception occurred, false if not.
    */
   virtual bool update(char* in, int inLength, char* out, int& outLength);
   
   /**
    * Puts the final chunk of encrypted or decrypted data into an array of
    * bytes.
    * 
    * The out buffer must be at least getBlockSize() bytes long.
    * 
    * @param out a buffer to fill with the data.
    * @param length to store the number of bytes put into the output buffer.
    * 
    * @return true if no exception occurred, false if not.
    */
   virtual bool finish(char* out, int& length);
};

} // end namespace crypto
} // end namespace db
#endif
