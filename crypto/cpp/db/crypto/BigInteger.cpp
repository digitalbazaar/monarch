/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/crypto/BigInteger.h"

// FIXME: if asserts are not compiled in, and an error occurs, math will be off
#include <cassert>
#include <openssl/crypto.h>

#include <cstring>

using namespace std;
using namespace db::crypto;
using namespace db::io;

BigInteger::BigInteger(unsigned long long value)
{
   initialize();
   
   if(value != 0)
   {
      *this = value;
   }
}

BigInteger::BigInteger(long long value)
{
   initialize();
   
   if(value != 0)
   {
      *this = value;
   }
}

BigInteger::BigInteger(unsigned int value)
{
   initialize();
   
   if(value != 0)
   {
      *this = value;
   }
}

BigInteger::BigInteger(int value)
{
   initialize();
   
   if(value != 0)
   {
      *this = value;
   }
}

BigInteger::BigInteger(const char* value)
{
   initialize();
   *this = value;
}

BigInteger::BigInteger(const string& value)
{
   initialize();
   *this = value;
}

BigInteger::BigInteger(const BigInteger& copy)
{
   initialize();
   bool success = (BN_copy(mBigNum, copy.mBigNum) == mBigNum);
   assert(success);
}

BigInteger::~BigInteger()
{
   BN_free(mBigNum);
   
   if(mBigNumContext != NULL)
   {
      BN_CTX_free(mBigNumContext);
   }
}

void BigInteger::initialize()
{
   mBigNum = BN_new();
   mBigNumContext = NULL;
}

BN_CTX* BigInteger::getContext()
{
   if(mBigNumContext == NULL)
   {
      mBigNumContext = BN_CTX_new();
   }
   else
   {
      BN_CTX_init(mBigNumContext);
   }
   
   return mBigNumContext;
}

BigInteger& BigInteger::operator=(const BigInteger& rhs)
{
   bool success = (BN_copy(mBigNum, rhs.mBigNum) == mBigNum);
   assert(success);
   return *this;
}

BigInteger& BigInteger::operator=(unsigned long long rhs)
{
   char temp[22];
   sprintf(temp, "%llu", rhs);
   *this = temp;
   
   return *this;
}

BigInteger& BigInteger::operator=(long long rhs)
{
   char temp[22];
   sprintf(temp, "%lli", rhs);
   *this = temp;
   
   return *this;
}

BigInteger& BigInteger::operator=(unsigned int rhs)
{
   int rc = (rhs == 0 ? BN_zero(mBigNum) : BN_set_word(mBigNum, rhs));
   assert(rc == 1);
   return *this;
}

BigInteger& BigInteger::operator=(int rhs)
{
   *this = (long long)rhs;
   return *this;
}

BigInteger& BigInteger::operator=(const char* rhs)
{
   if(strcmp(rhs, "0") == 0)
   {
      int rc = BN_zero(mBigNum);
      assert(rc == 1);
   }
   else
   {
      if(BN_dec2bn(&mBigNum, rhs) == 0)
      {
         // string was an invalid number, so zero out BIGNUM
         int rc = BN_zero(mBigNum);
         assert(rc == 1);
      }
   }
   
   return *this;
}

BigInteger& BigInteger::operator=(const string& rhs)
{
   *this = rhs.c_str();
   return *this;
}

bool BigInteger::operator==(const BigInteger& rhs)
{
   return BN_cmp(mBigNum, rhs.mBigNum) == 0;
}

bool BigInteger::operator==(long long rhs)
{
   return getInt64() == rhs;
}

bool BigInteger::operator!=(const BigInteger& rhs)
{
   return !(*this == rhs);
}

bool BigInteger::operator!=(long long rhs)
{
   return !(*this == rhs);
}

bool BigInteger::operator<(const BigInteger& rhs)
{
   return BN_cmp(mBigNum, rhs.mBigNum) == -1;
}

bool BigInteger::operator>(const BigInteger& rhs)
{
   return BN_cmp(mBigNum, rhs.mBigNum) == 1;
}

bool BigInteger::operator<=(const BigInteger& rhs)
{
   return BN_cmp(mBigNum, rhs.mBigNum) <= 0;
}

bool BigInteger::operator>=(const BigInteger& rhs)
{
   return BN_cmp(mBigNum, rhs.mBigNum) >= 0;
}

BigInteger BigInteger::operator<<(int n)
{
   BigInteger rval;
   int rc = BN_lshift(rval.mBigNum, mBigNum, n);
   assert(rc == 1);
   return rval;
}

