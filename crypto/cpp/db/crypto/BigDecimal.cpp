/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/crypto/BigDecimal.h"

#include <sstream>
#include <cstdlib>

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

BigDecimal::BigDecimal(double value)
{
   initialize();
   
   if(value != 0)
   {
      *this = value;
   }
}

BigDecimal::BigDecimal(long long value)
{
   initialize();
   
   if(value != 0)
   {
      *this = value;
   }
}

BigDecimal::BigDecimal(unsigned long long value)
{
   initialize();
   
   if(value != 0)
   {
      *this = value;
   }
}

BigDecimal::BigDecimal(int value)
{
   initialize();
   
   if(value != 0)
   {
      *this = value;
   }
}

BigDecimal::BigDecimal(unsigned int value)
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
   initialize();
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
   if(exponent > mExponent && !mSignificand.isZero())
   {
      // multiply significand by power difference
      BigInteger ten(10);
      mSignificand *= ten.pow(exponent - mExponent);
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

BigDecimal& BigDecimal::operator=(double rhs)
{
   // convert double to string
   ostringstream oss;
   oss << rhs;
   return *this = oss.str();
}

BigDecimal& BigDecimal::operator=(long long rhs)
{
   // convert long to string
   char temp[22];
   snprintf(temp, 22, "%lli", rhs);
   return *this = temp;
}

BigDecimal& BigDecimal::operator=(unsigned long long rhs)
{
   // convert long to string
   char temp[22];
   snprintf(temp, 22, "%llu", rhs);
   return *this = temp;
}

BigDecimal& BigDecimal::operator=(int rhs)
{
   // convert int to string
   char temp[12];
   snprintf(temp, 12, "%i", rhs);
   return *this = temp;
}

BigDecimal& BigDecimal::operator=(unsigned int rhs)
{
   // convert int to string
   char temp[12];
   snprintf(temp, 12, "%u", rhs);
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
   if(mExponent < 0)
   {
      mExponent = -mExponent;
      BigInteger ten(10);
      mSignificand *= ten.pow(mExponent);
      mExponent = 0;
   }
   else
   {
      // minimize the exponent
      while(mSignificand % 10 == 0)
      {
         mSignificand /= 10;
         mExponent--;
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

long double BigDecimal::getDouble() const
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
   string str = toString();
   
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
         if(mRoundingMode == Up)
         {
            // must round up if there is anything but a zero in extra
            bool round = false;
            for(const char* ptr = extra.c_str(); *ptr != 0 && !round; ptr++)
            {
               if(*ptr != '0')
               {
                  round = true;
               }
            }
            
            if(round)
            {
               // add 1 with the same exponent
               BigDecimal bd = 1;
               bd.mExponent = mExponent;
               *this += bd;
            }
         }
         else if(mRoundingMode == HalfUp)
         {
            if(extra.at(0) > '4' && extra.at(0) <= '9')
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

string BigDecimal::toString(bool zeroFill) const
{
   // Note: This implementation will print out only the significant digits,
   // up to a maximum of the set precision.
   
   // write out significand
   string str = mSignificand.toString();
   
   // erase negative sign
   if(mSignificand.isNegative())
   {
      str.erase(0, 1);
   }
   
   if(mExponent < 0)
   {
      // append zeros
      int zeros = -mExponent - str.length();
      if(zeros > 0)
      {
         str.append(zeros, '0');
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
   
   // cut the string to the last significant digit
   string::size_type pos = str.find_last_not_of('0');
   if(pos != string::npos)
   {
      if(str[pos] != '.')
      {
         // pos points to a number, so cut after that number
         if(pos > str.length() + 1)
         {
            str.erase(pos + 1, str.length() - pos - 1);
         }
      }
      else
      {
         // cut decimal and all other digits (all zeros)
         str.erase(pos, str.length() - pos);
      }
   }
   
   // insert negative sign
   if(mSignificand.isNegative())
   {
      str.insert(0, 1, '-');
   }
   
   if(zeroFill)
   {
      pos = str.find('.');
      if(pos != string::npos)
      {
         // determine number of digits after decimal point
         string::size_type count = str.length() - pos - 1;
         
         // alter string by difference between count and set precision
         if(count < mPrecision)
         {
            // zero-fill
            str.append(mPrecision - count, '0');
         }
         else
         {
            // cut off extra digits
            str.erase(pos + mPrecision + 1);
         }
      }
      else if(mPrecision > 0)
      {
         // add decimal place and zero-fill
         str.push_back('.');
         str.append(mPrecision, '0');
      }
   }
   
   return str;
}

ostream& operator<<(ostream& os, const BigDecimal& bd)
{
   os << bd.toString();
   return os;
}

istream& operator>>(istream& is, BigDecimal& bd)
{
   string str;
   is >> str; 
   bd = str;
   return is;
}
