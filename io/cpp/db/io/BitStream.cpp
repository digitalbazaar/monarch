/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/io/BitStream.h"

#include "db/rt/Exception.h"

#include <cstdlib>

using namespace db::io;
using namespace db::rt;

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

void BitStream::resize(int bytes)
{
   // increase by bytes
   int size = mSize + bytes;
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
      resize(mSize / 2);
   }
   
   // OR appropriate byte in bit set
   mBitSet[mLength / 8] |= bit ? (0x80 >> (mLength % 8)) : 0;
   mLength++;
}

void BitStream::append(const unsigned char* b, int length)
{
   // increase bit set size as appropriate
   int diff = (length * 8 - (mSize * 8 - mLength));
   if(diff > 0)
   {
      resize(diff);
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

bool BitStream::appendFromString(const char* str, int length)
{
   bool rval = true;
   
   // determine the number of bytes that need to be allocated
   int bytes = length / 8 + (length % 8 != 0 ? 1 : 0);
   if(mSize < bytes)
   {
      resize(bytes - mSize);
   }
   
   // append bits
   for(int i = 0; rval && i < length; i++)
   {
      if(str[i] == '0')
      {
         append(false);
      }
      else if(str[i] == '1')
      {
         append(true);
      }
      else
      {
         ExceptionRef e = new Exception(
            "Could not convert BitStream from string, invalid character '%c' "
            "at position %d.", "db.io.BitStream.InvalidBitChar");
         Exception::setLast(e, false);
         rval = false;
      }
   }
   
   return rval;
}

std::string& BitStream::appendToString(std::string& str)
{
   str.reserve(str.length() + length());
   for(int i = 0; i < length(); i++)
   {
      str.push_back((*this)[i] ? '1' : '0');
   }
   return str;
}

bool BitStream::operator[](int offset)
{
   return (mBitSet[offset / 8] & (0x80 >> (offset % 8))) != 0;
}

void BitStream::operator<<(int n)
{
   if(n >= mLength)
   {
      clear();
      setLength(0);
   }
   else
   {
      // FIXME: this implementation is terribly slow
      // FIXME: instead do memmove for every whole byte, then shift every byte
      // or something smarter
      
      std::string out = toString();
      out.erase(0, n);
      clear();
      setLength(0);
      appendFromString(out.c_str(), out.length());
   }
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
      resize(mSize / 2);
      setLength(length);
   }
   else
   {
      mLength = length;
   }
}

inline int BitStream::length()
{
   return mLength;
}

inline void BitStream::clear()
{
   memset(mBitSet, 0, mSize);
}

inline void BitStream::set()
{
   memset(mBitSet, 0xFF, mSize);
}

inline unsigned char* BitStream::bytes()
{
   return mBitSet;
}

int BitStream::bytesLength()
{
   return (length() / 8) + (((length() % 8) != 0) ? 1 : 0);
}

std::string BitStream::toString()
{
   std::string str;
   appendToString(str);
   return str;
}

std::string BitStream::toString(int offset, int length)
{
   std::string str;
   str.reserve(str.length() + length);
   int stop = offset + length;
   for(int i = offset; i < stop; i++)
   {
      str.push_back((*this)[i] ? '1' : '0');
   }
   return str;
}
