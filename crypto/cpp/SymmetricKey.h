/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_crypto_SymmetricKey_H
#define db_crypto_SymmetricKey_H

#include <string>

namespace db
{
namespace crypto
{

/**
 * A SymmetricKey is a cryptographic symmetric key. It can be used for
 * encryption and decryption. This key is shared between the sender and
 * receiver of a confidential message.
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
    * The Initialization Vector (IV) for this key, if any.
    */
   char* mIv;
   
   /**
    * The algorithm for this key.
    */
   std::string mAlgorithm;
   
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
   SymmetricKey(const std::string& algorithm);
   
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
    * @param true if the data is encrypted, false if not.
    */
   virtual void assignData(
      char* data, unsigned int length, char* iv, bool encrypted);
   
   /**
    * Sets the data and IV for this key. This method will copy the passed
    * bytes into this key.
    * 
    * The data and IV will be freed when this key is destructed.
    * 
    * @param data the buffer of data.
    * @param length the length, in bytes, of the data.
    * @param iv the buffer of IV data.
    * @param true if the data is encrypted, false if not.
    */
   virtual void setData(
      const char* data, unsigned int length, const char* iv, bool encrypted);
   
   /**
    * Gets the data and data length for this key. This method will provide
    * direct access to the data for this key.
    * 
    * @param data to store the data for this key.
    * @param length to store the data length for this key.
    * @param iv to store the IV for this key.
    */
   virtual void getData(char** data, unsigned int& length, char** iv);
   
   /**
    * Gets the algorithm for this key.
    * 
    * @return the algorithm for this key.
    */
   virtual const std::string& getAlgorithm();
   
   /**
    * Gets whether or not the key data is encrypted.
    * 
    * @return true if the key data is encrypted, false if not.
    */
   virtual bool isEncrypted();
};

} // end namespace crypto
} // end namespace db
#endif
