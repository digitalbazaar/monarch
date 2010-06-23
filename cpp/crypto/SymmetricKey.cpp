/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/crypto/SymmetricKey.h"

#include "monarch/util/Convert.h"

#include <cstdlib>
#include <cstring>

using namespace monarch::crypto;
using namespace monarch::util;

SymmetricKey::SymmetricKey(const char* algorithm) :
   mData(NULL),
   mDataLength(0),
   mIv(NULL),
   mIvLength(0),
   mAlgorithm(NULL),
   mEncrypted(false)
{
   setAlgorithm(algorithm);
}

SymmetricKey::~SymmetricKey()
{
   // clean up
   freeData();
   freeIv();
   if(mAlgorithm != NULL)
   {
      free(mAlgorithm);
   }
}

void SymmetricKey::freeData()
{
   // free key data
   if(mData != NULL)
   {
      // zero out data first
      memset(mData, 0, mDataLength);
      free(mData);
      mData = NULL;
      mDataLength = 0;
   }
}

void SymmetricKey::freeIv()
{
   // free IV
   if(mIv != NULL)
   {
      // zero out data first
      memset(mIv, 0, mIvLength);
      free(mIv);
      mIv = NULL;
      mIvLength = 0;
   }
}

void SymmetricKey::assignData(
   char* data, unsigned int length,
   char* iv, unsigned int ivLength, bool encrypted)
{
   // free existing data and IV
   freeData();
   freeIv();

   // set new data
   mData = data;
   mDataLength = length;
   mIv = iv;
   mIvLength = ivLength;

   // set encrypted flag
   mEncrypted = encrypted;
}

void SymmetricKey::setData(
   const char* data, unsigned int length,
   const char* iv, unsigned int ivLength, bool encrypted)
{
   // free existing data and IV
   freeData();
   freeIv();

   // allocate data as necessary
   if(data != NULL)
   {
      // copy data
      mData = (char*)malloc(length);
      memcpy(mData, data, length);
      mDataLength = length;
   }

   // allocate IV as necessary
   if(iv != NULL)
   {
      // copy IV
      mIv = (char*)malloc(ivLength);
      memcpy(mIv, iv, ivLength);
      mIvLength = ivLength;
   }

   // set encrypted flag
   mEncrypted = encrypted;
}

bool SymmetricKey::setHexData(const char* hex, int length)
{
   bool rval = true;

   if(length == -1)
   {
      length = strlen(hex);
   }

   char b[length / 2 + 1];
   unsigned int len;
   rval = Convert::hexToBytes(hex, length, b, len);
   if(rval)
   {
      setData(b, len);
   }

   return rval;
}

void SymmetricKey::getData(
   char** data, unsigned int& length,
   char** iv, unsigned int& ivLength)
{
   *data = mData;
   length = mDataLength;
   *iv = mIv;
   ivLength = mIvLength;
}

const char* SymmetricKey::data()
{
   return mData;
}

unsigned int SymmetricKey::length()
{
   return mDataLength;
}

const char* SymmetricKey::iv()
{
   return mIv;
}

unsigned int SymmetricKey::ivLength()
{
   return mIvLength;
}

void SymmetricKey::setAlgorithm(const char* algorithm)
{
   if(mAlgorithm != NULL)
   {
      free(mAlgorithm);
   }
   mAlgorithm = (algorithm == NULL) ? NULL : strdup(algorithm);
}

const char* SymmetricKey::getAlgorithm()
{
   return (mAlgorithm == NULL) ? "" : mAlgorithm;
}

bool SymmetricKey::isEncrypted()
{
   return mEncrypted;
}
