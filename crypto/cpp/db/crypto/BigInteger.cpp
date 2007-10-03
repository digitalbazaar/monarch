/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/BigInteger.h"

#include <assert.h>
#include <openssl/crypto.h>

using namespace std;
using namespace db::crypto;

BigInteger::BigInteger(long long value)
{
   initialize();
   
   if(value != 0)
   {
      *this = value;
   }
}

BigInteger::BigInteger(unsigned long value)
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

BigInteger::BigInteger(unsigned int value)
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
   bool success = BN_copy(mBigNum, copy.mBigNum) == mBigNum;
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
   bool success = BN_copy(mBigNum, rhs.mBigNum) == mBigNum;
   assert(success);
   return *this;
}

BigInteger& BigInteger::operator=(long long rhs)
{
   if(rhs > 0)
   {
      int rc = BN_set_word(mBigNum, (unsigned long)rhs);
      assert(rc == 1);
   }
   else if(rhs == 0)
   {
      int rc = BN_zero(mBigNum);
      assert(rc == 1);
   }
   else
   {
      int rc = BN_set_word(mBigNum, (unsigned long)-rhs);
      assert(rc == 1);
      mBigNum->neg = 1;
   }
   
   return *this;
}

BigInteger& BigInteger::operator=(unsigned long rhs)
{
   int rc;
   
   if(rhs == 0)
   {
      rc = BN_zero(mBigNum);
   }
   else
   {
      rc = BN_set_word(mBigNum, rhs);
   }
   
   assert(rc == 1);
   
   return *this;
}

BigInteger& BigInteger::operator=(int rhs)
{
   *this = (long long)rhs;
   return *this;
}

BigInteger& BigInteger::operator=(unsigned int rhs)
{
   *this = (unsigned long)rhs;
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
   const BigInteger divisor, BigInteger& quotient, BigInteger& remainder)
{
   int rc = BN_div(quotient.mBigNum, remainder.mBigNum,
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
