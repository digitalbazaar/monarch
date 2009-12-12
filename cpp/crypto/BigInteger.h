/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_crypto_BigInteger_H
#define monarch_crypto_BigInteger_H

#include "monarch/io/ByteBuffer.h"

#include <openssl/bn.h>
#include <string>
#include <inttypes.h>

namespace monarch
{
namespace crypto
{

/**
 * A BigInteger is an arbitrary precision whole number.
 *
 * This class relies on OpenSSL's crypto BN implementation.
 *
 * Note: Might need to change operators to friends for STL.
 *
 * @author Dave Longley
 */
class BigInteger
{
protected:
   /**
    * The underlying OpenSSL BIGNUM structure.
    */
   BIGNUM* mBigNum;

   /**
    * A BN_CTX structure for storing temporary variables while performing
    * arithmetic.
    */
   BN_CTX* mBigNumContext;

   /**
    * Initializes this BigInteger.
    */
   void initialize();

   /**
    * Gets an initialized BN_CTX structure.
    *
    * @return an initialized BN_CTX structure.
    */
   BN_CTX* getContext();

public:
   /**
    * Creates a new BigInteger with the specified value.
    *
    * @param value the value for this BigInteger.
    */
   BigInteger(unsigned long long value);

   /**
    * Creates a new BigInteger with the specified value.
    *
    * @param value the value for this BigInteger.
    */
   BigInteger(long long value);

   /**
    * Creates a new BigInteger with the specified value.
    *
    * @param value the value for this BigInteger.
    */
   BigInteger(int value);

   /**
    * Creates a new BigInteger with the specified value.
    *
    * @param value the value for this BigInteger.
    */
   BigInteger(unsigned int value = 0);

   /**
    * Creates a new BigInteger with the specified value.
    *
    * @param value the value for this BigInteger.
    */
   BigInteger(const char* value);

   /**
    * Creates a new BigInteger with the specified value.
    *
    * @param value the value for this BigInteger.
    */
   BigInteger(const std::string& value);

   /**
    * Creates a new BigInteger by copying another one.
    *
    * @param copy the BigInteger to copy.
    */
   BigInteger(const BigInteger& copy);

   /**
    * Destructs this BigInteger.
    */
   virtual ~BigInteger();

   /**
    * Sets this BigInteger equal to the passed one.
    *
    * @param rhs the BigInteger to set this one equal to.
    *
    * @return this BigInteger.
    */
   BigInteger& operator=(const BigInteger& rhs);

   /**
    * Sets this BigInteger's value to the passed value.
    *
    * @param value the new value for this BigInteger.
    *
    * @return this BigInteger.
    */
   BigInteger& operator=(unsigned long long rhs);

   /**
    * Sets this BigInteger's value to the passed value.
    *
    * @param value the new value for this BigInteger.
    *
    * @return this BigInteger.
    */
   BigInteger& operator=(long long rhs);

   /**
    * Sets this BigInteger's value to the passed value.
    *
    * @param value the new value for this BigInteger.
    *
    * @return this BigInteger.
    */
   BigInteger& operator=(unsigned int rhs);

   /**
    * Sets this BigInteger's value to the passed value.
    *
    * @param value the new value for this BigInteger.
    *
    * @return this BigInteger.
    */
   BigInteger& operator=(int rhs);

   /**
    * Sets this BigInteger's value to the passed value.
    *
    * @param value the new value for this BigInteger.
    *
    * @return this BigInteger.
    */
   BigInteger& operator=(const char* rhs);

   /**
    * Sets this BigInteger's value to the passed value.
    *
    * @param value the new value for this BigInteger.
    *
    * @return this BigInteger.
    */
   BigInteger& operator=(const std::string& rhs);

   /**
    * Returns true if this BigInteger is equal to the passed one.
    *
    * @param rhs the BigInteger to compare to this one.
    *
    * @return true if this BigInteger is equal to the passed one, false if not.
    */
   bool operator==(const BigInteger& rhs);

   /**
    * Returns true if this BigInteger is equal to the passed 64-bit integer.
    *
    * @param rhs the 64-bit integer to compare to this BigInteger.
    *
    * @return true if this BigInteger is equal to the passed 64-bit integer,
    *         false if not.
    */
   bool operator==(long long rhs);

   /**
    * Returns true if this BigInteger is not equal to the passed one.
    *
    * @param rhs the BigInteger to compare to this one.
    *
    * @return true if this BigInteger is not equal to the passed one, false if
    *         not.
    */
   bool operator!=(const BigInteger& rhs);

   /**
    * Returns true if this BigInteger is not equal to the passed 64-bit integer.
    *
    * @param rhs the 64-bit integer to compare to this BigInteger.
    *
    * @return true if this BigInteger is not equal to the passed 64-bit integer,
    *         false if not.
    */
   bool operator!=(long long rhs);

