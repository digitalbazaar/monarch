/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "BigDecimal.h"

#include <sstream>

using namespace std;
using namespace db::crypto;

BigDecimal::BigDecimal(long double value)
{
   initialize();
   
   if(value != 0)
   {
      *this = value;
   }
}

BigDecimal::BigDecimal(const char* value)
{
   initialize();
   *this = value;
}

BigDecimal::BigDecimal(const string& value)
{
   initialize();
   *this = value;
}

BigDecimal::BigDecimal(const BigDecimal& rhs)
{
   initialize();
   mSignificand = rhs.mSignificand;
   mExponent = rhs.mExponent;
}

BigDecimal::~BigDecimal()
{
}

void BigDecimal::initialize()
{
   mPrecision = 10;
   mRoundingMode = HALF_UP;
}

void BigDecimal::setExponent(int exponent)
{
   if(exponent > mExponent)
   {
      // multiply significand by power difference
      mSignificand *= BigInteger::TEN.pow(exponent - mExponent);
   }
   
   mExponent = exponent;
}

void BigDecimal::synchronizeExponents(BigDecimal& bd1, BigDecimal& bd2)
{
   // only do work if exponents are different
   if(bd1.mExponent != bd2.mExponent)
   {
      // use the larger exponent to retain precision
      if(bd1.mExponent > bd2.mExponent)
      {
         bd2.setExponent(bd1.mExponent);
      }
      else
      {
         bd1.setExponent(bd2.mExponent);
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
   // convert double to string
   ostringstream oss;
   oss << rhs;
   return *this = oss.str();
}

BigDecimal& BigDecimal::operator=(const char* rhs)
{
   *this = string(rhs);
   return *this;
}

BigDecimal& BigDecimal::operator=(const string& rhs)
{
   string temp;
   
   // find decimal point
   unsigned int dot = rhs.rfind('.');
   if(dot != string::npos)
   {
      // check for scientific notation
      unsigned int e = rhs.rfind('e');
      if(e != string::npos && e != rhs.length() - 2)
      {
         // parse exponent
         mExponent = -strtoll(rhs.c_str() + e + 1, NULL, 10);
         
         // add number of places between the e and the decimal point
         mExponent += e - dot - 1;
         
         // remove decimal point and e
         temp.append(rhs.substr(0, dot));
         temp.append(rhs.substr(dot + 1, e - dot));
      }
      else
      {
         // set exponent to the number of places between dot and end of string
         mExponent = rhs.length() - dot - 1;
         
         // remove decimal point
         temp.append(rhs.substr(0, dot));
         if(dot != rhs.length() - 1)
         {
            temp.append(rhs.substr(dot + 1));
         }
      }
   }
   else
   {
      // no decimal point, set exponent to 0
      mExponent = 0;
      temp = rhs;
   }
   
   // parse significand
   mSignificand = temp;
   
   // if exponent is negative, scale the significand so the exponent is zero
   if(mExponent < 0)
   {
      mExponent = -mExponent;
      mSignificand *= BigInteger::TEN.pow(mExponent);
      mExponent = 0;
   }
   
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
   
   // perform multiplication and then add exponents
   rval.mSignificand *= rhs.mSignificand;
   rval.mExponent += rhs.mExponent;
   
   return rval;
}

BigDecimal BigDecimal::operator/(const BigDecimal& rhs)
{
   BigDecimal rval = *this;
   
   // add the divisor's exponent to the dividend so that when a division is
   // performed, the exponents subtract to reproduce the original scale
   rval.setExponent(rval.mExponent + rhs.mExponent);
   
   // do division with remainder
   BigDecimal remainder;
   rval.mSignificand.divide(
      rhs.mSignificand, rval.mSignificand, remainder.mSignificand);
   
   // see if there is a remainder to add to the result
   if(remainder.mSignificand != 0)
   {
      // determine if the remainder should be rounded up
      bool roundUp = (mRoundingMode == UP) ? true : false;
      if(mRoundingMode == HALF_UP)
      {
         // if twice the remainder is greater than or equal to the divisor,
         // then it is at least half as large as the divisor
         if((remainder.mSignificand + remainder.mSignificand).absCompare(
            rhs.mSignificand) >= 0)
         {
            roundUp = true;
         }
      }
      
      // raise remainder to digits of precision (taking into account the
      // remainder has the same scale as the dividend rval)
      unsigned int digits = 0;
      if(mPrecision - rval.mExponent > 0)
      {
         digits = mPrecision - rval.mExponent;
         remainder.mSignificand *= BigInteger::TEN.pow(digits);
      }
      
      // perform division on significand
      remainder.mSignificand /= rhs.mSignificand;
      
      // set remainder exponent to digits of precision
      remainder.mExponent = mPrecision;
      
      // round up if appropriate
      if(roundUp)
      {
         BigDecimal bd;
         bd.mSignificand = 1;
         bd.mExponent = mPrecision;
         rval += bd;
      }
      
      // add remainder
      rval += remainder;
   }
   
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
   *this = *this + rhs;
   return *this;
}

BigDecimal& BigDecimal::operator-=(const BigDecimal& rhs)
{
   *this = *this - rhs;
   return *this;
}

BigDecimal& BigDecimal::operator*=(const BigDecimal& rhs)
{
   *this = *this * rhs;
   return *this;
}

BigDecimal& BigDecimal::operator/=(const BigDecimal& rhs)
{
   *this = *this / rhs;
   return *this;
}

BigDecimal& BigDecimal::operator%=(const BigDecimal& rhs)
{
   *this = *this % rhs;
   return *this;
}

bool BigDecimal::isZero() const
{
   return mSignificand.isZero();
}

void BigDecimal::setNegative(bool negative)
{
   mSignificand.setNegative(negative);
}

bool BigDecimal::isNegative() const
{
   return mSignificand.isNegative();
}

long double BigDecimal::getDouble() const
{
   // get value as a string
   string str;
   toString(str);
   
   // parse long double
   return strtold(str.c_str(), NULL);
}

void BigDecimal::setPrecision(unsigned int precision, RoundingMode roundingMode)
{
   mPrecision = precision;
   mRoundingMode = roundingMode;
}

unsigned int BigDecimal::getPrecision()
{
   return mPrecision;
}

void BigDecimal::round()
{
   // write out to a string
   string str;
   toString(str);
   
   // find exponent
   unsigned int dot = str.rfind('.');
   if(dot != string::npos)
   {
      // determine if there are more digits than the precision allows
      if(str.length() - (dot + 1) > mPrecision)
      {
         // get the extra digits
         string extra = str.substr(dot + 1 + mPrecision);
         
         // set new exponent by subtracting extra length
         mExponent -= extra.length();
         
         // truncate significand
         mSignificand = (str.substr(0, dot) + str.substr(dot + 1, mExponent));
         
         // round significand according to rounding mode
         if(mRoundingMode == UP)
         {
            // add 1 with the same exponent
            BigDecimal bd = 1;
            bd.mExponent = mExponent;
            *this += bd;
         }
         else if(mRoundingMode == HALF_UP)
         {
            // (52 = '4', 57 = '9')
            if(extra.at(0) > 52 && extra.at(0) <= 57)
            {
               // add 1 with the same exponent
               BigDecimal bd = 1;
               bd.mExponent = mExponent;
               *this += bd;
            }
         }
      }
   }
}

string& BigDecimal::toString(string& str) const
{
   // write out significand
   mSignificand.toString(str);
   
   if(mExponent < 0)
   {
      // append zeros
      int zeros = -mExponent - str.length();
      if(zeros > 0)
      {
         str.append(0, zeros, '0');
      }
      else
      {
         // insert decimal point
         str.insert(str.length() + mExponent, 1, '.');
      }
   }
   else if(mExponent > 0)
   {
      // prepend zeros
      int zeros = mExponent - str.length();
      if(zeros > 0)
      {
         str.insert(0, zeros, '0');
      }
      
      if((unsigned int)mExponent == str.length())
      {
         // prepend "0."
         str.insert(0, 1, '.');
         str.insert(0, 1, '0');
      }
      else
      {
         // insert decimal point
         str.insert(str.length() - mExponent, 1, '.');
      }
   }
   
   return str;
}

ostream& operator<<(ostream& os, const BigDecimal& bd)
{
   string str;
   os << bd.toString(str);
   return os;
}

istream& operator>>(istream& is, BigDecimal& bd)
{
   string str;
   is >> str; 
   bd = str;
   return is;
}
