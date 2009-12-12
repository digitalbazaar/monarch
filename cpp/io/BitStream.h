/*
 * Copyright (c) 2008-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_BitStream_H
#define monarch_util_BitStream_H

#include <string>

namespace monarch
{
namespace io
{

/**
 * A BitStream is a bit set that is optimized for use with entire bytes or
 * byte arrays.
 *
 * @author Dave Longley
 * @author Manu Sporny
 */
class BitStream
{
protected:
   /**
    * A pointer to the underlying bit set.
    */
   unsigned char* mBitSet;

   /**
    * The size of the bit set in bytes.
    */
   int mSize;

   /**
    * The length of this BitStream (the number of bits).
    */
   int mLength;

   /**
    * Resizes the bit set by the given number of bytes.
    *
    * @param bytes the number of bytes to increase by.
    */
   virtual void resize(int bytes);

public:
   /**
    * Creates a new BitStream.
    */
   BitStream();

   /**
    * Destructs this BitStream.
    */
   virtual ~BitStream();

   /**
    * Appends the passed bit to this stream.
    *
    * @param bit the bit.
    */
   virtual void append(bool bit);

   /**
    * Appends all of the bits in the passed byte array.
    *
    * @param b the array of bytes with bits to append.
    * @param length the number of bytes to append.
    */
   virtual void append(const unsigned char* b, int length);

   /**
    * Appends the passed string of 1's and 0's as bits to this stream.
    *
    * @param str a string of 1's and 0's, i.e. "100101101"
    * @param length the length of the string (excludes any null-terminator).
    *
    * @return true if successful, false if an exception occurred.
    */
   virtual bool appendFromString(const char* str, int length);

   /**
    * Writes this bit set to the passed string, i.e. appends "1110101101".
    *
    * @param str the string to write to.
    *
    * @return a reference to the string.
    */
   virtual std::string& appendToString(std::string& str);

   /**
    * Gets the value of a bit at a particular bit offset.
    *
    * @param offset the offset of the bit to use.
    *
    * @return true if the bit is set, false if it is cleared.
    */
   virtual bool operator[](int offset);

   /**
    * Shifts this entire stream to the left by n bits.
    *
    * @param n the number of bits to shift to the left.
    */
   virtual void operator<<(int n);

   /**
    * Extracts a byte from the bit stream at the given offset.
    *
    * @param bitOffset the bit offset at which to begin in the bit stream.

    * @return a byte containing the byte asked for from the given offset.
    */
   virtual unsigned char get(int bitOffset);

   /**
    * Extracts an array of bytes from the bit stream.
    *
    * @param bitOffset the bit offset at which to begin in the bit stream.
    * @param b the byte array to populate.
    * @param count the number of bytes to read from the bitstream.
    */
   virtual void get(int bitOffset, unsigned char* b, int count);

   /**
    * Sets the length of this bit stream.
    *
    * @param length the new length for this bit stream.
    */
   virtual void setLength(int length);

   /**
    * Gets the length of this bit stream.
    *
    * @return the number of bits in this bit stream.
    */
   virtual int length();

   /**
    * Sets all bits in this bit stream to zero.
    */
   virtual void clear();

   /**
    * Sets all bits in this bit stream to 1.
    */
   virtual void set();

   /**
    * Gets direct access to the underlying bit set's array of bytes. The array
    * has a length of "length() * 8 + (length() % 8) ? 1 : 0".
    *
    * @return this bit set as an array of bytes.
    */
   virtual unsigned char* bytes();

   /**
    * Gets the minimum number of bytes required to store all the bits
    * in this stream.
    *
    * @return the minimum number of bytes required to store all the bits
    *         in this stream.
    */
   virtual int bytesLength();

   /**
    * Writes this bit set to a string, i.e. "1110101101".
    *
    * @return the string.
    */
   virtual std::string toString();

   /**
    * Writes part this bit set to a string, i.e. "1110101101".
    *
    * @param offset the starting offset.
    * @param length the ending offset.
    *
    * @return the string.
    */
   virtual std::string toString(int offset, int length);
};

} // end namespace io
} // end namespace monarch
#endif
