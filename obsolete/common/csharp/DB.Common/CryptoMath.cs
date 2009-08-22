/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Collections;
using System.IO;
using System.Security;
using System.Security.Cryptography;

namespace DB.Common
{
   /// <summary>
   /// This class is used to execute math methods on cryptography
   /// parameters/keys (big integers). The big integers are unsigned and 
   /// stored as byte arrays in base 256, with the most significant digit
   /// first.
   /// </summary>
   ///
   /// <author>Dave Longley</author>
   public class CryptoMath
   {
      /// <summary>
      /// Adds two big integers and returns the result.
      /// </summary>
      /// 
      /// <remarks>
      /// The integers are broken into digits with a base of 256. Each digit of
      /// the first number is added to the digit of the second number. Any 
      /// overflow is carried to the next digit addition. 
      /// </remarks>
      ///
      /// <param name="x">the first number to add.</param>
      /// <param name="y">the second number to add.</param>
      /// <returns>the sum: (x + y).</returns>
      public static byte[] Add(byte[] x, byte[] y)
      {
         byte[] rval = null;
         
         // get the length for the sum
         int length = (x.Length > y.Length) ? x.Length : y.Length;
         rval = new byte[length];
         
         int carry = 0;
         for(int i = 0; i < length; i++)
         {
            // get the next index for each number
            int x_i = x.Length - 1 - i;
            int y_i = y.Length - 1 - i;
            
            // get the next digit
            int a = (x_i < 0) ? 0: (int)x[x_i];
            int b = (y_i < 0) ? 0: (int)y[y_i];
            
            // add each digit and the carry
            int sum = a + b + carry;
            
            // calculate overflow (always 1 or 0)
            carry = sum >> 8;
            
            // add byte value of sum
            rval[rval.Length - 1 - i] = (byte)(sum & 0xFF);
         }
         
         // add carry digit as necessary
         if(carry != 0)
         {
            byte[] sum = new byte[length + 1];
            Array.Copy(rval, 0, sum, 1, length);
            sum[0] = (byte)carry;
            rval = sum;
         }
         
         return rval;
      }
   
      /// <summary>
      /// Subtracts one big integer from another and returns the result. If
      /// x is less than y, then 0 is returned.
      /// </summary>
      /// 
      /// <remarks>
      /// The integers are broken into digits of base 256. Each digit of the
      /// first number is subtracted from the digit of the second number. If
      /// necessary, the first number's digit will borrow from the next digit.
      /// </remarks>
      ///
      /// <param name="x">the number to subtract from.</param>
      /// <param name="y">the number to subtract.</param>
      /// <returns>the difference between the two big integers
      /// (x - y) or 0 if x is less than y.</returns>
      public static byte[] Subtract(byte[] x, byte[] y)
      {
         byte[] rval = null;
         
         if(LessThan(x, y))
         {
            rval = new byte[1];
         }
         else
         {
            // get the length for the difference
            int length = (x.Length > y.Length) ? x.Length : y.Length;
            rval = new byte[length];
            
            int borrow = 0;
            for(int i = 0; i < length; i++)
            {
               // get the next index for each number
               int x_i = x.Length - 1 - i;
               int y_i = y.Length - 1 - i;
               
               // get the next digit
               int a = (x_i < 0) ? 0: (int)x[x_i];
               int b = (y_i < 0) ? 0: (int)y[y_i];
               
               // get the amount to be subtracted (include any borrowed amount)
               int subtract = b + borrow;
               
               // see if we need to borrow
               borrow = (a < subtract) ? 1 : 0;
               
               // get the difference between the digits (add borrow * base)
               int difference = (a + borrow * 256) - subtract;
               rval[rval.Length - 1 - i] = (byte)difference;
            }
            
            // shrink array if necessary
            if(rval[0] == 0)
            {
               byte[] bytes = new byte[rval.Length - 1];
               Array.Copy(rval, 1, bytes, 0, bytes.Length);
               rval = bytes;
            }
         }
            
         return rval;
      }
      
