/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#ifndef monarch_util_Crc16_H
#define monarch_util_Crc16_H

#include "monarch/util/HashAlgorithm.h"

namespace monarch
{
namespace util
{

/**
 * This class is used to calculate a 16-bit CRC (Cyclic Redundancy Check).
 *
 * A CRC n-bit value is the remainder of the modulo 2 division of a bit stream
 * (plus n zeros) by a polynomial "key" (represented as a bit stream). This
 * is explained in depth below:
 *
 * A CRC value is calculated, in part, by dividing an input bit stream by a
 * predefined bit stream of length n which represents the coefficients of a
 * polynomial (referred to as a "key") of degree n.
 *
 * The polynomial key uses coefficients that are either 0 or 1 and any
 * arithmetic performed using the polynomial is done using modulo 2. What this
 * means is that whenever any arithmetic is performed on the coefficients the
 * result is moded (% operator in java) with 2. Therefore, the remainder of a
 * division, for instance, of any two of the polynomial's coefficients, will
 * always be a 0 or a 1.
 *
 * The other part of calculating a CRC value is adding n zeros to the end of
 * the input bit stream before the division by the polynomial key. This
 * is done because it allows the recipient of the bit stream and its CRC
 * to run the bit stream and the CRC through the same CRC algorithm and get a
 * remainder of 0.
 *
 * An explanation for how this works follows:
 *
 * If the input bit stream for the CRC is defined as M(x) and the polynomial
 * key is defined as K(x), and the quotient of M(x) divided by K(x) is Q(x)
 * with a remainder of R(x) then their algebraic relationship is:
 *
 * M(x) = K(x)Q(x) + R(x)
 *
 * If we add n zeros to the message before the division then the formula is:
 *
 * M(x) * x^n = K(x)Q(x) + R(x)
 *
 * Which can be rewritten as:
 *
 * M(x) * x^n + R(x) = K(x)Q(x)[no remainder]
 *
 * Now if the remainder R(x) (now also known as the CRC value) is added to the
 * bit stream and sent along with it to its destination, the receiver can run
 * the entire message (bit stream plus the previously calculated CRC) through
 * the CRC algorithm and get a remainder (or a new CRC) of 0.
 *
 * Therefore, the remainder of the modulo 2 division of the bit stream (plus
 * n zeros) by the polynomial key is the CRC value.
 *
 * An example of a polynomial that can be used by this class is
 *
 * x^16 + x^15 + x^2 + 1.
 *
 * This can be written as a bit stream like so:
 *
 * 11000000000000101
 *
 * With each bit representing a coefficient in the polynomial. As you can see,
 * the bit stream is 17 bits long. CRC algorithms always use a coefficient of 1
 * for the degree of the polynomial, meaning a CRC-16 has a coefficient of 1
 * for x^16. Therefore, the polynomial can be stored as a bit stream using one
 * less bit -- the most significant bit can be dropped since it is always 1.
 *
 * Therefore the polynomial x^16 + x^15 + x^2 + 1 can be represented by 2
 * bytes:
 *
 * 10000000 00000101
 *
 * Hence, it is a 16-bit polynomial key -- for a CRC-16 algorithm. A CRC-32
 * algorithm would use a polynomial of the 32nd degree and the key would
 * take up 4 bytes.
 *
 * This example key can be written in hexidecimal as:
 *
 * 0x8005
 *
 * This is just 10000000 00000101 converted to hex. This value or another
 * 16-bit value like it is used as the divisor in the CRC algorithm implemented
 * by this class. If no key is specified when this class is instantiated, this
 * class will use the above key by default.
 *
 * The algorithm for calculating a CRC is relatively simple. The polynomial
 * key is divided into the bit stream the CRC is being produced for, with
 * the quotient being discarded and the remainder being the CRC.
 *
 * It is important to note that when using arithmetic modulo 2 on a bit stream,
 * addition and subtraction are the equivalent of performing a XOR. For
 * instance:
 *
 * 0 + 0 = 0 % 2 = 0
 * 0 + 1 = 1 % 2 = 1
 * 1 + 1 = 2 % 2 = 0
 *
 * 0 - 0 = 0 % 2 = 0
 * 1 - 0 = 1 % 2 = 1
 * 1 - 1 = 0 % 2 = 0
 *
 * A long division algorithm can therefore be performed on a bit by bit basis
 * like so:
 *
 * Note: This example is not specifically for a CRC-16 but the general idea
 * for works for any CRC ... it is just that the key length chosen is only
 * 3 bits long for simplicity.
 *
 * Suppose the input bit stream for the CRC is: 10010110
 * Suppose the the key is: 101 (i.e. 1x^2 + 0x^1 + 1x^0)
 *
 * Therefore, we must add 2 zeros (key polynomial has a degree of 2) to
 * the input bit stream which is now:
 *
 * 1001011000
 *
 * To divide the key into the bit stream, we use the standard long division
 * method but we don't care what the quotient is -- we are only interested
 * in the remainder.
 *     ____________
 * 101 | 1001011100
 *      -101
 *       ___
 *         110
 *        -101
 *         ___
 *          111
 *         -101
 *          ___
 *           101
 *          -101
 *           ___
 *             0100
 *             -101
 *              ___
 *                1
 *
 * Our remainder is 1, which is our CRC. One way to implement the above
 * division is by using a shift register (R) and XOR for subtraction. As we
 * go through each bit in the bit stream, we shift the shift register once to
 * the left and OR the bit onto its right hand side.
 *
 * If we re-write the above example like so, we can highlight the shift
 * register's values. (R) starts with a value of 000:
 *     ____________
 * 101 | 1001011100
 *  R:000.  .  .  .
 *  R: 001  .  .  .
 *  R:  010 .  .  .
 *  R:   100.  .  .
 *       XOR.  .  .
 *       101.  .  .
 *  R:   001.  .  .
 *  R:    011  .  .
 *  R:     110 .  .
 *         XOR .  .
 *         101 .  .
 *  R:     011 .  .
 *  R:      111.  .
 *          XOR.  .
 *          101.  .
 *  R:      010.  .
 *  R:       101  .
 *           XOR  .
 *           101  .
 *  R:       000  .
 *  R:        001 .
 *  R:         010.
 *  R:          100
 *              XOR
 *              101
 *  R:          001
 *
 * As you can see, as we move through the bit stream, the shift register shifts
 * one bit out to the left and shifts one bit in to the right from the bit
 * stream. The '.'s are there to help visualize which bit is being shifted
 * in from the right (OR'd with the register from the bit stream). You can also
 * see that whenever the register's left most bit is a 1, the register get's
 * XOR'd with the polynomial key. This mirrors the way polynomial division
 * works -- division can only be performed when the coefficient of the remaining
 * dividend portion is non-zero.
 *
 * Now, we also know that we store polynomial keys without using the first
 * coefficient of the polynomial -- as it is assumed to be 1. So for our
 * 3-bit key, we would actually be given a key that is 2-bits long that would
 * have the first bit dropped. The key would be 01.
 *
 * If we keep this in mind, then we can realize that we only need to store
 * 2-bits for our register as well, as the left most bit will always be set
 * to 0 after we XOR with the key since we only XOR when the register's
 * left most bit is a 1 (1 (for the register) XOR 1 (for the key) = 0).
 *     ____________
 * 101 | 1001011100
 *  R: 00.  .  .  .
 *  R:  01  .  .  .
 *  R:   10 .  .  .
 *  R:    00.  .  .
 *       XOR.  .  .
 *        01.  .  .
 *  R:    01.  .  .
 *  R:     11  .  .
 *  R:      10 .  .
 *         XOR .  .
 *          01 .  .
 *  R:      11 .  .
 *  R:       11.  .
 *          XOR.  .
 *           01.  .
 *  R:       10.  .
 *  R:        01  .
 *           XOR  .
 *            01  .
 *  R:        00  .
 *  R:         01 .
 *  R:          10.
 *  R:           00
 *              XOR
 *               01
 *  R:           01
 *
 * Therefore, as we add bits from the bit stream, we need to get the top
 * bit from the shift register and check to see if it is 1 and store that
 * value. Then we need to shift the next bit from the bit stream into the
 * register. Then, if the top bit was a 1, we need to XOR the register with
 * the key (where the key is only 2-bits [in this example]). If we perform
 * this action for every bit in the bit stream, then when we run out of
 * bits our shift register will be storing our CRC.
 *
 * To clarify: In the case of a CRC-16 our key will be 16-bits long (with the
 * coefficient for the 17th degree assumed to be 1 instead of 2-bits long
 * (with the coefficient for the 3rd degree assumed to be 1). We will need
 * a shift register that is, likewise, 16 bits in length.
 *
 * A CRC-32 would need a key of 32-bits and a register of 32-bits.
 *
 * The above implementation could be accomplished for a CRC-16 using this code:
 *
 * unsigned int mCrcValue;
 * unsigned int mPolynomialKey;
 *
 * void update(bool bit)
 * {
 *    // shift the crc value to the left
 *    mCrcValue <<= 1;
 *
 *    // put the bit on the right of the crc value
 *    if(bit)
 *    {
 *       mCrcValue |= 0x01;
 *    }
 *
 *    // if the left most bit was set, XOR the crc value with the key
 *    if((mCrcValue & 0x10000) != 0)
 *    {
 *       mCrcValue &= 0xffff;
 *       mCrcValue ^= mPolynomialKey;
 *    }
 * }
 *
 * void update(char* b, unsigned int offset, unsigned int length)
 * {
 *    // go through every byte
 *    unsigned int end = offset + length;
 *    for(; offset < end; offset++)
 *    {
 *       // get the next byte as an unsigned int
 *       unsigned int value = b[offset] & 0xff;
 *
 *       // go through each bit
 *       for(int i = 0; i < 8; i++)
 *       {
 *          bool bit = ((value >> (7 - i)) & 0x01) != 0;
 *          update(bit);
 *       }
 *    }
 * }
 *
 * With the above code, you have to make sure to run 2 null bytes through
 * the update method when finished to ensure that the n zeros are added.
 *
 * Now, there are also more optimized ways to calculate a CRC. These
 * involve operating on bytes instead of just bits, and, furthermore, such
 * methods automatically account for the n necessary null bytes.
 *
 * To speed up processing the input bit stream we can break our bit stream
 * up into chunks and XOR them onto our register (add them to the register
 * all at once). Once we have a chunk on our register, we can go through the
 * register bit by bit (of the chunk) shifting to the left once for each bit.
 * We then check to see if the bit we shifted out was a 1 or not, just like
 * before. If it was, then we divide the whole register by the polynomial key,
 * again, just like before.
 *
 * This is simply a faster way to get the bit stream into the register and
 * divide it by the key. It also has a positive side affect -- as we process
 * each chunk all the way through, if we place the chunks at the top of the
 * register, then the XOR with the key will automatically happen against the
 * n zeros we require at the end of the input bit stream.
 *
 * For instance, if we have a 4-bit key and we break our input up into
 * 2 bit chunks and place each chunk at the top of the register, then when
 * we have shifted both bits out, we will be XOR'ing the polynomial key against
 * a register that has the 4 extra zeros we need (since, as you shift left,
 * 0's are added to the right hand side of the register).
 *
 * The same applies for any size register -- for our CRC-16 we have a 16-bit
 * register, so if we want to process 1 byte at a time, we can place the
 * byte at the top of the register (left most 8 bits) and shift it out one bit
 * at a time (XOR'ing with the key as we shift out 1's) -- until the last bit
 * is processed with a register that is full of the 16 0's we need.
 *
 * Furthermore, if we add another byte to the process, our register get's
 * XOR'd with the new byte, affectively wiping out the zero's that were in
 * place until that next byte is processed and the zero's are divided against
 * again. This allows us to update the CRC easily with new bytes.
 *
 * This code can be used to accomplish the above:
 *
 * void update(unsigned char b)
 * {
 *    // get the byte as an unsigned int
 *    unsigned int value = b & 0xff;
 *
 *    // shift the byte to the left 8 and add it to the top of the register
 *    // so that it can be XOR'd against one bit at a time and so that when
 *    // the bit is shifted out, the register will have the n zeros necessary
 *    // before being XOR'd against the polynomial key
 *    mCrcValue ^= value << 8;
 *
 *    // go through each bit of the input byte
 *    for(int i = 0; i < 8; i++)
 *    {
 *       // shift out the first bit
 *       mCrcValue <<= 1;
 *
 *       // see if a 1 was shifted out
 *       if((mCrcValue & 0x10000) != 0)
 *       {
 *          // XOR the CRC value with the CRC polynomial
 *          mCrcValue ^= mPolynomialKey;
 *       }
 *    }
 *
 *    // cut crc to 16-bits (2 bytes)
 *    mCrcValue &= 0xffff;
 * }
 *
 * A further optimization can be made by storing all of the XOR'd bytes in
 * a table. There are only 256 different byte values -- and if we store
 * the register that results from processing any one of these bytes, we
 * can just look it up later and apply the value from the table. This is
 * true because XOR'ing is equivalent to addition, as explained earlier, which
 * is associative -- meaning it can happen in any order.
 *
 * If, for every byte we process, we get the top of our register and XOR
 * it with the incoming byte, we can then check the table for that value
 * and get the pre-XOR'd register and XOR it against our register. This is
 * the register that would result from running our byte through one bit at
 * time -- except that we have already performed all of the calculations that
 * were necessary.
 *
 * In other words, we start out with an empty register. We add our byte to
 * the top of the register via XOR. Our calculations would go through each bit
 * of the top of the register, shifting left, and XOR'ing with the polynomial
 * key when we shift out a 1. For any given byte at the top of the register,
 * this calculation will always be the same -- so we can store this value in
 * a table.
 *
 * So, when we want to add a new byte to the register, we store the current top
 * of the register, XOR our new byte with it, look up the value for that byte
 * in the table, shift out the old top of the register and XOR that result with
 * our new table value and store it as the new register.
 *
 * Every new byte is just an "addition" (modulo 2) to our current register.
 *
 * @author Dave Longley
 */
class Crc16 : public HashAlgorithm
{
protected:
   /**
    * The current CRC value.
    */
   unsigned int mCrcValue;

