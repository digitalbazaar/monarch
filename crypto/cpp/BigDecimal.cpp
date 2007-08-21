/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "BigDecimal.h"

using namespace std;
using namespace db::crypto;

BigDecimal::BigDecimal(long double value)
{
   *this = value;
}

BigDecimal::BigDecimal(const BigDecimal& rhs)
{
   mSignificand = rhs.mSignificand;
   mExponent = rhs.mExponent;
}

BigDecimal::~BigDecimal()
{
}

void BigDecimal::synchronizeExponents(BigDecimal& bd1, BigDecimal& bd2)
{
   // only do work if exponents are different
   if(bd1.mExponent != bd2.mExponent)
   {
      // use the larger exponent to retain precision
      if(bd1.mExponent > bd2.mExponent)
      {
         // increase significand by power difference
         bd2.mSignificand *= BigInteger(10).pow(bd1.mExponent - bd2.mExponent);
         bd2.mExponent = bd1.mExponent;
      }
      else
      {
         // increase significand by power difference
         bd1.mSignificand *= BigInteger(10).pow(bd2.mExponent - bd1.mExponent);
         bd1.mExponent = bd2.mExponent;
      }
   }
}

BigDecimal& BigDecimal::operator=(const BigDecimal& rhs)
{
   mSignificand = rhs.mSignificand;
   mExponent = rhs.mExponent;
   return *this;
}

BigDecimal& BigDecimal::operator=(long double rhs)
{
   // FIXME:
   return *this;
}

bool BigDecimal::operator==(const BigDecimal& rhs)
{
   BigDecimal temp = rhs;
   synchronizeExponents(*this, temp);
   return this->mSignificand == temp.mSignificand;
}

bool BigDecimal::operator==(long double rhs)
{
   return getDouble() == rhs;
}

bool BigDecimal::operator!=(const BigDecimal& rhs)
{
   return !(*this == rhs);
}

bool BigDecimal::operator!=(long double rhs)
{
   return !(*this == rhs);
}

bool BigDecimal::operator<(const BigDecimal& rhs)
{
   bool rval = false;
   
   if(isNegative() && !rhs.isNegative())
   {
      rval = true;
   }
   else if(isNegative() == rhs.isNegative())
   {
      BigDecimal temp = rhs;
      synchronizeExponents(*this, temp);
      if(mSignificand < temp.mSignificand)
      {
         rval = true;
      }
   }
   
   return rval;
}

bool BigDecimal::operator>(const BigDecimal& rhs)
{
   bool rval = false;
   
   if(!isNegative() && rhs.isNegative())
   {
      rval = true;
   }
   else if(isNegative() == rhs.isNegative())
   {
      BigDecimal temp = rhs;
      synchronizeExponents(*this, temp);
      if(mSignificand > temp.mSignificand)
      {
         rval = true;
      }
   }
   
   return rval;
}

bool BigDecimal::operator<=(const BigDecimal& rhs)
{
   return *this < rhs || *this == rhs;
}

bool BigDecimal::operator>=(const BigDecimal& rhs)
{
   return *this > rhs || *this == rhs;
}

BigDecimal BigDecimal::operator+(const BigDecimal& rhs)
{
   BigDecimal rval = *this;
   BigDecimal temp = rhs;
   synchronizeExponents(rval, temp);
   rval.mSignificand += temp.mSignificand;
   return rval;
}

BigDecimal BigDecimal::operator-(const BigDecimal& rhs)
{
   BigDecimal rval = *this;
   BigDecimal temp = rhs;
   synchronizeExponents(rval, temp);
   rval.mSignificand -= temp.mSignificand;
   return rval;
}

BigDecimal BigDecimal::operator*(const BigDecimal& rhs)
{
   BigDecimal rval = *this;
   BigDecimal temp = rhs;
   synchronizeExponents(rval, temp);
   rval.mSignificand *= temp.mSignificand;
   return rval;
}

BigDecimal BigDecimal::operator/(const BigDecimal& rhs)
{
   // FIXME: this one is more complicated
   BigDecimal rval = *this;
   BigDecimal temp = rhs;
   synchronizeExponents(rval, temp);
   rval.mSignificand /= temp.mSignificand;
   return rval;
}

BigDecimal BigDecimal::pow(const BigDecimal& rhs)
{
   BigDecimal rval = *this;
   BigDecimal temp = rhs;
   synchronizeExponents(rval, temp);
   rval.mSignificand.powEquals(temp.mSignificand);
   return rval;
}

BigDecimal BigDecimal::operator%(const BigDecimal& rhs)
{
   BigDecimal rval = *this;
   BigDecimal temp = rhs;
   synchronizeExponents(rval, temp);
   rval.mSignificand %= temp.mSignificand;
   return rval;
}

BigDecimal& BigDecimal::operator+=(const BigDecimal& rhs)
{
   BigDecimal temp = rhs;
   synchronizeExponents(*this, temp);
   this->mSignificand += temp.mSignificand;
   return *this;
}

BigDecimal& BigDecimal::operator-=(const BigDecimal& rhs)
{
   BigDecimal temp = rhs;
   synchronizeExponents(*this, temp);
   this->mSignificand -= temp.mSignificand;
   return *this;
}

BigDecimal& BigDecimal::operator*=(const BigDecimal& rhs)
{
   BigDecimal temp = rhs;
   synchronizeExponents(*this, temp);
   this->mSignificand *= temp.mSignificand;
   return *this;
}

BigDecimal& BigDecimal::operator/=(const BigDecimal& rhs)
{
   // FIXME: this one is more complicated
   BigDecimal temp = rhs;
   synchronizeExponents(*this, temp);
   this->mSignificand /= temp.mSignificand;
   return *this;
}

BigDecimal& BigDecimal::powEquals(const BigDecimal& rhs)
{
   BigDecimal temp = rhs;
   synchronizeExponents(*this, temp);
   this->mSignificand.powEquals(temp.mSignificand);
   return *this;
}

BigDecimal& BigDecimal::operator%=(const BigDecimal& rhs)
{
   BigDecimal temp = rhs;
   synchronizeExponents(*this, temp);
   this->mSignificand %= temp.mSignificand;
   return *this;
}

bool BigDecimal::isZero()
{
   return mSignificand.isZero();
}

bool BigDecimal::isNegative() const
{
   return mSignificand.isNegative();
}

long double BigDecimal::getDouble()
{
   long double rval = mSignificand.getInt64();
   //rval *= 10^-mExponent
   
   if(isNegative())
   {
      rval = -rval;
   }
   
   return rval;
}
