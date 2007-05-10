/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef DefaultBlockCipher_H
#define DefaultBlockCipher_H

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
    * @exception IOException thrown if an IO error occurs.
    * @exception UnsupportedAlgorithmException thrown if an unsupported key
    *            algorithm is used.
    */
   virtual void startEncrypting(
      const std::string& algorithm, SymmetricKey** symmetricKey)
   throw(db::io::IOException, UnsupportedAlgorithmException);
   
   /**
    * Starts encrypting with the given SymmetricKey.
    * 
    * @param symmetricKey to start encrypting with.
    * 
    * @exception IOException thrown if an IO error occurs.
    * @exception UnsupportedAlgorithmException thrown if an unsupported key
    *            algorithm is used.
    */
   virtual void startEncrypting(SymmetricKey* symmetricKey)
   throw(db::io::IOException, UnsupportedAlgorithmException);
   
   /**
    * Starts decrypting with the given SymmetricKey.
    * 
    * @param symmetricKey to start decrypting with.
    * 
    * @exception IOException thrown if an IO error occurs.
    * @exception UnsupportedAlgorithmException thrown if an unsupported key
    *            algorithm is used.
    */
   virtual void startDecrypting(SymmetricKey* symmetricKey)
   throw(db::io::IOException, UnsupportedAlgorithmException);
   
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
    * @exception IOException thrown if an IO error occurs.
    */
   virtual void update(char* in, int inLength, char* out, int& outLength)
   throw(db::io::IOException);
   
   /**
    * Puts the final chunk of encrypted or decrypted data into an array of
    * bytes.
    * 
    * The out buffer must be at least getBlockSize() bytes long.
    * 
    * @param out a buffer to fill with the data.
    * @param length to store the number of bytes put into the output buffer.
    * 
    * @exception IOException thrown if an IO error occurs.
    */
   virtual void finish(char* out, int& length) throw(db::io::IOException);
};

} // end namespace crypto
} // end namespace db
#endif
