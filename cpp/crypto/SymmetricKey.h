/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_crypto_SymmetricKey_H
#define monarch_crypto_SymmetricKey_H

#include <string>
#include "monarch/rt/Collectable.h"

namespace monarch
{
namespace crypto
{

/**
 * A SymmetricKey is a cryptographic symmetric key. It can be used for
 * encryption and decryption. This key is shared between the sender and
 * receiver of a confidential message.
 *
 * For convenience, an IV associated with a particular encrypted message
 * that this key is used with may be stored along with the key. Also, the
 * key data itself may also be encrypted with another key and it may be
 * flagged as such.
 *
 * @author Dave Longley
 */
class SymmetricKey
{
protected:
   /**
    * The key data for this key.
    */
   char* mData;

   /**
    * The length of the key data.
    */
   unsigned int mDataLength;

   /**
    * The Initialization Vector (IV) associated with the use of this key,
    * if any.
    */
   char* mIv;

   /**
    * The length of the IV.
    */
   unsigned int mIvLength;

   /**
    * The algorithm for this key.
    */
   char* mAlgorithm;

   /**
    * True if this key is encrypted, false if not.
    */
   bool mEncrypted;

   /**
    * Frees the data stored for this key.
    */
   void freeData();

   /**
    * Frees the IV stored for this key.
    */
   void freeIv();

public:
   /**
    * Creates a new, empty, SymmetricKey with the given algorithm.
    *
    * @param algorithm the algorithm to use.
    */
   SymmetricKey(const char* algorithm = NULL);

   /**
    * Destructs this SymmetricKey.
    */
   virtual ~SymmetricKey();

   /**
    * Assigns the data and IV buffers for this key. This method will not copy
    * the passed bytes into this key, it will only set the internal data and
    * IV to point directly at the passed buffers.
    *
    * The data and IV will be freed when this key is destructed.
    *
    * @param data the buffer of data.
    * @param length the length, in bytes, of the data.
    * @param iv the buffer of IV data.
    * @param ivLength the length of the IV data.
    * @param true if the data is encrypted, false if not.
    */
   virtual void assignData(
      char* data, unsigned int length,
      char* iv = NULL, unsigned int ivLength = 0, bool encrypted = false);

   /**
    * Sets the data and IV for this key. This method will copy the passed
    * bytes into this key.
    *
    * The data and IV will be freed when this key is destructed.
    *
    * @param data the buffer of data.
    * @param length the length, in bytes, of the data.
    * @param iv the buffer of IV data.
    * @param ivLength the length of the IV data.
    * @param true if the data is encrypted, false if not.
    */
   virtual void setData(
      const char* data, unsigned int length,
      const char* iv = NULL, unsigned int ivLength = 0,
      bool encrypted = false);

   /**
    * Sets the data and IV for this key using the given hex string.
    *
    * The data and IV will be freed when this key is destructed.
    *
    * @param keyHex the hex string for the key.
    * @param ivHex the hex string for the IV, NULL for none.
    * @param keyLength the length of the hex string, -1 if NULL-terminated.
    * @param ivLength the length of the IV data, -1 if NULL-terminated.
    *
    * @return true if the hex was parsed correctly, false on error.
    */
   virtual bool setHexData(
      const char* keyHex, const char* ivHex = NULL,
      int keyLength = -1, int ivLength = -1);

   /**
    * Gets the data and data length for this key. This method will provide
    * direct access to the data for this key.
    *
    * @param data to store the data for this key.
    * @param length to store the data length for this key.
    * @param iv to store the IV for this key.
    * @param ivLength to store the length of the IV data.
    */
   virtual void getData(
      char** data, unsigned int& length,
      char** iv, unsigned int& ivLength);

   /**
    * Gets the data for this key.
    *
    * @return the data for this key.
    */
   virtual const char* data();

   /**
    * Gets the length of the data for this key.
    *
    * @return the length of the data for this key.
    */
   virtual unsigned int length();

   /**
    * Gets the IV data for this key.
    *
    * @return the IV data for this key.
    */
   virtual const char* iv();

   /**
    * Gets the IV data length for this key.
    *
    * @return the IV data length for this key.
    */
   virtual unsigned int ivLength();

   /**
    * Sets the algorithm for this key.
    *
    * @param algorithm the algorithm for this key.
    */
   virtual void setAlgorithm(const char* algorithm);

   /**
    * Gets the algorithm for this key.
    *
    * @return the algorithm for this key.
    */
   virtual const char* getAlgorithm();

   /**
    * Gets whether or not the key data is encrypted.
    *
    * @return true if the key data is encrypted, false if not.
    */
   virtual bool isEncrypted();
};

// typedef for a reference-counted SymmetricKey
typedef monarch::rt::Collectable<SymmetricKey> SymmetricKeyRef;

} // end namespace crypto
} // end namespace monarch
#endif