   /**
    * Set to true once the register table has been initialized.
    */
   static bool sRegisterTableInitialized;

   /**
    * A table of pre-XOR'd registers. One value for each possible byte value.
    */
   static short sRegisterTable[256];

   /**
    * Initializes the table of registers for a given polynomial, one register
    * for each byte value.
    *
    * @param key the polynomial key to use.
    */
   static bool initializeTable(const unsigned int& key);

public:
   /**
    * The length of a checksum for the CRC-16 algorithm in bytes.
    */
   static const unsigned int CRC16_CHECKSUM_LENGTH = 2;

   /**
    * Creates a new Crc16.
    */
   Crc16();

   /**
    * Destructs this Crc16.
    */
   virtual ~Crc16();

   /**
    * Resets this HashAlgorithm so it can be used again with new input.
    */
   virtual void reset();

   /**
    * Updates the current CRC value with the given byte.
    *
    * @param b the byte to update the CRC value with.
    */
   void update(const unsigned char& b);

   /**
    * Updates the data to hash. This method can be called repeatedly with
    * smaller chunks of the data that is to be hashed.
    *
    * @param b a buffer with data to hash.
    * @param length the length of the data.
    */
   virtual void update(const char* b, unsigned int length);

   /**
    * Puts the hash value into an array of bytes. The length of the hash value
    * is CRC16_CHECKSUM_LENGTH (2 bytes).
    *
    * @param b a buffer to fill with the hash data.
    * @param length the length of the value.
    */
   virtual void getValue(char* b, unsigned int& length);

   /**
    * Gets the maximum length of the hash value in bytes.
    *
    * @return the maximum length of the hash value in bytes.
    */
   virtual unsigned int getValueLength();

   /**
    * Gets the hash value (the checksum) as an integer.
    *
    * @return the checksum for this CRC.
    */
   unsigned int getChecksum();
};

} // end namespace util
} // end namespace monarch
#endif
