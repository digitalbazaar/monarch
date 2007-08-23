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
 * A RoundingMode specifies a type of decimal rounding.
 */
typedef enum RoundingMode
{
   Up, HalfUp, Down
};

/**
 * A BigDecimal is a multi/arbitrary precision decimal number. If the number
 * is used to perform arithmetic that may require rounding, like 1 / 3, then
 * the number of digits of precision must be set via setPrecision(). The
 * default precision is 10.
 * 
 * The value of a BigDecimal is stored with exponential notation using negative
 * powers of 10. A signed BigInteger "a" is used to store the significand (or
 * coefficient) and a signed 32-bit int "b" is used to store the exponent.
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
 * Note: Might need to change operators to friends for STL.
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
   int mExponent;
   
   /**
    * The precision (number of digits) for this BigDecimal, if arithmetic
    * requires rounding.
    */
   unsigned int mPrecision;
   
   /**
    * The rounding mode for this BigDecimal, if arithmetic requires rounding.
    * 
    * Defaults to HalfUp.
    */
   RoundingMode mRoundingMode;
   
   /**
    * Initializes this BigDecimal.
    */
   void initialize();
   
   /**
    * Sets the exponent for this BigDecimal, increasing the significand if
    * necessary. This will not alter the value of this BigDecimal.
    * 
    * @param exponent the new exponent to use.
    */
   void setExponent(int exponent);
   
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
    * Creates a new BigDecimal with the specified value.
    * 
    * @param value the value for this BigDecimal.
    */
   BigDecimal(double value);
   
   /**
    * Creates a new BigDecimal with the specified value.
    * 
    * @param value the value for this BigDecimal.
    */
   BigDecimal(long long value);
   
   /**
    * Creates a new BigDecimal with the specified value.
    * 
    * @param value the value for this BigDecimal.
    */
   BigDecimal(unsigned long long value);
   
   /**
    * Creates a new BigDecimal with the specified value.
    * 
    * @param value the value for this BigDecimal.
    */
   BigDecimal(int value);
   
   /**
    * Creates a new BigDecimal with the specified value.
    * 
    * @param value the value for this BigDecimal.
    */
   BigDecimal(unsigned int value);
   
   /**
    * Creates a new BigDecimal with the specified value.
    * 
    * @param value the value for this BigDecimal.
    */
   BigDecimal(const char* value);
   
   /**
    * Creates a new BigDecimal with the specified value.
    * 
    * @param value the value for this BigDecimal.
    */
   BigDecimal(const std::string& value);
   
   /**
    * Creates a new BigDecimal by copying another one.
    * 
    * @param copy the BigDecimal to copy.
    */
   BigDecimal(const BigDecimal& copy);
   
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
    * Sets this BigDecimal's value to the passed value.
    * 
    * @param value the new value for this BigDecimal.
    * 
    * @return this BigDecimal.
    */
   BigDecimal& operator=(double rhs);
   
   /**
    * Sets this BigDecimal's value to the passed value.
    * 
    * @param value the new value for this BigDecimal.
    * 
    * @return this BigDecimal.
    */
   BigDecimal& operator=(long long rhs);
   
   /**
    * Sets this BigDecimal's value to the passed value.
    * 
    * @param value the new value for this BigDecimal.
    * 
    * @return this BigDecimal.
    */
   BigDecimal& operator=(unsigned long long rhs);
   
   /**
    * Sets this BigDecimal's value to the passed value.
    * 
    * @param value the new value for this BigDecimal.
    * 
    * @return this BigDecimal.
    */
   BigDecimal& operator=(int rhs);
   
   /**
    * Sets this BigDecimal's value to the passed value.
    * 
    * @param value the new value for this BigDecimal.
    * 
    * @return this BigDecimal.
    */
   BigDecimal& operator=(unsigned int rhs);
   
   /**
    * Sets this BigDecimal's value to the passed value.
    * 
    * @param value the new value for this BigDecimal.
    * 
    * @return this BigDecimal.
    */
   BigDecimal& operator=(const char* rhs);
   
   /**
    * Sets this BigDecimal's value to the passed value.
    * 
    * @param value the new value for this BigDecimal.
    * 
    * @return this BigDecimal.
    */
   BigDecimal& operator=(const std::string& rhs);
   
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
    * Sets this BigDecimal to the modulus of the passed one.
    * 
    * @param rhs the BigDecimal to mod this one by.
    * 
    * @return the new value of this BigDecimal.
    */
   BigDecimal& operator%=(const BigDecimal& rhs);
   
   /**
    * Returns true if this BigDecimal is zero, false if not.
    * 
    * @return true if this BigDecimal is zero, false if not.
    */
   bool isZero() const;
   
   /**
    * Sets whether or not this BigDecimal is negative.
    * 
    * @param negative true if this BigDecimal should be negative, false if not.
    */
   void setNegative(bool negative);
   
   /**
    * Returns true if this BigDecimal is negative, false if not.
    * 
    * @return true if this BigDecimal is negative, false if not.
    */
   bool isNegative() const;
   
   /**
    * Gets the value of this BigDecimal as a double.
    * 
    * @return the value of this BigDecimal as a double.
    */
   long double getDouble() const;
   
   /**
    * Sets the number of digits of precision for this BigDecimal for
    * arithmetic operations that require rounding.
    * 
    * @param precision the number of digits of precision to use.
    * @param roundingMode the RoundingMode to use.
    */
   void setPrecision(unsigned int precision, RoundingMode roundingMode);
   
   /**
    * Gets the number of digits of precision for this BigDecimal for
    * arithmetic operations that require rounding.
    * 
    * @return the number of digits of precision to use.
    */
   unsigned int getPrecision();
   
   /**
    * Rounds this BigDecimal according to its set precision.
    */
   void round();
   
   /**
    * Gets the value of this BigInteger as a string.
    * 
    * @param str string to store the value of this BigInteger in.
    * 
    * @return a reference to the string.
    */
   std::string& toString(std::string& str) const;
};

} // end namespace crypto
} // end namespace db

/**
 * Writes a BigDecimal to an ostream.
 * 
 * @param os the ostream to write to.
 * @param bd the BigDecimal to write.
 * 
 * @return the ostream.
 */
std::ostream& operator<<(std::ostream& os, const db::crypto::BigDecimal& bd);

/**
 * Reads a BigDecimal from an istream.
 * 
 * @param is the istream to read from.
 * @param bi the BigDecimal to populate.
 * 
 * @return the istream.
 */
std::istream& operator>>(std::istream& is, db::crypto::BigDecimal& bd);

#endif
