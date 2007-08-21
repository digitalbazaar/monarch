/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "BigInteger.h"

#include <assert.h>

using namespace std;
using namespace db::crypto;

BigInteger::BigInteger(long long value)
{
   mBigNum = BN_new();
   mBigNumContext = NULL;
   *this = value;
}

BigInteger::BigInteger(const BigInteger& rhs)
{
   mBigNum = BN_new();
   mBigNumContext = NULL;
   assert(BN_copy(mBigNum, rhs.mBigNum) == mBigNum);
}

BigInteger::~BigInteger()
{
   BN_free(mBigNum);
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
   assert(BN_copy(mBigNum, rhs.mBigNum) == mBigNum);
   return *this;
}

BigInteger& BigInteger::operator=(long long rhs)
{
   if(rhs > 0)
   {
      assert(BN_set_word(mBigNum, (unsigned long)rhs) == 0);
   }
   else if(rhs == 0)
   {
      assert(BN_zero(mBigNum) == 0);
   }
   else
   {
      assert(BN_set_word(mBigNum, (unsigned long)-rhs) == 0);
      mBigNum->neg = 1;
   }
   
   return *this;
}

BigInteger& BigInteger::operator=(unsigned long rhs)
{
   if(rhs == 0)
   {
      assert(BN_zero(mBigNum) == 0);
   }
   else
   {
      assert(BN_set_word(mBigNum, rhs) == 0);
   }
   
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

BigInteger BigInteger::operator+(const BigInteger& rhs)
{
   BigInteger rval;
   assert(BN_add(rval.mBigNum, mBigNum, rhs.mBigNum) == 0);
   return rval;
}

BigInteger BigInteger::operator-(const BigInteger& rhs)
{
   BigInteger rval;
   assert(BN_sub(rval.mBigNum, mBigNum, rhs.mBigNum) == 0);
   return rval;
}

BigInteger BigInteger::operator*(const BigInteger& rhs)
{
   BigInteger rval;
   assert(BN_mul(rval.mBigNum, mBigNum, rhs.mBigNum, getContext()) == 0);
   return rval;
}

BigInteger BigInteger::operator/(const BigInteger& rhs)
{
   BigInteger rval;
   assert(BN_div(rval.mBigNum, NULL, mBigNum, rhs.mBigNum, getContext()) == 0);
   return rval;
}

BigInteger BigInteger::pow(const BigInteger& rhs)
{
   BigInteger rval;
   assert(BN_exp(rval.mBigNum, mBigNum, rhs.mBigNum, getContext()) == 0);
   return rval;
}

BigInteger BigInteger::operator%(const BigInteger& rhs)
{
   BigInteger rval;
   assert(BN_mod(rval.mBigNum, mBigNum, rhs.mBigNum, getContext()) == 0);
   return rval;
}

BigInteger& BigInteger::operator+=(const BigInteger& rhs)
{
   assert(BN_add(mBigNum, mBigNum, rhs.mBigNum) == 0);
   return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& rhs)
{
   assert(BN_sub(mBigNum, mBigNum, rhs.mBigNum) == 0);
   return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& rhs)
{
   assert(BN_mul(mBigNum, mBigNum, rhs.mBigNum, getContext()) == 0);
   return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& rhs)
{
   assert(BN_div(mBigNum, NULL, mBigNum, rhs.mBigNum, getContext()) == 0);
   return *this;
}

BigInteger& BigInteger::powEquals(const BigInteger& rhs)
{
   assert(BN_exp(mBigNum, mBigNum, rhs.mBigNum, getContext()) == 0);
   return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& rhs)
{
   assert(BN_mod(mBigNum, mBigNum, rhs.mBigNum, getContext()) == 0);
   return *this;
}

void BigInteger::divide(
   const BigInteger divisor, BigInteger& quotient, BigInteger& remainder)
{
   assert(BN_div(quotient.mBigNum, remainder.mBigNum,
      mBigNum, divisor.mBigNum, getContext()) == 0);
}

bool BigInteger::isZero()
{
   return BN_get_word(mBigNum) == 0;
}

bool BigInteger::isNegative() const
{
   return mBigNum->neg == 1;
}

bool BigInteger::isCompact()
{
   return BN_get_word(mBigNum) < 0xffffffffL;
}

long long BigInteger::getInt64()
{
   long long rval = BN_get_word(mBigNum);
   if(isNegative())
   {
      rval = -rval;
   }
   
   return rval;
}
