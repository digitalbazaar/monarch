/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/DigitalEnvelope.h"
#include "db/crypto/PrivateKey.h"
#include "db/crypto/PublicKey.h"
#include "db/crypto/SymmetricKeyFactory.h"
#include "db/io/IOException.h"

#include <openssl/err.h>

using namespace std;
using namespace db::crypto;
using namespace db::io;
using namespace db::rt;

DigitalEnvelope::DigitalEnvelope() : AbstractBlockCipher(true)
{
}

DigitalEnvelope::~DigitalEnvelope()
{
}

bool DigitalEnvelope::startSealing(
   const char* algorithm, PublicKey* publicKey, SymmetricKey** symmetricKey)
{
   // use just a single public key
   return startSealing(algorithm, &publicKey, symmetricKey, 1);
}

bool DigitalEnvelope::startSealing(
   const char* algorithm, PublicKey** publicKeys,
   SymmetricKey** symmetricKeys, unsigned int keys)
{
   bool rval = false;
   
   // enable encryption mode
   mEncryptMode = true;
   
   // get the cipher function
   mCipherFunction = getCipherFunction(algorithm);
   if(mCipherFunction != NULL)
   {
      // create symmetric key buffers for each public key
      EVP_PKEY* pKeys[keys];
      char* eKeys[keys];
      int eKeyLengths[keys];
      for(unsigned int i = 0; i < keys; i++)
      {
         pKeys[i] = publicKeys[i]->getPKEY();
         eKeys[i] = new char[publicKeys[i]->getOutputSize()];
      }
      
      // create iv buffer
      unsigned int ivLength = EVP_CIPHER_iv_length(mCipherFunction);
      char* iv = (ivLength == 0) ? NULL : new char[ivLength];
      
      // initialize sealing the envelope
      if(EVP_SealInit(
         &mCipherContext, mCipherFunction,
         (unsigned char**)eKeys, eKeyLengths, (unsigned char*)iv,
         pKeys, keys) == 1)
      {
         // initialization successful
         rval = true;
         
         // set the encrypted symmetric key data
         for(unsigned int i = 0; i < keys; i++)
         {
            // copy iv
            char* ivCopy = NULL;
            if(ivLength > 0)
            {
               ivCopy = new char[ivLength];
               memcpy(ivCopy, iv, ivLength);
            }
            
            // create encrypted symmetric key
            symmetricKeys[i] = new SymmetricKey(algorithm);
            symmetricKeys[i]->assignData(
               eKeys[i], eKeyLengths[i], ivCopy, ivLength, true);
         }
         
         if(iv != NULL)
         {
            // delete iv buffer (freeing the other buffers is up
            // to the SymmetricKeys)
            delete [] iv;
         }
      }
      else
      {
         // delete all allocated key data
         for(unsigned int i = 0; i < keys; i++)
         {
            delete [] eKeys[i];
         }
         
         if(iv != NULL)
         {
            // delete iv buffer (freeing the other buffers is up to the
            // SymmetricKeys)
            delete [] iv;
         }
         
         Exception::setLast(new IOException(
            "Could not start opening envelope!",
            ERR_error_string(ERR_get_error(), NULL)));
      }
   }
   
   return rval;
}

bool DigitalEnvelope::startOpening(
   PrivateKey* privateKey, SymmetricKey* symmetricKey)
{
   bool rval = false;
   
   // disable encryption mode
   mEncryptMode = false;
   
   // get the cipher function
   mCipherFunction = getCipherFunction(symmetricKey->getAlgorithm());
   if(mCipherFunction != NULL)
   {
      // get the symmetric key data
      char* eKey;
      unsigned int eKeyLength;
      char* iv;
      unsigned int ivLength;
      symmetricKey->getData(&eKey, eKeyLength, &iv, ivLength);
      
      // initialize opening the envelope
      if(EVP_OpenInit(
         &mCipherContext, mCipherFunction,
         (unsigned char*)eKey, eKeyLength, (unsigned char*)iv,
         privateKey->getPKEY()) == 1)
      {
         rval = true;
      }
      else
      {
         Exception::setLast(new IOException(
            "Could not start opening envelope!",
            ERR_error_string(ERR_get_error(), NULL)));
      }
   }
   
   return rval;
}

bool DigitalEnvelope::update(char* in, int inLength, char* out, int& outLength)
{
   bool rval = false;
   
   // only proceed if the cipher function has been set
   if(mCipherFunction != NULL)
   {
      if(isEncryptEnabled())
      {
         // seal more data
         if(EVP_SealUpdate(
            &mCipherContext, (unsigned char*)out, &outLength,
            (unsigned char*)in, inLength) == 1)
         {
            rval = true;
         }
         else
         {
            Exception::setLast(new IOException(
               "Could not seal envelope data!",
               ERR_error_string(ERR_get_error(), NULL)));
         }
      }
      else
      {
         // open more data
         if(EVP_OpenUpdate(
            &mCipherContext, (unsigned char*)out, &outLength,
            (unsigned char*)in, inLength) == 1)
         {
            rval = true;
         }
         else
         {
            Exception::setLast(new IOException(
               "Could not open envelope data!",
               ERR_error_string(ERR_get_error(), NULL)));
         }
      }
   }
   else
   {
      Exception::setLast(new IOException(
         "Cannot update envelope; envelope not started!"));
   }
   
   return rval;
}

bool DigitalEnvelope::finish(char* out, int& length)
{
   bool rval = false;
   
   // only proceed if the cipher function has been set
   if(mCipherFunction != NULL)
   {
      if(isEncryptEnabled())
      {
         // finish sealing
         if(EVP_SealFinal(&mCipherContext, (unsigned char*)out, &length) == 1)
         {
            rval = true;
         }
         else
         {
            Exception::setLast(new IOException(
               "Could not finish sealing envelope!",
               ERR_error_string(ERR_get_error(), NULL)));
         }
      }
      else
      {
         // finish opening
         if(EVP_OpenFinal(&mCipherContext, (unsigned char*)out, &length) == 1)
         {
            rval = true;
         }
         else
         {
            Exception::setLast(new IOException(
               "Could not finish opening envelope!",
               ERR_error_string(ERR_get_error(), NULL)));
         }
      }
   }
   else
   {
      Exception::setLast(new IOException(
         "Cannot finish envelope; envelope not started!"));
   }
   
   return rval;
}
