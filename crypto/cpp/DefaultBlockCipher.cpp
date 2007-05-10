/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "DefaultBlockCipher.h"
#include "SymmetricKeyFactory.h"

#include <openssl/err.h>

using namespace std;
using namespace db::crypto;
using namespace db::io;

DefaultBlockCipher::DefaultBlockCipher() : AbstractBlockCipher(true)
{
}

DefaultBlockCipher::~DefaultBlockCipher()
{
}

void DefaultBlockCipher::startEncrypting(
   const std::string& algorithm, SymmetricKey** symmetricKey)
throw(db::io::IOException, UnsupportedAlgorithmException)
{
   // create a symmetric key
   SymmetricKeyFactory factory;
   factory.createKey(algorithm, symmetricKey);
   
   // start encrypting
   startEncrypting(*symmetricKey);
}

void DefaultBlockCipher::startEncrypting(SymmetricKey* symmetricKey)
throw(db::io::IOException, UnsupportedAlgorithmException)
{
   // enable encryption mode
   mEncryptMode = true;
   
   // get the cipher function
   mCipherFunction = getCipherFunction(symmetricKey->getAlgorithm());
   
   // get pointers to key and iv
   char* key;
   unsigned int length;
   char* iv;
   symmetricKey->getData(&key, length, &iv);
   
   // initialize encryption
   if(EVP_EncryptInit_ex(
      &mCipherContext, mCipherFunction, NULL,
      (unsigned char*)key, (unsigned char*)iv) != 1)
   {
      throw IOException(
         "Could not start encrypting!",
         ERR_error_string(ERR_get_error(), NULL));
   }
}

void DefaultBlockCipher::startDecrypting(SymmetricKey* symmetricKey)
throw(IOException, UnsupportedAlgorithmException)
{
   // disable encryption mode
   mEncryptMode = false;
   
   // get the cipher function
   mCipherFunction = getCipherFunction(symmetricKey->getAlgorithm());
   
   // get pointers to key and iv
   char* key;
   unsigned int length;
   char* iv;
   symmetricKey->getData(&key, length, &iv);
   
   // initialize decryption
   if(EVP_DecryptInit_ex(
      &mCipherContext, mCipherFunction, NULL,
      (unsigned char*)key, (unsigned char*)iv) != 1)
   {
      throw IOException(
         "Could not start decrypting!",
         ERR_error_string(ERR_get_error(), NULL));
   }
}

void DefaultBlockCipher::update(
   char* in, int inLength, char* out, int& outLength)
throw(IOException)
{
   // only proceed if the cipher function has been set
   if(mCipherFunction != NULL)
   {
      if(isEncryptEnabled())
      {
         // encrypt more data
         if(EVP_EncryptUpdate(
            &mCipherContext, (unsigned char*)out, &outLength,
            (unsigned char*)in, inLength) != 1)
         {
            throw IOException(
               "Could not encrypt data!",
               ERR_error_string(ERR_get_error(), NULL));
         }
      }
      else
      {
         // decrypt more data
         if(EVP_DecryptUpdate(
            &mCipherContext, (unsigned char*)out, &outLength,
            (unsigned char*)in, inLength) != 1)
         {
            throw IOException(
               "Could not decrypt data!",
               ERR_error_string(ERR_get_error(), NULL));
         }
      }
   }
}

void DefaultBlockCipher::finish(char* out, int& length) throw(IOException)
{
   // only proceed if the cipher function has been set
   if(mCipherFunction != NULL)
   {
      if(isEncryptEnabled())
      {
         // finish encrypting
         if(EVP_EncryptFinal_ex(
            &mCipherContext, (unsigned char*)out, &length) != 1)
         {
            throw IOException(
               "Could not finish encrypting!",
               ERR_error_string(ERR_get_error(), NULL));
         }
      }
      else
      {
         // finish decrypting
         if(EVP_DecryptFinal_ex(
            &mCipherContext, (unsigned char*)out, &length) != 1)
         {
            throw IOException(
               "Could not finish decrypting!",
               ERR_error_string(ERR_get_error(), NULL));
         }
      }
   }
}
