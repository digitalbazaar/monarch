/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "BigDecimal.h"

#include <sstream>

using namespace std;
using namespace db::crypto;

BigDecimal::BigDecimal(long double value)
{
   if(value != 0)
   {
      *this = value;
   }
}

BigDecimal::BigDecimal(const char* value)
{
   *this = value;
}

BigDecimal::BigDecimal(const string& value)
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
         bd2.mSignificand *= BigInteger::TEN.pow(bd1.mExponent - bd2.mExponent);
         bd2.mExponent = bd1.mExponent;
      }
      else
      {
         // increase significand by power difference
         bd1.mSignificand *= BigInteger::TEN.pow(bd2.mExponent - bd1.mExponent);
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
   if(mExponent.isNegative())
   {
      mExponent.setNegative(false);
      mSignificand *= (BigInteger::TEN.powEquals(mExponent));
      mExponent = 0;
   }
   
   cout << "str=" << rhs << ", significand=" << mSignificand
      << ", exponent=" << mExponent << endl;
   
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

string& BigDecimal::toString(string& str) const
{
   if(mExponent.isCompact())
   {
      // write out significand
      mSignificand.toString(str);
      
      int e = mExponent.getInt64();
      if(e < 0)
      {
         // append zeros
         int zeros = -e - str.length();
         if(zeros > 0)
         {
            str.append(0, zeros, '0');
         }
         else
         {
            // insert decimal point
            str.insert(str.length() + e, 1, '.');
         }
      }
      else if(e > 0)
      {
         // prepend zeros
         int zeros = e - str.length();
         if(zeros > 0)
         {
            str.insert(0, zeros, '0');
         }
         
         if((unsigned int)e == str.length())
         {
            // prepend "0."
            str.insert(0, 1, '.');
            str.insert(0, 1, '0');
         }
         else
         {
            // insert decimal point
            str.insert(str.length() - e, 1, '.');
         }
      }
      
      // handle decimal point accuracy
      // FIXME:
   }
   else
   {
      // FIXME: determine how to handle this
      str = "0";
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
