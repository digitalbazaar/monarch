/*
 * Copyright (c) 2005 Digital Bazaar, Inc.  All rights reserved.
 */
using System;
using System.Text;
using System.Text.RegularExpressions;

using DB.Common.Logging;

namespace DB.Common
{
   /// <summary>
   /// Encodes and decodes data into/from Base64.
   /// </summary>
   ///
   /// <remarks>
   /// Base64 encoding consists of taking 3 bytes (24 bits) and converting
   /// it into 4 6-bit index values. Each index value corresponds to an
   /// alphanumeric character. Every 76 characters, an CRLF must be
   /// inserted. Decoders should ignore whitespace.
   /// 
   /// So these bytes:
   /// 
   /// b0: 00000001
   /// b1: 00000010
   /// b2: 00000011
   /// 
   /// Would be concatenated to:
   /// 
   /// 000000010000001000000011
   /// 
   /// And then split up like this:
   /// 
   /// 000000|010000|001000|000011
   /// 
   /// An algorithm for doing this would be the following,
   /// where 0x3F = 63 (6-bits):
   /// 
   /// Index 1 = b0 >> 2
   /// Index 2 = (b0 << 4 | b1 >> 4) & 0x3F 
   /// Index 3 = (b1 << 2 | b2 >> 6) & 0x3F
   /// Index 4 = b2 & 0x3F
   /// 
   ///     1   |   2    |   3   |    4
   /// 00000000
   ///   000000
   ///   000000|00010000
   ///   000000|  010000
   ///   000000|  010000|00001000
   ///   000000|  010000|  001000
   ///   000000|  010000|  001000|00000011
   ///   000000|  010000|  001000|  000011
   ///   
   ///   000000|  010000|  001000|  000011
   /// 
   /// And these values would be translated into index values:
   /// 
   /// 0, 32, 16, 3
   /// 
   /// Which would in turn translate into:
   /// 
   /// AgQD
   /// 
   /// If the byte array that is being translated is not a multiple of 3, then
   /// the bytes are translated and the '=' is used as padding.
   /// 
   /// So for 2 bytes:
   /// 
   /// 00000001
   /// 00000010
   /// 
   /// 0000000100000010
   /// 
   /// 000000|010000|001000|=
   /// 
   /// And for 1 byte:
   /// 
   /// 0000001
   /// 
   /// 000000|001000|=|=
   /// </remarks>
   ///
   /// <author>Dave Longley</author>
   public class Base64Coder
   {
      /// <summary>
      /// Base64 encoding maps 6-bit (0-63) indices to alphanumeric characters.
      /// This is the Base64 map.
      /// </summary>
      protected static readonly char[] INDEX_TO_BASE64 =
      {
         /*  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12, */   
           'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
           
         /* 13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, */   
           'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
           
         /* 26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38, */   
           'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
           
         /* 39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, */   
           'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
           
         /* 52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63 */   
           '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
      };
      
      /// <summary>
      /// This is the reverse map that maps alphanumeric characters to
      /// 6-bit (0-63) indices.
      /// </summary>
      /// 
      /// <remarks>
      /// The character '+' has the lowest integer value of all of the
      /// Base64 characters. Its value is 43, so we subtract 43 from
      /// all of the character int values to get the appropriate index.
      /// 
      /// -1 is an invalid entry, -2 is the padding entry for '='.
      /// </remarks>
      protected static readonly int[] BASE64_TO_INDEX =    
      {
         /* 43 -43 = 0*/  
         /* '+',  1,  2,  3,'/' */  
             62, -1, -1, -1, 63,
             
         /* '0','1','2','3','4','5','6','7','8','9' */
             52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
            
         /* 15, 16, 17,'=', 19, 20, 21 */
            -1, -1, -1, -2, -1, -1, -1,
         
         /* 65 - 43 = 22
         /*'A','B','C','D','E','F','G','H','I','J','K','L','M', */
             0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
            
         /* 'N','O','P','Q','R','S','T','U','V','W','X','Y','Z' */
             13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
         
         /* 91 - 43 = 48 */
         /* 48, 49, 50, 51, 52, 53 */
            -1, -1, -1, -1, -1, -1,

         /* 97 - 43 = 54
         /* 'a','b','c','d','e','f','g','h','i','j','k','l','m' */   
             26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
             
         /* 'n','o','p','q','r','s','t','u','v','w','x','y','z' */
             39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
      };
      
      /// <summary>
      /// Creates a new base 64 coder. 
      /// </summary>
      public Base64Coder()
      {
      }
      
      /// <summary>
      /// Converts a character into an integer (according to the
      /// decoder map).
      /// 
      /// <param name="c">the character to convert.</param>
      /// <returns>the integer.</returns>
      /// </summary>
      protected static int CharToInt(char c)
      {
         int rval = -1;
         
         int index = c - 43;
         rval = BASE64_TO_INDEX[index];
         
         return rval;
      }
      
