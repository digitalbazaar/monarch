/*
 * Copyright (c) 2007-2011 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/AbstractBlockCipher.h"

#include "monarch/rt/Exception.h"

#include <cstring>

using namespace monarch::crypto;
using namespace monarch::rt;

AbstractBlockCipher::AbstractBlockCipher(bool encrypt) :
   mEncryptMode(encrypt),
   mInputBytes(0),
   mOutputBytes(0),
   mCipherFunction(NULL)
{
   // initialize the cipher context
   EVP_CIPHER_CTX_init(&mCipherContext);
}

AbstractBlockCipher::~AbstractBlockCipher()
{
   // clean up the cipher context
   EVP_CIPHER_CTX_cleanup(&mCipherContext);
}

const EVP_CIPHER* AbstractBlockCipher::getCipherFunction(const char* algorithm)
{
   const EVP_CIPHER* rval = NULL;

   if(strcmp(algorithm, "AES") == 0 || strcmp(algorithm, "AES256") == 0)
   {
      rval = EVP_aes_256_cbc();
   }
   else if(strcmp(algorithm, "AES128") == 0)
   {
      rval = EVP_aes_128_cbc();
   }
   else if(strcmp(algorithm, "3DES") == 0)
   {
      rval = EVP_des_ede3_cbc();
   }
   else
   {
      ExceptionRef e = new Exception(
         "Unsupported key algorithm", "monarch.crypto.UnsupportedAlgorithm");
      e->getDetails()["algorithm"] = algorithm;
      Exception::set(e);
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