BigInteger BigInteger::operator>>(int n)
{
   BigInteger rval;
   int rc = BN_rshift(rval.mBigNum, mBigNum, n);
   assert(rc == 1);
   return rval;
}

BigInteger BigInteger::operator+(const BigInteger& rhs)
{
   BigInteger rval;
   int rc = BN_add(rval.mBigNum, mBigNum, rhs.mBigNum);
   assert(rc == 1);
   return rval;
}

BigInteger BigInteger::operator-(const BigInteger& rhs)
{
   BigInteger rval;
   int rc = BN_sub(rval.mBigNum, mBigNum, rhs.mBigNum);
   assert(rc == 1);
   return rval;
}

BigInteger BigInteger::operator*(const BigInteger& rhs)
{
   BigInteger rval;
   int rc = BN_mul(rval.mBigNum, mBigNum, rhs.mBigNum, getContext());
   assert(rc == 1);
   return rval;
}

BigInteger BigInteger::operator/(const BigInteger& rhs)
{
   BigInteger rval;
   int rc = BN_div(rval.mBigNum, NULL, mBigNum, rhs.mBigNum, getContext());
   assert(rc == 1);
   return rval;
}

BigInteger BigInteger::pow(const BigInteger& rhs)
{
   BigInteger rval;
   int rc = BN_exp(rval.mBigNum, mBigNum, rhs.mBigNum, getContext());
   assert(rc == 1);
   return rval;
}

BigInteger BigInteger::operator%(const BigInteger& rhs)
{
   BigInteger rval;
   int rc = BN_mod(rval.mBigNum, mBigNum, rhs.mBigNum, getContext());
   assert(rc == 1);
   return rval;
}

BigInteger& BigInteger::operator+=(const BigInteger& rhs)
{
   int rc = BN_add(mBigNum, mBigNum, rhs.mBigNum);
   assert(rc == 1);
   return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& rhs)
{
   int rc = BN_sub(mBigNum, mBigNum, rhs.mBigNum);
   assert(rc == 1);
   return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& rhs)
{
   int rc = BN_mul(mBigNum, mBigNum, rhs.mBigNum, getContext());
   assert(rc == 1);
   return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& rhs)
{
   int rc = BN_div(mBigNum, NULL, mBigNum, rhs.mBigNum, getContext());
   assert(rc == 1);
   return *this;
}

BigInteger& BigInteger::powEquals(const BigInteger& rhs)
{
   int rc = BN_exp(mBigNum, mBigNum, rhs.mBigNum, getContext());
   assert(rc == 1);
   return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& rhs)
{
   int rc = BN_mod(mBigNum, mBigNum, rhs.mBigNum, getContext());
   assert(rc == 1);
   return *this;
}

int BigInteger::absCompare(const BigInteger& rhs)
{
   return BN_ucmp(mBigNum, rhs.mBigNum);
}

void BigInteger::divide(
   const BigInteger& divisor, BigInteger& quotient, BigInteger& remainder)
{
   int rc = BN_div(
      quotient.mBigNum, remainder.mBigNum,
      mBigNum, divisor.mBigNum, getContext());
   assert(rc == 1);
}

bool BigInteger::isZero() const
{
   return BN_get_word(mBigNum) == 0;
}

void BigInteger::setNegative(bool negative)
{
   mBigNum->neg = (negative) ? 1 : 0;
}

bool BigInteger::isNegative() const
{
   return mBigNum->neg == 1;
}

bool BigInteger::isCompact() const
{
   return BN_get_word(mBigNum) < 0xffffffffL;
}

unsigned int BigInteger::getUInt32() const
{
   return BN_get_word(mBigNum);
}

long long BigInteger::getInt64() const
{
   long long rval = BN_get_word(mBigNum);
   if(isNegative())
   {
      rval = -rval;
   }
   
   return rval;
}

int BigInteger::getNumBytes() const
{
   return BN_num_bytes(mBigNum);
}

void BigInteger::fromBytes(const char* data, int length)
{
   // read the number in
   BN_bin2bn((const unsigned char *)data, length, mBigNum);
}

void BigInteger::toBytes(ByteBuffer* b)
{
   // make enough room for the number
   int size = getNumBytes();
   b->allocateSpace(size, true);
   
   // write the number out
   BN_bn2bin(mBigNum, (unsigned char*)b->data() + b->length());
   b->extend(size);
}

string BigInteger::toString() const
{
   char* s = BN_bn2dec(mBigNum);
   assert(s != NULL);
   string str = s;
   OPENSSL_free(s);
   return str;
}
