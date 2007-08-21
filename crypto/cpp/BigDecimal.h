/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#ifndef db_crypto_BigDecimal_H
#define db_crypto_BigDecimal_H

#include <openssl/bn.h>

#include "BigInteger.h"

namespace db
{
namespace crypto
{

/**
 * A BigDecimal is an arbitrary precision decimal number.
 * 
 * The value of a BigDecimal is stored with exponential notation using negative
 * powers of 10. A signed BigInteger "a" is used to store the significand (or
 * coefficient) and an unsigned BigInteger "b" is used to store the exponent.
 * 
 * The value of a BigDecimal is:
 * 
 * value = a x 10^-b
 * 
 * Where a is the significand and b is the exponent. The value of b specifies
 * where to place the decimal for the value of a. The larger the exponent,
 * the smaller the value. For instance, if b = 2 and a = 514, then the value
 * of the BigDecimal is 5.14. If b = 5 and a = 514, then the value of the
 * BigDecimal would be 0.00514.
 * 
 * @author Dave Longley
 */
class BigDecimal
{
protected:
   /**
    * The significand for the value.
    */
   BigInteger mSignificand;
   
   /**
    * The exponent for the value.
    */
   BigInteger mExponent;
   
   /**
    * Makes the exponents equal for the passed two BigDecimals. The larger
    * exponent will be chosen so as to retain precision.
    * 
    * @param bd1 the first BigDecimal.
    * @param bd2 the second BigDecimal.
    */
   void synchronizeExponents(BigDecimal& bd1, BigDecimal& bd2);
   
public:
   /**
    * Creates a new BigDecimal with the specified value.
    * 
    * @param value the value for this BigDecimal.
    */
   BigDecimal(long double value = 0);
   
   /**
    * Creates a new BigDecimal by copying another one.
    * 
    * @param bn the BigDecimal to copy.
    */
   BigDecimal(const BigDecimal& bn);
   
   /**
    * Destructs this BigDecimal.
    */
   virtual ~BigDecimal();
   
   /**
    * Sets this BigDecimal equal to the passed one.
    * 
    * @param rhs the BigDecimal to set this one equal to.
    * 
    * @return this BigDecimal.
    */
   BigDecimal& operator=(const BigDecimal& rhs);
   
   /**
    * Sets this BigDecimal's value to the passed value.
    * 
    * @param value the new value for this BigDecimal.
    * 
    * @return this BigDecimal.
    */
   BigDecimal& operator=(long double rhs);
   
   /**
    * Returns true if this BigDecimal is equal to the passed one.
    * 
    * @param rhs the BigDecimal to compare to this one.
    * 
    * @return true if this BigDecimal is equal to the passed one, false if not.
    */
   bool operator==(const BigDecimal& rhs);
   
   /**
    * Returns true if this BigDecimal is equal to the passed double.
    * 
    * @param rhs the double to compare to this BigDecimal.
    * 
    * @return true if this BigDecimal is equal to the passed double,
    *         false if not.
    */
   bool operator==(long double rhs);
   
   /**
    * Returns true if this BigDecimal is not equal to the passed one.
    * 
    * @param rhs the BigDecimal to compare to this one.
    * 
    * @return true if this BigDecimal is not equal to the passed one, false if
    *         not.
    */
   bool operator!=(const BigDecimal& rhs);
   
   /**
    * Returns true if this BigDecimal is not equal to the passed double.
    * 
    * @param rhs the double to compare to this BigDecimal.
    * 
    * @return true if this BigDecimal is not equal to the passed double,
    *         false if not.
    */
   bool operator!=(long double rhs);
   
   /**
    * Returns true if this BigDecimal is less than the passed one.
    * 
    * @param rhs the BigDecimal to compare to this one.
    * 
    * @return true if this BigDecimal is less than the passed one, false if not.
    */
   bool operator<(const BigDecimal& rhs);
   
   /**
    * Returns true if this BigDecimal is greater than the passed one.
    * 
    * @param rhs the BigDecimal to compare to this one.
    * 
    * @return true if this BigDecimal is greater than the passed one, false
    *         if not.
    */
   bool operator>(const BigDecimal& rhs);
   
