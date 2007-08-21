/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "BigInteger.h"

#include <assert.h>
#include <openssl/crypto.h>

using namespace std;
using namespace db::crypto;

BigInteger::BigInteger(long long value)
{
   mBigNum = BN_new();
   mBigNumContext = NULL;
   
   if(value != 0)
   {
      *this = value;
   }
}

BigInteger::BigInteger(const string& value)
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
      assert(BN_set_word(mBigNum, (unsigned long)rhs) == 1);
   }
   else if(rhs == 0)
   {
      assert(BN_zero(mBigNum) == 1);
   }
   else
   {
      assert(BN_set_word(mBigNum, (unsigned long)-rhs) == 1);
      mBigNum->neg = 1;
   }
   
   return *this;
}

BigInteger& BigInteger::operator=(unsigned long rhs)
{
   if(rhs == 0)
   {
      assert(BN_zero(mBigNum) == 1);
   }
   else
   {
      assert(BN_set_word(mBigNum, rhs) == 1);
   }
   
   return *this;
}

BigInteger& BigInteger::operator=(const string& rhs)
{
   if(strcmp(rhs.c_str(), "0") == 0)
   {
      assert(BN_zero(mBigNum) == 1);
   }
   else
   {
      if(BN_dec2bn(&mBigNum, rhs.c_str()) == 0)
      {
         // string was an invalid number, so zero out BIGNUM
         assert(BN_zero(mBigNum) == 1);
      }
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
   assert(BN_add(rval.mBigNum, mBigNum, rhs.mBigNum) == 1);
   return rval;
}

BigInteger BigInteger::operator-(const BigInteger& rhs)
{
   BigInteger rval;
   assert(BN_sub(rval.mBigNum, mBigNum, rhs.mBigNum) == 1);
   return rval;
}

BigInteger BigInteger::operator*(const BigInteger& rhs)
{
   BigInteger rval;
   assert(BN_mul(rval.mBigNum, mBigNum, rhs.mBigNum, getContext()) == 1);
   return rval;
}

BigInteger BigInteger::operator/(const BigInteger& rhs)
{
   BigInteger rval;
   assert(BN_div(rval.mBigNum, NULL, mBigNum, rhs.mBigNum, getContext()) == 1);
   return rval;
}

BigInteger BigInteger::pow(const BigInteger& rhs)
{
   BigInteger rval;
   assert(BN_exp(rval.mBigNum, mBigNum, rhs.mBigNum, getContext()) == 1);
   return rval;
}

BigInteger BigInteger::operator%(const BigInteger& rhs)
{
   BigInteger rval;
   assert(BN_mod(rval.mBigNum, mBigNum, rhs.mBigNum, getContext()) == 1);
   return rval;
}

BigInteger& BigInteger::operator+=(const BigInteger& rhs)
{
   assert(BN_add(mBigNum, mBigNum, rhs.mBigNum) == 1);
   return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& rhs)
{
   assert(BN_sub(mBigNum, mBigNum, rhs.mBigNum) == 1);
   return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& rhs)
{
   assert(BN_mul(mBigNum, mBigNum, rhs.mBigNum, getContext()) == 1);
   return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& rhs)
{
   assert(BN_div(mBigNum, NULL, mBigNum, rhs.mBigNum, getContext()) == 1);
   return *this;
}

BigInteger& BigInteger::powEquals(const BigInteger& rhs)
{
   assert(BN_exp(mBigNum, mBigNum, rhs.mBigNum, getContext()) == 1);
   return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& rhs)
{
   assert(BN_mod(mBigNum, mBigNum, rhs.mBigNum, getContext()) == 1);
   return *this;
}

void BigInteger::divide(
   const BigInteger divisor, BigInteger& quotient, BigInteger& remainder)
{
   assert(BN_div(quotient.mBigNum, remainder.mBigNum,
      mBigNum, divisor.mBigNum, getContext()) == 1);
}

bool BigInteger::isZero() const
{
   return BN_get_word(mBigNum) == 0;
}

bool BigInteger::isNegative() const
{
   return mBigNum->neg == 1;
}

bool BigInteger::isCompact() const
{
   return BN_get_word(mBigNum) < 0xffffffffL;
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

string& BigInteger::toString(string& str) const
{
   char* s = BN_bn2dec(mBigNum);
   assert(s != NULL);
   str.assign(s);
   OPENSSL_free(s);
   return str;
}

ostream& operator<<(ostream& os, const BigInteger& bi)
{
   string str;
   os << bi.toString(str);
   return os;
}

istream& operator>>(istream& is, BigInteger& bi)
{
   string str;
   is >> str; 
   bi = str;
   return is;
}
