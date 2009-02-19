/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_crypto_BlockCipher_H
#define db_crypto_BlockCipher_H

#include "db/io/ByteBuffer.h"

#include <inttypes.h>

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
   BlockCipher();
   
   /**
    * Destructs this BlockCipher.
    */
   virtual ~BlockCipher();
   
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
   virtual bool update(
      const char* in, int inLength, char* out, int& outLength) = 0;
   
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
    * Updates the data that is being encrypted or decrypted. This method can
    * be called repeatedly with chunks of the data that is to be encrypted or
    * decrypted.
    * 
    * The out buffer must be at least inLength + getBlockSize() or it will
    * be resized if permitted. The out buffer's length will be extended to
    * accommodate any bytes written to it.
    * 
    * @param in a buffer with data to encrypt/decrypt.
    * @param inLength the length of the data.
    * @param out a buffer to fill with encrypted/decrypted data.
    * @param resize true to allow resizing of the buffer, false not to.
    * 
    * @return true if no exception occurred, false if not.
    */
   virtual bool update(
      const char* in, int inLength, db::io::ByteBuffer* out, bool resize);
   
   /**
    * Puts the final chunk of encrypted or decrypted data into a ByteBuffer,
    * resizing if necessary, if permitted.
    * 
    * The out buffer must be at least getBlockSize() bytes long or it will
    * be resized if permitted. The out buffer's length will be extended to
    * accommodate any bytes written to it.
    * 
    * @param out a buffer to fill with the data.
    * @param resize true to allow resizing of the buffer, false not to.
    * 
    * @return true if no exception occurred, false if not.
    */
   virtual bool finish(db::io::ByteBuffer* out, bool resize);
   
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
   
   /**
    * Gets the number of input bytes. If encrypting, this is the total size
    * of the unencrypted data. If decrypting, this is the total size of the
    * encrypted data. This number is reset when a new encryption or decryption
    * is started.
    * 
    * @return the total number of input bytes.
    */
   virtual uint64_t getTotalInput() = 0;
   
   /**
    * Gets the number of output bytes. If encrypting, this is the total size
    * of the encrypted data. If decrypting, this is the total size of the
    * decrypted data. This number is reset when a new encryption or decryption
    * is started.
    * 
    * @return the total number of output bytes.
    */
   virtual uint64_t getTotalOutput() = 0;
};

} // end namespace crypto
} // end namespace db
#endif
