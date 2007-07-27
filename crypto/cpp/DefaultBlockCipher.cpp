/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "DefaultBlockCipher.h"
#include "SymmetricKeyFactory.h"
#include "IOException.h"
#include "Thread.h"

#include <openssl/err.h>

using namespace db::crypto;
using namespace db::io;
using namespace db::rt;

DefaultBlockCipher::DefaultBlockCipher() : AbstractBlockCipher(true)
{
}

DefaultBlockCipher::~DefaultBlockCipher()
{
}

bool DefaultBlockCipher::startEncrypting(
   const char* algorithm, SymmetricKey** symmetricKey)
{
   // create a symmetric key
   SymmetricKeyFactory factory;
   factory.createKey(algorithm, symmetricKey);
   
   // start encrypting
   return startEncrypting(*symmetricKey);
}

bool DefaultBlockCipher::startEncrypting(SymmetricKey* symmetricKey)
{
   bool rval = false;
   
   // enable encryption mode
   mEncryptMode = true;
   
   // get the cipher function
   mCipherFunction = getCipherFunction(symmetricKey->getAlgorithm());
   if(mCipherFunction != NULL)
   {
      // get pointers to key and iv
      char* key;
      unsigned int length;
      char* iv;
      unsigned int ivLength;
      symmetricKey->getData(&key, length, &iv, ivLength);
      
      // initialize encryption
      if(EVP_EncryptInit_ex(
         &mCipherContext, mCipherFunction, NULL,
         (unsigned char*)key, (unsigned char*)iv) == 1)
      {
         rval = true;
      }
      else
      {
         Thread::setException(new IOException(
            "Could not start encrypting!",
            ERR_error_string(ERR_get_error(), NULL)));
      }
   }
   
   return rval;
}

bool DefaultBlockCipher::startDecrypting(SymmetricKey* symmetricKey)
{
   bool rval = false;
   
   // disable encryption mode
   mEncryptMode = false;
   
   // get the cipher function
   mCipherFunction = getCipherFunction(symmetricKey->getAlgorithm());
   if(mCipherFunction != NULL)
   {
      // get pointers to key and iv
      char* key;
      unsigned int length;
      char* iv;
      unsigned int ivLength;
      symmetricKey->getData(&key, length, &iv, ivLength);
      
      // initialize decryption
      if(EVP_DecryptInit_ex(
         &mCipherContext, mCipherFunction, NULL,
         (unsigned char*)key, (unsigned char*)iv) == 1)
      {
         rval = true;
      }
      else
      {
         Thread::setException(new IOException(
            "Could not start decrypting!",
            ERR_error_string(ERR_get_error(), NULL)));
      }
   }
   
   return rval;
}

bool DefaultBlockCipher::update(
   char* in, int inLength, char* out, int& outLength)
{
   bool rval = false;
   
   // only proceed if the cipher function has been set
   if(mCipherFunction != NULL)
   {
      if(isEncryptEnabled())
      {
         // encrypt more data
         if(EVP_EncryptUpdate(
            &mCipherContext, (unsigned char*)out, &outLength,
            (unsigned char*)in, inLength) == 1)
         {
            rval = true;
         }
         else
         {
            Thread::setException(new IOException(
               "Could not encrypt data!",
               ERR_error_string(ERR_get_error(), NULL)));
         }
      }
      else
      {
         // decrypt more data
         if(EVP_DecryptUpdate(
            &mCipherContext, (unsigned char*)out, &outLength,
            (unsigned char*)in, inLength) == 1)
         {
            rval = true;
         }
         else
         {
            Thread::setException(new IOException(
               "Could not decrypt data!",
               ERR_error_string(ERR_get_error(), NULL)));
         }
      }
   }
   else
   {
      Thread::setException(new IOException(
         "Cannot update cipher; cipher not started!"));
   }
   
   return rval;
}

bool DefaultBlockCipher::finish(char* out, int& length)
{
   bool rval = false;
   
   // only proceed if the cipher function has been set
   if(mCipherFunction != NULL)
   {
      if(isEncryptEnabled())
      {
         // finish encrypting
         if(EVP_EncryptFinal_ex(
            &mCipherContext, (unsigned char*)out, &length) == 1)
         {
            rval = true;
         }
         else
         {
            Thread::setException(new IOException(
               "Could not finish encrypting!",
               ERR_error_string(ERR_get_error(), NULL)));
         }
      }
      else
      {
         // finish decrypting
         if(EVP_DecryptFinal_ex(
            &mCipherContext, (unsigned char*)out, &length) == 1)
         {
            rval = true;
         }
         else
         {
            Thread::setException(new IOException(
               "Could not finish decrypting!",
               ERR_error_string(ERR_get_error(), NULL)));
         }
      }
   }
   else
   {
      Thread::setException(new IOException(
         "Cannot finish cipher; cipher not started!"));
   }
   
   return rval;
}
