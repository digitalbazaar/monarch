/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/SymmetricKey.h"

#include <cstdlib>
#include <cstring>

using namespace db::crypto;

SymmetricKey::SymmetricKey(const char* algorithm)
{
   // no key data yet
   mData = NULL;
   mDataLength = 0;
   
   // no IV yet
   mIv = NULL;
   mIvLength = 0;
   
   // set algorithm
   mAlgorithm = strdup(algorithm);
   
   // default to unencrypted
   mEncrypted = false;
}

SymmetricKey::~SymmetricKey()
{
   // clean up
   freeData();
   freeIv();
   free(mAlgorithm);
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
   free(mAlgorithm);
   mAlgorithm = strdup(algorithm);
}

const char* SymmetricKey::getAlgorithm()
{
   return mAlgorithm;
}

bool SymmetricKey::isEncrypted()
{
   return mEncrypted;
}
