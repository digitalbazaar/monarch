/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "DigitalEnvelope.h"
#include "PrivateKey.h"
#include "PublicKey.h"
#include "SymmetricKeyFactory.h"

#include <openssl/err.h>

using namespace std;
using namespace db::crypto;
using namespace db::io;

DigitalEnvelope::DigitalEnvelope() : AbstractBlockCipher(true)
{
}

DigitalEnvelope::~DigitalEnvelope()
{
}

void DigitalEnvelope::startSealing(
   const string& algorithm, PublicKey* publicKey, SymmetricKey** symmetricKey)
throw(IOException, UnsupportedAlgorithmException)
{
   // use just a single public key
   startSealing(algorithm, &publicKey, symmetricKey, 1);
}

void DigitalEnvelope::startSealing(
   const string& algorithm, PublicKey** publicKeys,
   SymmetricKey** symmetricKeys, unsigned int keys)
throw(IOException, UnsupportedAlgorithmException)
{
   if(algorithm != "AES256")
   {
      throw UnsupportedAlgorithmException(
         "Unsupported key algorithm '" + algorithm + "'!");
   }
   
   // enable encryption mode
   mEncryptMode = true;
   
   // get the cipher function
   mCipherFunction = getCipherFunction();
   
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
         symmetricKeys[i]->assignData(eKeys[i], eKeyLengths[i], ivCopy, true);
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
         // delete iv buffer (freeing the other buffers is up to the SymmetricKeys)
         delete [] iv;
      }
      
      throw IOException(
         "Could not start opening envelope!",
         ERR_error_string(ERR_get_error(), NULL));
   }
}

void DigitalEnvelope::startOpening(
   PrivateKey* privateKey, SymmetricKey* symmetricKey)
throw(IOException, UnsupportedAlgorithmException)
{
   if(symmetricKey->getAlgorithm() != "AES256")
   {
      throw UnsupportedAlgorithmException(
         "Unsupported key algorithm '" + symmetricKey->getAlgorithm() + "'!"); 
   }
   
   // disable encryption mode
   mEncryptMode = false;
   
   // get the cipher function
   mCipherFunction = getCipherFunction();
   
   // get the symmetric key data
   char* eKey;
   unsigned int eKeyLength;
   char* iv;
   symmetricKey->getData(&eKey, eKeyLength, &iv);
   
   // initialize opening the envelope
   if(EVP_OpenInit(
      &mCipherContext, mCipherFunction,
      (unsigned char*)eKey, eKeyLength, (unsigned char*)iv,
      privateKey->getPKEY()) != 1)
   {
      throw IOException(
         "Could not start opening envelope!",
         ERR_error_string(ERR_get_error(), NULL));
   }
}

const EVP_CIPHER* DigitalEnvelope::getCipherFunction()
{
   return EVP_aes_256_cbc();
}

void DigitalEnvelope::update(char* in, int inLength, char* out, int& outLength)
 throw(IOException)
{
   // only proceed if the cipher function has been set
   if(mCipherFunction != NULL)
   {
      if(isEncryptEnabled())
      {
         // seal more data
         if(EVP_SealUpdate(
            &mCipherContext, (unsigned char*)out, &outLength,
            (unsigned char*)in, inLength) != 1)
         {
            throw IOException(
               "Could not seal envelope data!",
               ERR_error_string(ERR_get_error(), NULL));
         }
      }
      else
      {
         // open more data
         if(EVP_OpenUpdate(
            &mCipherContext, (unsigned char*)out, &outLength,
            (unsigned char*)in, inLength) != 1)
         {
            throw IOException(
               "Could not open envelope data!",
               ERR_error_string(ERR_get_error(), NULL));
         }
      }
   }
}

void DigitalEnvelope::finish(char* out, int& length) throw(IOException)
{
   // only proceed if the cipher function has been set
   if(mCipherFunction != NULL)
   {
      if(isEncryptEnabled())
      {
         // finish sealing
         if(EVP_SealFinal(&mCipherContext, (unsigned char*)out, &length) != 1)
         {
            throw IOException(
               "Could not finish sealing envelope!",
               ERR_error_string(ERR_get_error(), NULL));
         }
      }
      else
      {
         // finish opening
         if(EVP_OpenFinal(&mCipherContext, (unsigned char*)out, &length) != 1)
         {
            throw IOException(
               "Could not finish opening envelope!",
               ERR_error_string(ERR_get_error(), NULL));
         }
      }
   }
}
