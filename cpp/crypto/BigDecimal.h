/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_crypto_BigDecimal_H
#define monarch_crypto_BigDecimal_H

#include <openssl/bn.h>

#include "monarch/crypto/BigInteger.h"
#include "monarch/rt/DynamicObject.h"

namespace monarch
{
namespace crypto
{

/**
 * A RoundingMode specifies a type of decimal rounding.  Rounding is symmetric
 * such that rounding is applied on the absolute value and the sign is
 * re-applied afterwards.
 *
 * Rounding action on final rounding digit:
 *
 * Up = add 1 if followed by non-zero
 * Down = do nothing
 * HalfUp = add 1 if next digit is 5-9, do nothing if 0-4
 * HalfEven = add 1 if next digit is 6 or more or 5 followed by non-zero,
 *    do nothing if next digit is 4 or less,
 *    add 1 if digit is odd and followed by 5 and possible zeros,
 *    do nothing if digit is even.
 *
 * (See http://en.wikipedia.org/wiki/Rounding)
 */
enum RoundingMode
{
   Up, HalfUp, HalfEven, Down
};

/**
 * A BigDecimal is a multi/arbitrary precision decimal number. If the number
 * is used to perform arithmetic that may require rounding via division, like
 * 1 / 3, then the number of digits of precision must be set via setPrecision().
 * The default precision is 10 with rounding using HalfUp.
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

public:
   /**
    * Creates a new BigDecimal with the specified value.
    *
    * @param value the value for this BigDecimal.
    */
   BigDecimal(double value = 0.0);

   /**
    * Creates a new BigDecimal with the specified value.
    *
    * @param value the value for this BigDecimal.
    */
   BigDecimal(int64_t value);

   /**
    * Creates a new BigDecimal with the specified value.
    *
    * @param value the value for this BigDecimal.
    */
   BigDecimal(uint64_t value);

   /**
    * Creates a new BigDecimal with the specified value.
    *
    * @param value the value for this BigDecimal.
    */
   BigDecimal(int32_t value);

   /**
    * Creates a new BigDecimal with the specified value.
    *
    * @param value the value for this BigDecimal.
    */
   BigDecimal(uint32_t value);

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
    * Creates a new BigDecimal with the specified value.
    *
    * @param value the value for this BigDecimal.
    */
   BigDecimal(monarch::rt::DynamicObject& value);

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
   virtual BigDecimal& operator=(const BigDecimal& rhs);

   /**
    * Sets this BigDecimal's value to the passed value.
    *
    * @param value the new value for this BigDecimal.
    *
    * @return this BigDecimal.
    */
   virtual BigDecimal& operator=(double rhs);

   /**
    * Sets this BigDecimal's value to the passed value.
    *
    * @param value the new value for this BigDecimal.
    *
    * @return this BigDecimal.
    */
   virtual BigDecimal& operator=(int64_t rhs);

   /**
    * Sets this BigDecimal's value to the passed value.
    *
    * @param value the new value for this BigDecimal.
    *
    * @return this BigDecimal.
    */
   virtual BigDecimal& operator=(uint64_t rhs);

   /**
    * Sets this BigDecimal's value to the passed value.
    *
    * @param value the new value for this BigDecimal.
    *
    * @return this BigDecimal.
    */
   virtual BigDecimal& operator=(int32_t rhs);

   /**
    * Sets this BigDecimal's value to the passed value.
    *
    * @param value the new value for this BigDecimal.
    *
    * @return this BigDecimal.
    */
   virtual BigDecimal& operator=(uint32_t rhs);

   /**
    * Sets this BigDecimal's value to the passed value.
    *
    * @param value the new value for this BigDecimal.
    *
    * @return this BigDecimal.
    */
   virtual BigDecimal& operator=(const char* rhs);

   /**
    * Sets this BigDecimal's value to the passed value.
    *
    * @param value the new value for this BigDecimal.
    *
    * @return this BigDecimal.
    */
   virtual BigDecimal& operator=(const std::string& rhs);

   /**
    * Sets this BigDecimal's value to the passed value.
    *
    * @param value the new value for this BigDecimal.
    *
    * @return this BigDecimal.
    */
   virtual BigDecimal& operator=(monarch::rt::DynamicObject& rhs);

   /**
    * Returns true if this BigDecimal is equal to the passed one.
    *
    * @param rhs the BigDecimal to compare to this one.
    *
    * @return true if this BigDecimal is equal to the passed one, false if not.
    */
   virtual bool operator==(const BigDecimal& rhs);

