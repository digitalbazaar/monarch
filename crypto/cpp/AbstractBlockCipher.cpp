/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "AbstractBlockCipher.h"

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

unsigned int AbstractBlockCipher::getBlockSize()
{
   return EVP_CIPHER_CTX_block_size(&mCipherContext);
}

bool AbstractBlockCipher::isEncryptEnabled()
{
   return mEncryptMode;
}
