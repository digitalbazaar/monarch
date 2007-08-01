/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "AbstractBlockCipher.h"
#include "UnsupportedAlgorithmException.h"

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
      char* msg = new char[27 + strlen(algorithm) + 2 + 1];
      sprintf(msg, "Unsupported key algorithm '%s'!", algorithm);
      Exception::setLast(new UnsupportedAlgorithmException(msg));
      delete msg;
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
