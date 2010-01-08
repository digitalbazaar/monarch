/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/crypto/BigDecimal.h"

#include <cstdlib>

using namespace std;
using namespace monarch::crypto;

BigDecimal::BigDecimal(double value)
{
   initialize();

   if(value != 0)
   {
      *this = value;
   }
}

BigDecimal::BigDecimal(int64_t value)
{
   initialize();

   if(value != 0)
   {
      *this = value;
   }
}

BigDecimal::BigDecimal(uint64_t value)
{
   initialize();

   if(value != 0)
   {
      *this = value;
   }
}

BigDecimal::BigDecimal(int32_t value)
{
   initialize();

   if(value != 0)
   {
      *this = value;
   }
}

BigDecimal::BigDecimal(uint32_t value)
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

BigDecimal::BigDecimal(const BigDecimal& copy)
{
   mSignificand = copy.mSignificand;
   mExponent = copy.mExponent;
   mPrecision = copy.mPrecision;
   mRoundingMode = copy.mRoundingMode;
}

BigDecimal::~BigDecimal()
{
}

void BigDecimal::initialize()
{
   mExponent = 0;
   mPrecision = 10;
   mRoundingMode = HalfUp;
}

void BigDecimal::setExponent(int exponent)
{
   if(exponent != mExponent)
   {
      // If mSignificand is zero can skip the adjustment but need to change the
      // exponent so syncronizeExponents() has the correct behavior.
      if(!mSignificand.isZero())
      {
         BigInteger ten(10);
         BigInteger pow = ten.pow(exponent - mExponent);
         if(exponent > mExponent)
         {
            // multiply significand by power difference
            // no loss of accuracy
            mSignificand *= pow;
         }
         else
         {
            // divide significand by power difference
            // may result in loss of accuracy
            mSignificand /= pow;
         }
      }

      mExponent = exponent;
   }
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

BigDecimal& BigDecimal::operator=(double rhs)
{
   // convert double to string
   char temp[1024];
   snprintf(temp, 1024, "%.*f", mPrecision, rhs);
   return *this = temp;
}

BigDecimal& BigDecimal::operator=(int64_t rhs)
{
   // convert int64_t to string
   char temp[22];
   snprintf(temp, 22, "%" PRIi64, rhs);
   return *this = temp;
}

BigDecimal& BigDecimal::operator=(uint64_t rhs)
{
   // convert uint64_t to string
   char temp[22];
   snprintf(temp, 22, "%" PRIu64, rhs);
   return *this = temp;
}

BigDecimal& BigDecimal::operator=(int32_t rhs)
{
   // convert int32_t to string
   char temp[12];
   snprintf(temp, 12, "%" PRIi32, rhs);
   return *this = temp;
}

BigDecimal& BigDecimal::operator=(uint32_t rhs)
{
   // convert uint32_t to string
   char temp[12];
   snprintf(temp, 12, "%" PRIu32, rhs);
   return *this = temp;
}

BigDecimal& BigDecimal::operator=(const char* rhs)
{
   string s = rhs;
   *this = s;
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
      BigInteger ten(10);
      mSignificand *= ten.pow(mExponent);
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

bool BigDecimal::operator==(double rhs)
{
   return getDouble() == rhs;
}

bool BigDecimal::operator!=(const BigDecimal& rhs)
{
   return !(*this == rhs);
}

bool BigDecimal::operator!=(double rhs)
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
   BigDecimal rval;

   if(mSignificand.isZero())
   {
      rval = 0;
   }
   else
   {
      rval = *this;

      // ensure exponent is large enough to include precision
      // (this does not change the value of rval)
      rval.setExponent(rval.mPrecision + mPrecision + rhs.mPrecision);

      // do division with remainder
      BigDecimal remainder;
      rval.mSignificand.divide(
         rhs.mSignificand, rval.mSignificand, remainder.mSignificand);

      // when dividing exponential numbers, subtract the exponents
      rval.mExponent -= rhs.mExponent;

      // if exponent is negative, scale the significand so the exponent is zero
      // FIXME: these cases reverse the exponent to be like the other case
      //        ie, force exp up to zero or force it possibly below zero
      //
      // FIXME: A degenerate case is when multiple divisions occur with
      //        intermediate results that cannot be reduced with the logic
      //        below.  Each result will have an increasing exponent (set
      //        above) that may not shrink.  Calling round() will fix this but
      //        may not be appropriate here if the user wants higher precision
      //        intermediate results in a calculation.
      if(rval.mSignificand.isZero())
      {
         rval.mExponent = 0;
      }
      else if(rval.mExponent < 0)
      {
         BigInteger ten(10);
         rval.mSignificand *= ten.pow(-rval.mExponent);
         rval.mExponent = 0;
      }
      else
      {
         // minimize the exponent
         BigInteger ten(10);
         while((rval.mSignificand % ten).isZero())
         {
            rval.mSignificand /= ten;
            rval.mExponent--;
         }
      }
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

double BigDecimal::getDouble() const
{
   // get value as a string
   // parse long double
   return strtold(toString().c_str(), NULL);
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
   string str = toString(false, false);

   // find exponent
   unsigned int dot = str.rfind('.');
   if(dot != string::npos)
   {
      // determine if there are more digits than the precision allows
      if(str.length() - (dot + 1) > mPrecision)
      {
         // get the extra digits
         string extra = str.substr(dot + 1 + mPrecision);

         // set new exponent to the precision
         mExponent = mPrecision;

         // truncate significand
         mSignificand = (str.substr(0, dot) + str.substr(dot + 1, mExponent));

         // round significand according to rounding mode
         bool roundUp = false;
         switch(mRoundingMode)
         {
            case Down:
               // do nothing
               break;
            case Up:
               // round up if there is a non-zero in extra
               roundUp = (extra.find_first_not_of('0') != string::npos);
               break;
            case HalfUp:
               // round up if extra starts with 5 or greater
               roundUp = (extra[0] >= '5');
               break;
            case HalfEven:
               // round up if next digit in [6,9]
               roundUp = (extra[0] >= '6');

               if(!roundUp && extra[0] == '5')
               {
                  // round up if next digit of 5 is followed by non-zero
                  roundUp = (extra.find_first_not_of('0', 1) != string::npos);

                  // '5' followed by zeros, check current digit for even/odd
                  if(!roundUp)
                  {
                     // get the rounding digit from non-extra
                     // scan backwards for first number
                     // assume '0' and no roundUp
                     const char* start = str.c_str();
                     const char* ptr = start + dot + mPrecision;
                     // check current if digit, else previous, else '0'
                     // covers current == '.' and current == start cases
                     const char current = (*ptr >= '0' && *ptr <= '9') ?
                        *ptr : ((ptr != start) ? *(ptr - 1) : '0');
                     // up on odd, nothing on even
                     roundUp = ((current - '0') & 0x1);
                  }
               }
               break;
         }

         if(roundUp)
         {
            // add 1 with the proper sign and same exponent
            BigDecimal bd = mSignificand.isNegative() ? -1 : 1;
            bd.mExponent = mExponent;
            *this += bd;
         }
      }
   }
}

string BigDecimal::toString(bool zeroFill, bool truncate) const
{
   // Note: This implementation will print out only the significant digits,
   // up to a maximum of the set precision.

   // write out significand
   string str = mSignificand.toString();

   // pretend exponent is zero if significand is zero
   int exponent = mSignificand.isZero() ? 0 : mExponent;

   // remove non-significant trailing zeros for positive exponents
   // the zeros may be added back later for zerofill
   if(exponent > 0)
   {
      string::size_type trailingZerosStart = str.find_last_not_of('0') + 1;
      if(trailingZerosStart > 0 && trailingZerosStart < str.length())
      {
         exponent -= (str.length() - trailingZerosStart);
         str.erase(trailingZerosStart);
      }
   }

   if(exponent <= 0)
   {
      // append zeros
      if(exponent < 0)
      {
         str.append(-exponent, '0');
      }

      // zero fill
      if(zeroFill && mPrecision > 0)
      {
         // add decimal place and zero-fill
         str.push_back('.');
         str.append(mPrecision, '0');
      }
   }
   else // mExponent > 0
   {
      // adjustment for potential negative sign
      int start = (mSignificand.isNegative() ? 1 : 0);
      // count of current precision
      unsigned int precisionCount;

      // decimal point position
      int pointPos = str.length() - exponent;
      // check that at least one pre-point digit is available
      if(pointPos >= (start + 1))
      {
         precisionCount = exponent;
         // insert decimal point
         str.insert(pointPos, 1, '.');
      }
      else
      {
         // add "0."
         str.insert(start, "0.", 2);
         // add pre-zeros
         str.insert(start + 2, start - pointPos, '0');
         precisionCount = str.length() - start - 2;
      }

      if(precisionCount < mPrecision)
      {
         // need more digits
         if(zeroFill)
         {
            // zero fill
            str.append(mPrecision - precisionCount, '0');
         }
      }
      else if(precisionCount > mPrecision)
      {
         // need less digits
         if(truncate)
         {
            str.erase(str.length() - precisionCount + mPrecision);
         }
      }
   }

   return str;
}

void BigDecimal::_setValue(BigInteger& significand, int exponent)
{
   mSignificand = significand;
   mExponent = exponent;
}
