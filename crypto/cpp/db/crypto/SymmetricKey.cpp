/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/SymmetricKey.h"

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
      free(mData);
   }
   
   mData = NULL;
   mDataLength = 0;
}

void SymmetricKey::freeIv()
{
   // free IV
   if(mIv != NULL)
   {
      free(mIv);
   }
   
   mIv = NULL;
   mIvLength = 0;
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
   // free existing data as necessary
   if(mData != NULL && mDataLength <= length)
   {
      freeData();
   }
   
   // free existing IV as necessary
   if(iv == NULL || (mIv != NULL && mIvLength <= ivLength))
   {
      freeIv();
   }
   
   // allocate data as necessary
   if(mData == NULL)
   {
      mData = (char*)malloc(length);
   }
   
   // allocate IV as necessary
   if(mIv == NULL && iv != NULL)
   {
      mIv = (char*)malloc(ivLength);
   }
   
   // copy data
   memcpy(mData, data, length);
   mDataLength = length;
   
   // copy iv
   if(iv != NULL)
   {
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

const char* SymmetricKey::getAlgorithm()
{
   return mAlgorithm;
}

bool SymmetricKey::isEncrypted()
{
   return mEncrypted;
}