   /**
    * Returns true if this BigDecimal is less than or equal to the passed one.
    * 
    * @param rhs the BigDecimal to compare to this one.
    * 
    * @return true if this BigDecimal is less than or equal to the passed one,
    *         false if not.
    */
   bool operator<=(const BigDecimal& rhs);
   
   /**
    * Returns true if this BigDecimal is greater than or equal to the passed one.
    * 
    * @param rhs the BigDecimal to compare to this one.
    * 
    * @return true if this BigDecimal is greater than or equal to the passed one,
    *         false if not.
    */
   bool operator>=(const BigDecimal& rhs);
   
   /**
    * Returns the result of the passed BigDecimal added to this one.
    * 
    * @param rhs the BigDecimal to add to this one.
    * 
    * @return the sum of this BigDecimal and the passed one.
    */
   BigDecimal operator+(const BigDecimal& rhs);
   
   /**
    * Returns the result of the passed BigDecimal subtracted from this one.
    * 
    * @param rhs the BigDecimal to substract from this one.
    * 
    * @return the difference between this BigDecimal and the passed one.
    */
   BigDecimal operator-(const BigDecimal& rhs);
   
   /**
    * Returns the result of the passed BigDecimal multiplied by this one.
    * 
    * @param rhs the BigDecimal to multiply by this one.
    * 
    * @return the product of this BigDecimal and the passed one.
    */
   BigDecimal operator*(const BigDecimal& rhs);
   
   /**
    * Returns the result of dividing this BigDecimal by the passed one.
    * 
    * @param rhs the BigDecimal to divide into this one.
    * 
    * @return the quotient of this BigDecimal and the passed one.
    */
   BigDecimal operator/(const BigDecimal& rhs);
   
   /**
    * Returns the result of this BigDecimal raised to the power of the passed
    * one.
    * 
    * @param rhs the BigDecimal to raise this one by.
    * 
    * @return the result of this BigDecimal raised by the passed one.
    */
   BigDecimal pow(const BigDecimal& rhs);
   
   /**
    * Returns this BigDecimal modulo the passed one.
    * 
    * @param rhs the BigDecimal to mod this one by.
    * 
    * @return the modulus of the passed BigDecimal respective to this one.
    */
   BigDecimal operator%(const BigDecimal& rhs);
   
   /**
    * Adds the passed BigDecimal to this one and returns the result.
    * 
    * @param rhs the BigDecimal to add to this one.
    * 
    * @return the new value of this BigDecimal.
    */
   BigDecimal& operator+=(const BigDecimal& rhs);
   
   /**
    * Subtracts the passed BigDecimal from this one and returns the result.
    * 
    * @param rhs the BigDecimal to substract from this one.
    * 
    * @return the new value of this BigDecimal.
    */
   BigDecimal& operator-=(const BigDecimal& rhs);
   
   /**
    * Multiplies the passed BigDecimal by this one and returns the result.
    * 
    * @param rhs the BigDecimal to multiply by this one.
    * 
    * @return the new value of this BigDecimal.
    */
   BigDecimal& operator*=(const BigDecimal& rhs);
   
   /**
    * Divides the passed BigDecimal into this one and returns the result.
    * 
    * @param rhs the BigDecimal to divide into this one.
    * 
    * @return the new value of this BigDecimal.
    */
   BigDecimal& operator/=(const BigDecimal& rhs);
   
   /**
    * Raises this BigDecimal to the power of the passed one and returns the
    * result.
    * 
    * @param rhs the BigDecimal to raise this one by.
    * 
    * @return the new value of this BigDecimal.
    */
   BigDecimal& powEquals(const BigDecimal& rhs);
   
   /**
    * Sets this BigDecimal to the modulus of the passed one.
    * 
    * @param rhs the BigDecimal to mod this one by.
    * 
    * @return the new value of this BigDecimal.
    */
   BigDecimal& operator%=(const BigDecimal& rhs);
   
   /**
    * Returns true if this BigInteger is zero, false if not.
    * 
    * @return true if this BigInteger is zero, false if not.
    */
   bool isZero();
   
   /**
    * Returns true if this BigInteger is negative, false if not.
    * 
    * @return true if this BigInteger is negative, false if not.
    */
   bool isNegative() const;
   
   /**
    * Gets the value of this BigInteger as a double.
    * 
    * @return the value of this BigInteger as a double.
    */
   long double getDouble();
};

} // end namespace crypto
} // end namespace db
#endif
