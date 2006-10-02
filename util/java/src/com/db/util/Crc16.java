/*
 * Copyright (c) 2006 Digital Bazaar, Inc.  All rights reserved.
 */
package com.db.util;

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
 * protected int mCrcValue;
 * protected int mPolynomialKey;
 * 
 * public void update(boolean bit)
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
 * public void update(byte[] bytes, int offset, int length)
 * {
 *    // go through every byte
 *    int end = offset + length;
 *    for(; offset < end; offset++)
 *    {
 *       // get the next byte as an unsigned int
 *       int value = bytes[offset] & 0xff;
 *       
 *       // go through each bit
 *       for(int i = 0; i < 8; i++)
 *       {
 *          boolean bit = ((value >> (7 - i)) & 0x01) != 0;
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
 * methods automatically account for the 2 necessary null bytes. 
 * 
 * 
 * 
 * FIXME
 * 
 * @author Dave Longley
 */
public class Crc16
{
   /**
    * The current CRC value.
    */
   protected int mCrcValue;
   
   /**
    * The CRC polynomial key to use.
    */
   protected int mPolynomialKey;
   
   /**
    * Creates a new Crc16 that uses a polynomial key of 0x8005.
    */
   public Crc16()
   {
      this(0x8005);
   }
   
   /**
    * Creates a new Crc16.
    * 
    * @param key the polynomial key to use.
    */
   public Crc16(int key)
   {
      // sets the polynomial key to use
      mPolynomialKey = key;
      
      // reset
      reset();
   }
   
   /**
    * Resets the CRC value to 0.
    */
   public void reset()
   {
      mCrcValue = 0;
   }
   
   /**
    * Resets the CRC value to 0xffff.
    */
   public void resetValueToMax()
   {
      mCrcValue = 0xffff;
   }
   
   /**
    * Updates the current CRC value with the given byte.
    * 
    * @param b the byte to update the CRC value with.
    */
   public void update(byte b)
   {
      // get the byte as an unsigned int
      int value = b & 0xff;
      
      // shift byte value left 8
      value <<= 8;
      
      // go through each bit
      for(int i = 0; i < 8; i++)
      {
         // shift byte value and crc left 1
         value <<= 1;
         mCrcValue <<= 1;
         
         // determine if the CRC value is divisible by the value
         if(((mCrcValue ^ value) & 0x10000) != 0)
         {
            // XOR the CRC value with the CRC polynomial
            mCrcValue ^= mPolynomialKey;
         }
      }
      
      // cut crc to 16-bits (2 bytes)
      mCrcValue &= 0xffff;
   }
   
   /**
    * Updates the current CRC value with the given byte array.
    * 
    * @param bytes the bytes to update the CRC value with.
    */
   public void update(byte[] bytes)
   {
      update(bytes, 0, bytes.length);
   }
   
   /**
    * Updates the current CRC value with the given byte array starting at
    * the given offset and using the given length as the number of bytes.
    * 
    * @param bytes the bytes to update the CRC value with.
    * @param offset the offset to start using bytes at in the passed buffer.
    * @param length the number of bytes to use from the passed buffer.
    */
   public void update(byte[] bytes, int offset, int length)
   {
      // go through every byte
      int end = offset + length;
      for(; offset < end; offset++)
      {
         update(bytes[offset]);
      }
   }
   
   /**
    * Gets the current CRC value.
    * 
    * @return the current CRC value.
    */
   public int getValue()
   {
      return mCrcValue;
   }
}