      /// <summary>
      /// Encodes 3 bytes into 4 characters.
      /// 
      /// <param name="data">the byte array with the 3 bytes.</param>
      /// <param name="offset">the offset where the 3 bytes start.</param>
      /// <returns>the 4 characters.</returns>
      /// </summary>
      public static char[] EncodeGroup(byte[] data, int offset)
      {
         char[] chars = new char[4];
         
         int len = data.Length - offset;

         int b0 = data[offset++] & 0xFF;
         int b1 = (len > 1) ? data[offset++] & 0xFF : 0;
         int b2 = (len > 2) ? data[offset++] & 0xFF : 0;
         
         chars[0] = INDEX_TO_BASE64[b0 >> 2];
         chars[1] = INDEX_TO_BASE64[(b0 << 4 | b1 >> 4) & 0x3F];
         chars[2] = (len > 1) ?
            INDEX_TO_BASE64[(b1 << 2 | b2 >> 6) & 0x3F] : '=';
         chars[3] = (len > 2) ?
            INDEX_TO_BASE64[b2 & 0x3F] : '='; 
         
         return chars;
      }
      
      /// <summary>
      /// Decodes ONE group of Base64 characters into bytes.
      /// 
      /// <param name="str">the string with the characters to decode into
      /// bytes.</param>
      /// <param name="offset">the offset to begin decoding at.</param>
      /// <returns>the decoded bytes.</returns>
      /// </summary>
      public static byte[] DecodeGroup(string str, int offset)
      {
         byte[] bytes = null;

         // get 6-bit integer values
         int i0 = CharToInt(str[offset++]);
         int i1 = CharToInt(str[offset++]);
         int i2 = CharToInt(str[offset++]);
         int i3 = CharToInt(str[offset++]);
         
         if(i0 != -1 && i1 != -1 && i2 != -1 && i3 != -1)
         {
            // this byte will definitely be used, padding or not
            byte b0 = (byte)(i0 << 2 | i1 >> 4);
            
            // determine how many bytes we'll be decoding to and decode
            // -2 maps is the padding character '='
            if(i2 == -2)
            {
               bytes = new byte[1];
               bytes[0] = b0;
            }
            else if(i3 == -2)
            {
               bytes = new byte[2];
               bytes[0] = b0;
               bytes[1] = (byte)(i1 << 4 | i2 >> 2);
            }
            else
            {
               bytes = new byte[3];
               bytes[0] = b0;
               bytes[1] = (byte)(i1 << 4 | i2 >> 2);
               bytes[2] = (byte)(i2 << 6 | i3);
            }
         }
         else
         {
            LoggerManager.Error("dbcommon", 
                  "invalid character during Base64 decode");
         }
         
         return bytes;
      }
      
      /// <summary>
      /// Base64 encodes data.
      /// 
      /// <param name="data">the byte array to encode.</param>
      /// <returns>the Base64 encoded string.</returns>
      /// </summary>
      public static string Encode(byte[] data)
      {
         // get the length of the data
         int length = data.Length;
         
         // Base64 encoding requires 24 bit groups, and each
         // byte is 8 bits, so the data should be broken into groups
         // of 3 bytes each
         int groups = length / 3;
         
         // see if there is an incomplete group
         if(groups * 3 != length)
         {
            groups++;
         }
         
         // the string buffer for string the encoded data
         // Base64 encoding turns 3 bytes into 4 characters, so the
         // length of the encoded data will be:
         int encodedLength = groups * 4;
         
         // add end of line characters padding
         encodedLength += (encodedLength / 76);
         StringBuilder encoded = new StringBuilder(encodedLength);
                  
         int dataIndex = 0;
         
         // encode all the groups
         int lineLength = 0;
         for(int i = 0; i < groups; i++, dataIndex += 3)
         {
            // encode the group
            char[] chars = EncodeGroup(data, dataIndex);
            
            // Base64 allows no more than 76 characters per line
            // if the line length is greater 76, then insert a line break
            if(lineLength + chars.Length > 76)
            {
               encoded.Append("\r\n");
               lineLength = 0;
            }
            
            // update line length
            lineLength += chars.Length;

            // add the group to the buffer
            encoded.Append(chars);
         }
         
         return encoded.ToString();
      }
      
      /// <summary>
      /// Decodes a Base64-encoded string. Calls AbstractPreferences
      /// getByteArray() method which Base64 decodes a value from the internal
      /// Coder map.
      ///  
      /// <param name="str">the Base64-encoded string.</param>
      /// <returns>the decoded byte array.</returns>
      /// </summary>
      public static byte[] Decode(string str)
      {
         byte[] decoded = null;
         
         // remove all white space
         str = Regex.Replace(str, @"\s", "");

         // make sure the string has length
         int length = str.Length;
         if(length != 0)
         {
            // get and check the number of groups, must be a multiple of 4
            int groups = length / 4;
            if(groups * 4 == length)
            {
               // get the number of pad characters
               int padChars = 0;
               if(str[length - 2] == '=')
               {
                  padChars = 2;
               }
               else if(str[length - 1] == '=')
               {
                  padChars = 1;
               }
               
               // calculate the decoded length, it should be the number of
               // groups * 3 - padBytes
               int decodedLength = groups * 3 - padChars;
               
               // allocate space for the byte array
               decoded = new byte[decodedLength];
               
               int dataIndex = 0;
               int strIndex = 0;

               // decode all the groups
               for(int i = 0; i < groups; i++, strIndex += 4)
               {
                  byte[] bytes = DecodeGroup(str, strIndex);
                  
                  if(bytes == null)
                  {
                     break;
                  }

                  // copy the decoded bytes into the decoded buffer
                  int len = bytes.Length;
                  Array.Copy(bytes, 0, decoded, dataIndex, len);
                  dataIndex += len;
               }
            }
            else
            {
               LoggerManager.Error("dbcommon",
                     "Could not Base64 decode, string is not a multiple of 4!");
            }
         }
         
         return decoded;
      }
   }
}
