/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "SymmetricKey.h"

using namespace std;
using namespace db::crypto;

SymmetricKey::SymmetricKey(const string& algorithm)
{
   // no key data yet
   mData = NULL;
   mDataLength = 0;
   
   // no IV yet
   mIv = NULL;
   
   // set algorithm
   mAlgorithm = algorithm;
   
   // default to unencrypted
   mEncrypted = false;
}

SymmetricKey::~SymmetricKey()
{
   // clean up
   freeData();
   freeIv();
}

void SymmetricKey::freeData()
{
   // free key data
   if(mData != NULL)
   {
      delete [] mData;
   }
   
   mData = NULL;
   mDataLength = 0;
}

void SymmetricKey::freeIv()
{
   // free IV
   if(mIv != NULL)
   {
      delete [] mIv;
   }
   
   mIv = NULL;
}

void SymmetricKey::assignData(
   char* data, unsigned int length, char* iv, bool encrypted)
{
   // free existing data and IV
   freeData();
   freeIv();
   
   // set new data
   mData = data;
   mDataLength = length;
   mIv = iv;
   
   // set encrypted flag
   mEncrypted = encrypted;
}

void SymmetricKey::setData(
   const char* data, unsigned int length, const char* iv, bool encrypted)
{
   // free existing data as necessary
   if(mData != NULL && sizeof(mData) <= length)
   {
      freeData();
   }
   
   // free existing IV as necessary
   if(iv == NULL || (mIv != NULL && sizeof(mIv) <= sizeof(iv)))
   {
      freeIv();
   }
   
   // allocate data as necessary
   if(mData == NULL)
   {
      mData = new char[length];
   }
   
   // allocate IV as necessary
   if(mIv == NULL && iv != NULL)
   {
      mIv = new char[sizeof(iv)];
   }
   
   // copy data
   memcpy(mData, data, length);
   mDataLength = length;
   
   // copy iv
   if(iv != NULL)
   {
      memcpy(mIv, iv, sizeof(iv));
   }
   
   // set encrypted flag
   mEncrypted = encrypted;
}

void SymmetricKey::getData(char** data, unsigned int& length, char** iv)
{
   *data = mData;
   length = mDataLength;
   *iv = mIv;
}

const string& SymmetricKey::getAlgorithm()
{
   return mAlgorithm;
}

bool SymmetricKey::isEncrypted()
{
   return mEncrypted;
}
