/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef BlockCipher_H
#define BlockCipher_H

namespace db
{
namespace crypto
{

/**
 * The BlockCipher is a symmetric key cipher that uses a fixed block size
 * for encryption and decryption.
 * 
 * When encrypting data, "plaintext" is transformed into "ciphertext". When
 * decrypting data "ciphertext" is transformed into "plaintext." A cipher is
 * used to keep data confidential.
 * 
 * @author Dave Longley
 */
class BlockCipher
{
public:
   /**
    * Creates a new BlockCipher.
    */
   BlockCipher() {};
   
   /**
    * Destructs this BlockCipher.
    */
   virtual ~BlockCipher() {};
   
   /**
    * Updates the data that is being encrypted or decrypted. This method can
    * be called repeatedly with chunks of the data that is to be encrypted or
    * decrypted.
    * 
    * The out buffer must be at least inLength + getBlockSize().
    * 
    * @param in a buffer with data to encrypt/decrypt.
    * @param inLength the length of the data.
    * @param out a buffer to fill with encrypted/decrypted data.
    * @param outLength to store the number of bytes put into the output buffer.
    * 
    * @return true if no exception occurred, false if not.
    */
   virtual bool update(char* in, int inLength, char* out, int& outLength) = 0;
   
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
   virtual bool finish(char* out, int& length) = 0;
   
   /**
    * Gets the cipher block size.
    * 
    * @return the cipher block size.
    */
   virtual unsigned int getBlockSize() = 0;
   
   /**
    * Gets whether this BlockCipher is in encrypt or decrypt mode.
    * 
    * @return true if encryption mode is enabled, false if decryption mode is.
    */
   virtual bool isEncryptEnabled() = 0;
};

} // end namespace crypto
} // end namespace db
#endif