      /// <summary>
      /// Multiplies two big integers and returns the result.
      /// </summary>
      /// 
      /// <remarks>
      /// The integers are broken into digits of base 256. Each digit of
      /// the first number is multiplied by each digit of the second number.
      /// Each digit product is summed with the previous digit product.
      /// 
      /// <pre>
      ///   42
      ///  x66
      /// ----
      ///  252
      /// 2520
      /// ----
      /// 2772 // sum of digit products
      /// </pre>
      /// 
      /// </remarks>
      ///
      /// <param name="x">the first number to multiply.</param>
      /// <param name="y">the second number to multiply by.</param>
      /// <returns>the product: (x * y).</returns>
      public static byte[] Multiply(byte[] x, byte[] y)
      {
         byte[] rval = null;
         
         // for storing partial products and the total product
         ArrayList product = new ArrayList();
         
         for(int i = 0; i < x.Length; i++)
         {
            // get next x digit index
            int x_i = x.Length - 1 - i;
            
            int carry = 0;
            for(int j = 0; j < y.Length; j++)
            {
               // get next y digit index
               int y_i = y.Length - 1 - j;
               
               // for storing the current digit product
               int dp = 0;
               
               // add new digit product if necessary
               if((i + j) == product.Count)
               {
                  product.Add((byte)dp);
               }
               else
               {
                  // get existing digit product
                  dp = (byte)product[i + j];
               }
               
               // multiply by this digit and add the digit product and carry
               dp = x[x_i] * y[y_i] + dp + carry;
               
               // update the digit product and set the new carry
               product[i + j] = (byte)(dp & 0xFF);
               carry = dp >> 8;
            }
            
            // add new digit product for carry as necessary
            if(carry != 0)
            {
               if((i + y.Length) == product.Count)
               {
                  product.Add((byte)carry);
               }
               else
               {
                  product[i + y.Length] = (byte)carry;
               }
            }
         }
         
         // copy array and reverse it
         rval = new byte[product.Count];
         product.CopyTo(rval, 0);
         Array.Reverse(rval);
         
         return rval;
      }
      
      /// <summary>
      /// Divides one big integer into another.
      /// </summary>
      ///
      /// <param name="x">the number to be divided.</param>
      /// <param name="y">the number to divide with.</param>
      /// <returns>the quotient: (x / y).</returns>
      public static byte[] Divide(byte[] x, byte[] y)
      {
         byte[] rval = new byte[0];
         
         return rval;
      }
      
      /// <summary>
      /// Raises one big integer to the power of another.
      /// </summary>
      ///
      /// <param name="x">the base to be raised.</param>
      /// <param name="y">the exponent.</param>
      /// <returns>the power: (x^y).</returns>
      public static byte[] Pow(byte[] x, byte[] y)
      {
         byte[] rval = new byte[0];
         
         return rval;
      }
      
      /// <summary>
      /// Mods one big integer by another.
      /// </summary>
      ///
      /// <param name="x">the number to be modded.</param>
      /// <param name="y">the modulus.</param>
      /// <returns>the modulus: (x mod y).</returns>
      public static byte[] Mod(byte[] x, byte[] y)
      {
         byte[] rval = new byte[0];
         
         return rval;
      }
      
      /// <summary>
      /// Determines the log of a big integer.
      /// </summary>
      ///
      /// <param name="x">the number to determine the log of.</param>
      /// <param name="y">the base of the log.</param>
      /// <returns>the log, base y, of x.</returns>
      public static byte[] Log(byte[] x, byte[] y)
      {
         byte[] rval = new byte[0];
         
         return rval;
      }
      
      /// <summary>
      /// Determines if one big integer is less than another.
      /// </summary>
      /// 
      /// <param name="x">the left operand in the less-than operation.</param>
      /// <param name="y">the right operand in the less-than operation.</param>
      /// <returns>true if x is less than y, false if not.</returns>
      public static bool LessThan(byte[] x, byte[] y)
      {
         bool rval = false;
         
         if(x.Length < y.Length)
         {
            rval = true;
         }
         else if(x.Length == y.Length)
         {
            for(int i = x.Length - 1; i >= 0; i--)
            {
               if(x[i] < y[i])
               {
                  // if the digit of x is less than the digit of y
                  // then x is less than y
                  rval = true;
                  break;
               }
               else if(x[i] > y[i])
               {
                  // if the digit of x is greater than the digit of y
                  // then x is greater than y
                  break;
               }
            }
         }
         
         return rval;
      }
   }
}