   /**
    * Returns true if this BigInteger is less than the passed one.
    *
    * @param rhs the BigInteger to compare to this one.
    *
    * @return true if this BigInteger is less than the passed one, false if not.
    */
   bool operator<(const BigInteger& rhs);

   /**
    * Returns true if this BigInteger is greater than the passed one.
    *
    * @param rhs the BigInteger to compare to this one.
    *
    * @return true if this BigInteger is greater than the passed one, false
    *         if not.
    */
   bool operator>(const BigInteger& rhs);

   /**
    * Returns true if this BigInteger is less than or equal to the passed one.
    *
    * @param rhs the BigInteger to compare to this one.
    *
    * @return true if this BigInteger is less than or equal to the passed one,
    *         false if not.
    */
   bool operator<=(const BigInteger& rhs);

   /**
    * Returns true if this BigInteger is greater than or equal to the passed one.
    *
    * @param rhs the BigInteger to compare to this one.
    *
    * @return true if this BigInteger is greater than or equal to the passed one,
    *         false if not.
    */
   bool operator>=(const BigInteger& rhs);

   /**
    * Returns the result of the passed BigInteger shifted to the left n bits.
    *
    * @param n the number of bits to shift to the left.
    *
    * @return the result of shifting this BigInteger to the left n bits.
    */
   BigInteger operator<<(int n);

   /**
    * Returns the result of the passed BigInteger shifted to the right n bits.
    *
    * @param n the number of bits to shift to the right.
    *
    * @return the result of shifting this BigInteger to the right n bits.
    */
   BigInteger operator>>(int n);

   /**
    * Returns the result of the passed BigInteger added to this one.
    *
    * @param rhs the BigInteger to add to this one.
    *
    * @return the sum of this BigInteger and the passed one.
    */
   BigInteger operator+(const BigInteger& rhs);

   /**
    * Returns the result of the passed BigInteger subtracted from this one.
    *
    * @param rhs the BigInteger to substract from this one.
    *
    * @return the difference between this BigInteger and the passed one.
    */
   BigInteger operator-(const BigInteger& rhs);

   /**
    * Returns the result of the passed BigInteger multiplied by this one.
    *
    * @param rhs the BigInteger to multiply by this one.
    *
    * @return the product of this BigInteger and the passed one.
    */
   BigInteger operator*(const BigInteger& rhs);

   /**
    * Returns the result of dividing this BigInteger by the passed one.
    *
    * @param rhs the BigInteger to divide into this one.
    *
    * @return the quotient of this BigInteger and the passed one.
    */
   BigInteger operator/(const BigInteger& rhs);

   /**
    * Returns the result of this BigInteger raised to the power of the passed
    * one.
    *
    * @param rhs the BigInteger to raise this one by.
    *
    * @return the result of this BigInteger raised by the passed one.
    */
   BigInteger pow(const BigInteger& rhs);

   /**
    * Returns this BigInteger modulo the passed one.
    *
    * @param rhs the BigInteger to mod this one by.
    *
    * @return the modulus of the passed BigInteger respective to this one.
    */
   BigInteger operator%(const BigInteger& rhs);

   /**
    * Adds the passed BigInteger to this one and returns the result.
    *
    * @param rhs the BigInteger to add to this one.
    *
    * @return the new value of this BigInteger.
    */
   BigInteger& operator+=(const BigInteger& rhs);

   /**
    * Subtracts the passed BigInteger from this one and returns the result.
    *
    * @param rhs the BigInteger to substract from this one.
    *
    * @return the new value of this BigInteger.
    */
   BigInteger& operator-=(const BigInteger& rhs);

   /**
    * Multiplies the passed BigInteger by this one and returns the result.
    *
    * @param rhs the BigInteger to multiply by this one.
    *
    * @return the new value of this BigInteger.
    */
   BigInteger& operator*=(const BigInteger& rhs);

   /**
    * Divides the passed BigInteger into this one and returns the result.
    *
    * @param rhs the BigInteger to divide into this one.
    *
    * @return the new value of this BigInteger.
    */
   BigInteger& operator/=(const BigInteger& rhs);

   /**
    * Raises this BigInteger to the power of the passed one and returns the
    * result.
    *
    * @param rhs the BigInteger to raise this one by.
    *
    * @return the new value of this BigInteger.
    */
   BigInteger& powEquals(const BigInteger& rhs);

   /**
    * Sets this BigInteger to the modulus of the passed one.
    *
    * @param rhs the BigInteger to mod this one by.
    *
    * @return the new value of this BigInteger.
    */
   BigInteger& operator%=(const BigInteger& rhs);

   /**
    * Performs a modular exponentiation and returns the remainder of the
    * division of this BigIntger raised to the power of "e" and divided by "m".
    *
    * @param e the exponent to raise this BigInteger to.
    * @param m the modulus to divide by.
    *
    * @return the result of the modular exponentiation.
    */
   BigInteger modexp(const BigInteger& e, const BigInteger& m);