   /**
    * Returns true if this BigDecimal is equal to the passed double.
    *
    * @param rhs the double to compare to this BigDecimal.
    *
    * @return true if this BigDecimal is equal to the passed double,
    *         false if not.
    */
   virtual bool operator==(double rhs);

   /**
    * Returns true if this BigDecimal is equal to the passed DynamicObject.
    *
    * @param rhs the DynamicObject to compare to this BigDecimal.
    *
    * @return true if this BigDecimal is equal to the passed DynamicObject,
    *         false if not.
    */
   virtual bool operator==(monarch::rt::DynamicObject& rhs);

   /**
    * Returns true if this BigDecimal is not equal to the passed one.
    *
    * @param rhs the BigDecimal to compare to this one.
    *
    * @return true if this BigDecimal is not equal to the passed one, false if
    *         not.
    */
   virtual bool operator!=(const BigDecimal& rhs);

   /**
    * Returns true if this BigDecimal is not equal to the passed double.
    *
    * @param rhs the double to compare to this BigDecimal.
    *
    * @return true if this BigDecimal is not equal to the passed double,
    *         false if not.
    */
   virtual bool operator!=(double rhs);

   /**
    * Returns true if this BigDecimal is not equal to the passed DynamicObject.
    *
    * @param rhs the DynamicObject to compare to this BigDecimal.
    *
    * @return true if this BigDecimal is not equal to the passed DynamicObject,
    *         false if not.
    */
   virtual bool operator!=(monarch::rt::DynamicObject& rhs);

   /**
    * Returns true if this BigDecimal is less than the passed one.
    *
    * @param rhs the BigDecimal to compare to this one.
    *
    * @return true if this BigDecimal is less than the passed one, false if not.
    */
   virtual bool operator<(const BigDecimal& rhs);

   /**
    * Returns true if this BigDecimal is greater than the passed one.
    *
    * @param rhs the BigDecimal to compare to this one.
    *
    * @return true if this BigDecimal is greater than the passed one, false
    *         if not.
    */
   virtual bool operator>(const BigDecimal& rhs);

   /**
    * Returns true if this BigDecimal is less than or equal to the passed one.
    *
    * @param rhs the BigDecimal to compare to this one.
    *
    * @return true if this BigDecimal is less than or equal to the passed one,
    *         false if not.
    */
   virtual bool operator<=(const BigDecimal& rhs);

   /**
    * Returns true if this BigDecimal is greater than or equal to the passed one.
    *
    * @param rhs the BigDecimal to compare to this one.
    *
    * @return true if this BigDecimal is greater than or equal to the passed one,
    *         false if not.
    */
   virtual bool operator>=(const BigDecimal& rhs);

   /**
    * Returns the result of the passed BigDecimal added to this one.
    *
    * @param rhs the BigDecimal to add to this one.
    *
    * @return the sum of this BigDecimal and the passed one.
    */
   virtual BigDecimal operator+(const BigDecimal& rhs);

   /**
    * Returns the result of the passed BigDecimal subtracted from this one.
    *
    * @param rhs the BigDecimal to substract from this one.
    *
    * @return the difference between this BigDecimal and the passed one.
    */
   virtual BigDecimal operator-(const BigDecimal& rhs);

   /**
    * Returns the result of the passed BigDecimal multiplied by this one.
    *
    * @param rhs the BigDecimal to multiply by this one.
    *
    * @return the product of this BigDecimal and the passed one.
    */
   virtual BigDecimal operator*(const BigDecimal& rhs);

   /**
    * Returns the result of dividing this BigDecimal by the passed one.
    *
    * @param rhs the BigDecimal to divide into this one.
    *
    * @return the quotient of this BigDecimal and the passed one.
    */
   virtual BigDecimal operator/(const BigDecimal& rhs);

   /**
    * Returns this BigDecimal modulo the passed one.
    *
    * @param rhs the BigDecimal to mod this one by.
    *
    * @return the modulus of the passed BigDecimal respective to this one.
    */
   virtual BigDecimal operator%(const BigDecimal& rhs);

   /**
    * Adds the passed BigDecimal to this one and returns the result.
    *
    * @param rhs the BigDecimal to add to this one.
    *
    * @return the new value of this BigDecimal.
    */
   virtual BigDecimal& operator+=(const BigDecimal& rhs);

