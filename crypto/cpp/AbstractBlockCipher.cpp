/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "AbstractBlockCipher.h"
#include "UnsupportedAlgorithmException.h"
#include "Thread.h"

using namespace std;
using namespace db::crypto;
using namespace db::rt;

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
{
   const EVP_CIPHER* rval = NULL;
   
   if(algorithm == "AES" || algorithm == "AES256")
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
      Thread::setException(new UnsupportedAlgorithmException(
         "Unsupported key algorithm '" + algorithm + "'!"));
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
