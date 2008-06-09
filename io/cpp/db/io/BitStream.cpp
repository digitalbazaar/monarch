/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/BitStream.h"

#include <cstdlib>
#include <cstring>

using namespace db::io;

BitStream::BitStream()
{
   // start with a 16 byte capacity
   mSize = 16;
   mBitSet = (unsigned char*)malloc(mSize);
   memset(mBitSet, 0, mSize);
   mLength = 0;
}

BitStream::~BitStream()
{
   free(mBitSet);
}

void BitStream::resize()
{
   // increase bit set size by half
   int size = mSize / 2 + mSize;
   unsigned char* data = (unsigned char*)malloc(size);
   memset(data + mSize, 0, size - mSize);
   memcpy(data, mBitSet, mSize);
   free(mBitSet);
   mBitSet = data;
   mSize = size;
}

void BitStream::append(bool bit)
{
   // increase bit set size as appropriate
   if(mLength == mSize * 8)
   {
      resize();
   }
   
   // OR appropriate byte in bit set
   mBitSet[mLength / 8] |= bit ? (0x80 >> (mLength % 8)) : 0;
   mLength++;
}

void BitStream::append(const unsigned char* b, int length)
{
   // increase bit set size as appropriate
   if(mSize * 8 - mLength < length * 8)
   {
      resize();
      append(b, length);
   }
   else if(length == 1)
   {
      for(int i = 0; i < 8; i++)
      {
         append((b[0] & (0x80 >> i)) != 0);
      }
   }
   else if(mLength % 8 == 0)
   {
      // copy bytes directly
      memcpy(mBitSet + (mLength / 8), b, length);
      mLength += length * 8;
   }
   else
   {
      // do unoptimized bit-appending
      for(int i = 0; i < length; i++)
      {
         append(b + i, 1);
      }
   }
}

bool BitStream::operator[](int offset)
{
   return (mBitSet[offset / 8] & (0x80 >> (offset % 8))) != 0;
}

unsigned char BitStream::get(int bitOffset)
{
   unsigned char rval = 0;
   
   if(bitOffset < mLength && bitOffset % 8 == 0)
   {
      rval = mBitSet[bitOffset / 8];
   }
   else
   {
      for(int i = 0; i < 8 && (bitOffset + i) < length(); i++)
      {
         if((*this)[bitOffset + i])
         {
            rval |= (0x1 << i);
         }
      }
   }
   
   return rval;
}

void BitStream::get(int bitOffset, unsigned char* b, int count)
{
   if(bitOffset < mLength && bitOffset % 8 == 0)
   {
      // copy bytes directly
      memcpy(b, mBitSet + (bitOffset / 8), count);
   }
   else
   {
      // do unoptimized OR'ing of bits
      for(int i = 0; i < count && bitOffset < length(); i++)
      {
         b[i] = 0;
         for(int n = 0; n < 8 && bitOffset < length(); n++, bitOffset++)
         {
            b[i] |= (*this)[bitOffset] ? (0x80 >> (n % 8)) : 0;
         }
      }
   }
}

void BitStream::setLength(int length)
{
   if(mLength > mSize)
   {
      // resize to accomodate length
      // Note: this could obviously be optimized, but works fine 
      // in the general case for bfp watermarks
      resize();
      setLength(length);
   }
   else
   {
      mLength = length;
   }
}

int BitStream::length()
{
   return mLength;
}

void BitStream::clear()
{
   memset(mBitSet, 0, mSize);
}

void BitStream::set()
{
   memset(mBitSet, 0xFF, mSize);
}

unsigned char* BitStream::bytes()
{
   return (unsigned char*)mBitSet;
}

int BitStream::bytesLength()
{
   return (length() / 8) + (((length() % 8) != 0) ? 1 : 0);
}

std::string& BitStream::toString(std::string& str)
{
   str.reserve(str.length() + length());
   for(int i = 0; i < length(); i++)
   {
      str.push_back((*this)[i] ? '1' : '0');
   }
   return str;
}