   /**
    * Performs a modular exponentiation and sets this BigInteger to the
    * remainder of the division of this BigIntger raised to the power of
    * "e" and divided by "m".
    *
    * @param e the exponent to raise this BigInteger to.
    * @param m the modulus to divide by.
    *
    * @return the new value of this BigInteger.
    */
   BigInteger& modexpEquals(const BigInteger& e, const BigInteger& m);

   /**
    * Compares the absolute value of this BigInteger to the absolute value
    * of another one.
    *
    * @param rhs the BigInteger to compare this one to.
    *
    * @return -1 if this BigInteger's absolute value is less than the passed
    *         BigInteger's absolute value, 0 if they are the same, and 1 if
    *         this BigInteger's absolute value is greater.
    */
   int absCompare(const BigInteger& rhs);

   /**
    * Divides this BigInteger by another and returns the whole number
    * quotient and the remainder.
    *
    * @param divisor the BigInteger to divide by.
    * @param quotient the BigInteger to store the result of the division in.
    * @param remainder the BigInteger to store the remainder in.
    */
   void divide(
      const BigInteger& divisor, BigInteger& quotient, BigInteger& remainder);

   /**
    * Returns true if this BigInteger is zero, false if not.
    *
    * @return true if this BigInteger is zero, false if not.
    */
   bool isZero() const;

   /**
    * Sets whether or not this BigInteger is negative.
    *
    * @param negative true if this BigInteger should be negative, false if not.
    */
   void setNegative(bool negative);

   /**
    * Returns true if this BigInteger is negative, false if not.
    *
    * @return true if this BigInteger is negative, false if not.
    */
   bool isNegative() const;

   /**
    * Returns true if this BigInteger can be stored in a 64-bit integer, false
    * if not.
    *
    * @return true if this BigInteger can be stored in a 64-bit integer, false
    *         if not.
    */
   bool isCompact() const;

   /**
    * Gets the value of this BigInteger as a 32-bit unsigned integer.
    *
    * @return the value of this BigInteger as a 32-bit unsigned integer.
    */
   uint32_t getUInt32() const;

   /**
    * Gets the value of this BigInteger as a 64-bit integer.
    *
    * @return the value of this BigInteger as a 64-bit integer.
    */
   int64_t getInt64() const;

   /**
    * Gets the number of bytes required to store this BigInteger.
    *
    * @return the number of bytes required to store this BigInteger.
    */
   int getNumBytes() const;

   /**
    * Converts an array of bytes into a BigInteger.
    *
    * @param data the binary data.
    * @param length the number of bytes.
    */
   void fromBytes(const char* data, int length);

   /**
    * Writes this BigInteger to a ByteBuffer, resizing it if necessary.
    *
    * @param b the ByteBuffer to write to.
    */
   void toBytes(monarch::io::ByteBuffer* b);

   /**
    * Gets the value of this BigInteger as a decimal string.
    *
    * @return the string.
    */
   std::string toString() const;

   /**
    * Converts a hexadecimal string into a BigInteger.
    *
    * @param hex the hexadecimal string.
    *
    * @return the BigInteger.
    */
   void fromHex(const char* hex);

   /**
    * Gets the value of this BigInteger as a hexadecimal string.
    *
    * @return the hexadecimal string.
    */
   std::string toHex();

   /**
    * Generates a cryptographically strong random number with the given
    * number of bits.
    *
    * @param bits the number of bits.
    * @param top if -1 then the top bit may be 0, if 0 then the top bit will
    *            be set to 1, if 1 then the two most significant bits will
    *            be set to 1 so that the product of two similarly generated
    *            random numbers will be 2*bits in length.
    * @param bottom true to make the number odd, false to make it even.
    *
    * @return the generated number.
    */
   static BigInteger random(int bits, int top, bool bottom);

   /**
    * Generates a cryptographically strong random number between 0 and the
    * given BigInteger.
    *
    * @param max the maximum value for the random BigInteger.
    *
    * @return the generated number.
    */
   static BigInteger random(BigInteger& max);

   /**
    * Generates a pseudo-random number that should not be considered
    * cryptographically strong.
    *
    * @param bits the number of bits.
    * @param top if -1 then the top bit may be 0, if 0 then the top bit will
    *            be set to 1, if 1 then the two most significant bits will
    *            be set to 1 so that the product of two similarly generated
    *            random numbers will be 2*bits in length.
    * @param bottom true to make the number odd, false to make it even.
    *
    * @return the generated number.
    */
   static BigInteger pseudoRandom(int bits, int top, bool bottom);

   /**
    * Generates a pseudo-random number number between 0 and the
    * given BigInteger that should not be considered cryptographically strong.
    *
    * @param max the maximum value for the random BigInteger.
    *
    * @return the generated number.
    */
   static BigInteger pseudoRandom(BigInteger& max);
};

} // end namespace crypto
} // end namespace monarch

#endif
