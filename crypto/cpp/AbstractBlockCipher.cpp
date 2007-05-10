/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "AbstractBlockCipher.h"

using namespace std;
using namespace db::crypto;

AbstractBlockCipher::AbstractBlockCipher(bool encrypt)
{
   // store encrypt mode
   mEncryptMode = encrypt;
   
   // initialize the cipher context
   EVP_CIPHER_CTX_init(&mCipherContext);
   
   // set the cipher function to null
   mCipherFunction = NULL;
}

AbstractBlockCipher::~AbstractBlockCipher()
{
   // clean up the cipher context
   EVP_CIPHER_CTX_cleanup(&mCipherContext);
}

const EVP_CIPHER* AbstractBlockCipher::getCipherFunction(
   const string& algorithm)
throw(UnsupportedAlgorithmException)
{
   const EVP_CIPHER* rval;
   
   if(algorithm == "AES256")
   {
      rval = EVP_aes_256_cbc();
   }
   else if(algorithm == "AES128")
   {
      rval = EVP_aes_128_cbc();
   }
   else if(algorithm == "3DES")
   {
      rval = EVP_des_ede3_cbc();
   }
   else
   {
      throw UnsupportedAlgorithmException(
         "Unsupported key algorithm '" + algorithm + "'!");
   }
   
   return rval;
}

unsigned int AbstractBlockCipher::getBlockSize()
{
   return EVP_CIPHER_CTX_block_size(&mCipherContext);
}

bool AbstractBlockCipher::isEncryptEnabled()
{
   return mEncryptMode;
}