   /**
    * Subtracts the passed BigDecimal from this one and returns the result.
    *
    * @param rhs the BigDecimal to substract from this one.
    *
    * @return the new value of this BigDecimal.
    */
   virtual BigDecimal& operator-=(const BigDecimal& rhs);

   /**
    * Multiplies the passed BigDecimal by this one and returns the result.
    *
    * @param rhs the BigDecimal to multiply by this one.
    *
    * @return the new value of this BigDecimal.
    */
   virtual BigDecimal& operator*=(const BigDecimal& rhs);

   /**
    * Divides the passed BigDecimal into this one and returns the result.
    *
    * @param rhs the BigDecimal to divide into this one.
    *
    * @return the new value of this BigDecimal.
    */
   virtual BigDecimal& operator/=(const BigDecimal& rhs);

   /**
    * Sets this BigDecimal to the modulus of the passed one.
    *
    * @param rhs the BigDecimal to mod this one by.
    *
    * @return the new value of this BigDecimal.
    */
   virtual BigDecimal& operator%=(const BigDecimal& rhs);

   /**
    * Returns true if this BigDecimal is zero, false if not.
    *
    * @return true if this BigDecimal is zero, false if not.
    */
   virtual bool isZero() const;

   /**
    * Sets whether or not this BigDecimal is negative.
    *
    * @param negative true if this BigDecimal should be negative, false if not.
    */
   virtual void setNegative(bool negative);

   /**
    * Returns true if this BigDecimal is negative, false if not.
    *
    * @return true if this BigDecimal is negative, false if not.
    */
   virtual bool isNegative() const;

   /**
    * Gets the value of this BigDecimal as a double.
    *
    * @return the value of this BigDecimal as a double.
    */
   virtual double getDouble() const;

   /**
    * Sets the number of digits of precision for this BigDecimal for
    * arithmetic operations that require rounding.
    *
    * @param precision the number of digits of precision to use.
    * @param roundingMode the RoundingMode to use.
    */
   virtual void setPrecision(unsigned int precision, RoundingMode roundingMode);

   /**
    * Gets the number of digits of precision for this BigDecimal for
    * arithmetic operations that require rounding.
    *
    * @return the number of digits of precision to use.
    */
   virtual unsigned int getPrecision();

   /**
    * Rounds this BigDecimal according to its set precision.
    */
   virtual void round();

   /**
    * Gets the value of this BigDecimal as a string. By default the string will
    * include the significant digits up to a maximum precision as set by
    * setPrecision().  If zeroFill is true, zeros will be filled in up to the
    * set precision.  If truncate is "false" and the internal representation
    * has more significant digits that the current precision, they will all be
    * returned.
    *
    * @param zeroFill true to zero-fill after the decimal point to the set
    *           precision, false to only use significant digits.
    * @param truncate true to truncate result at current precision, false to
    *           return all current significant digits.
    *
    * @return the string.
    */
   virtual std::string toString(
      bool zeroFill = false, bool truncate = true) const;

   /**
    * Gets the value of this BigDecimal as a DynamicObject.
    *
    * @param zeroFill true to zero-fill after the decimal point to the set
    *           precision, false to only use significant digits.
    * @param truncate true to truncate result at current precision, false to
    *           return all current significant digits.
    *
    * @return the DynamicObject.
    */
   virtual monarch::rt::DynamicObject toDynamicObject(
      bool zeroFill = true, bool truncate = true) const;

   /**
    * Sets the components of the internal representation for this BigDecimal.
    * This method is for private use by unit tests and should not be needed
    * for general use.  This method may be removed in the future.
    *
    * @param significand the significand.
    * @param exponent the exponent.
    */
   virtual void _setValue(BigInteger& significand, int exponent);

protected:
   /**
    * Initializes this BigDecimal.
    */
   virtual void initialize();

   /**
    * Sets the exponent for this BigDecimal.  If the new exponent is larger
    * than the current exponent, the significand will be increased and the
    * final value will remain the same.  If the new exponent is smaller than
    * the current exponent, the significand will be decreased and the value
    * may become less accurate.
    *
    * @param exponent the new exponent to use.
    */
   virtual void setExponent(int exponent);

   /**
    * Makes the exponents equal for the passed two BigDecimals. The larger
    * exponent will be chosen so as to retain precision.
    *
    * @param bd1 the first BigDecimal.
    * @param bd2 the second BigDecimal.
    */
   static void synchronizeExponents(BigDecimal& bd1, BigDecimal& bd2);
};

} // end namespace crypto
} // end namespace monarch

#endif
