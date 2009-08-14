/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef db_crypto_BlockCipherInputStream_H
#define db_crypto_BlockCipherInputStream_H

#include "db/io/FilterInputStream.h"
#include "db/crypto/BlockCipher.h"

namespace db
{
namespace crypto
{

/**
 * A BlockCipherInputStream is used to encrypt or decrypt data as it is
 * read from an underlying input stream.
 *
 * @author Dave Longley
 */
class BlockCipherInputStream : public db::io::FilterInputStream
{
protected:
   /**
    * The BlockCipher to update.
    */
   BlockCipher* mCipher;

   /**
    * True to clean up the BlockCipher when destructing, false not to.
    */
   bool mCleanupCipher;

   /**
    * A read buffer.
    */
   db::io::ByteBuffer mReadBuffer;

   /**
    * Set to true once the current cipher has finished.
    */
   bool mCipherFinished;

public:
   /**
    * Creates a new BlockCipherInputStream that encrypts or decrypts the
    * data filtered from the passed input stream.
    *
    * @param cipher the BlockCipher to use.
    * @param cleanupCipher true to clean up the cipher when destructing,
    *                      false not to.
    * @param is the underlying InputStream to read from.
    * @param cleanupStream true to clean up the passed InputStream when
    *                      destructing, false not to.
    */
   BlockCipherInputStream(
      BlockCipher* cipher, bool cleanupCipher,
      db::io::InputStream* os, bool cleanupStream);

   /**
    * Destructs this BlockCipherInputStream.
    */
   virtual ~BlockCipherInputStream();

   /**
    * Reads some bytes from the stream. This method will block until at least
    * one byte can be read or until the end of the stream is reached. A
    * value of 0 will be returned if the end of the stream has been reached,
    * a value of -1 will be returned if an IO exception occurred, otherwise
    * the number of bytes read will be returned.
    *
    * @param b the array of bytes to fill.
    * @param length the maximum number of bytes to read into the buffer.
    *
    * @return the number of bytes read from the stream or 0 if the end of the
    *         stream has been reached or -1 if an IO exception occurred.
    */
   virtual int read(char* b, int length);

   /**
    * Sets the BlockCipher associated with this stream.
    *
    * @param cipher the BlockCipher to associate with this stream.
    * @param cleanup true to clean up the cipher when destructing,
    *                false not to.
    */
   virtual void setCipher(BlockCipher* cipher, bool cleanup);

   /**
    * Gets the BlockCipher associated with this stream.
    *
    * @return the BlockCipher associated with this stream.
    */
   virtual BlockCipher* getCipher();
};

} // end namespace crypto
} // end namespace db
#endif
