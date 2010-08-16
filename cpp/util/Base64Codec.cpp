/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/util/Base64Codec.h"

#include <cstring>
#include <cstdlib>
#include <cctype>

using namespace std;
using namespace monarch::util;

// Special values for maps
// invalid
#define _X '\xff'
// symbol
#define _S '\xfe'
#define _SS "\xfe"
// padding
#define _P '\xfd'

/**
 * Base64 encoding maps 6-bit (0-63) values to alphanumeric characters or
 * special codes. This is the Base64 map. According to RFC 3548 the standard
 * characters are [A-Za-z0-9+/]. "+" and "/" have issues due to URL encoding
 * rules and filename formats. Alternate alphabets for URLs and filenames are
 * possible. One common standard is "-_". In this map those symbols are mapped
 * to special values outside of [0-127] as noted above.
 */
static const char _bitsToBase64Map[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" _SS _SS;

const char Base64Codec::sStandardSymChars[] = "+/";
const char Base64Codec::sUrlSymChars[] = "-_";

/**
 * This is the reverse map that maps 7-bit characters to 6-bit (0-61) values.
 * The map has 128 values most of which are always invalid.
 *
 * The following values are allowed in the map:
 *    [0-61]: valid output bits
 *    _X: is an invalid entry
 *    _S: is potentially in the symbol map
 *    _P: is the padding entry for '='.
 */
static const char _base64ToBitsMap[] =
{
   /* NUL,SOH,STX,ETX,EOT,ENQ,ACK,BEL, BS, HT, LF, VT, FF, CR, SO, SI, */
       _X, _X, _X, _X, _X, _X, _X, _X, _X, _X, _X, _X, _X, _X, _X, _X,
   /* DLE,DC1,DC2,DC3,DC4,NAK,SYN,ETB,CAN, EM,SUB,ESC, FS, GS, RS, US, */
       _X, _X, _X, _X, _X, _X, _X, _X, _X, _X, _X, _X, _X, _X, _X, _X,
   /* ' ','!','"','#','$','%','&',''','(',')','*','+',',','-','.','/', */
       _X, _S, _S, _S, _S, _S, _S, _S, _S, _S, _S, _S, _S, _S, _S, _S,
   /* '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?', */
       52, 53, 54, 55, 56, 57, 58, 59, 60, 61, _S, _S, _S, _P, _S, _S,
   /* '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O', */
       _S,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
   /* 'P','Q','R','S','T','U','V','W','X','Y','Z','[','\',']','^','_', */
       15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, _S, _S, _S, _S, _S,
   /* '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o', */
       _S, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
   /* 'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~',DEL  */
       41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, _S, _S, _S, _S, _X
};

/**
 * Converts a character into a 6-bit integer (according to the
 * decoder map). Characters outside of the base-64 and symbol map character
 * set will produce undefined output.
 *
 * @param c the character to convert.
 * @param symMap array of 2 symbol characters.
 *
 * @return the 6-bit integer.
 */
static inline char _base64ToBits(char c, const char symMap[])
{
   char rval;

   // lookup bits and ensure [0-127] range
   rval = _base64ToBitsMap[c & 0x7F];

   // potential symbol
   if(rval == _S)
   {
      if(c == symMap[0])
      {
         rval = 62;
      }
      else if(c == symMap[1])
      {
         rval = 63;
      }
      else
      {
         // invalid
         rval = _X;
      }
   }

   return rval;
}

/**
 * Converts a 6-bit integer into a character (according to the
 * encoder map).
 *
 * @param c the bits to convert.
 * @param symMap array of 2 symbol characters.
 *
 * @return the 6-bit integer.
 */
static inline char _bitsToBase64(unsigned char bits, const char symMap[])
{
   char rval;

   // bits always in [0-64], no range check needed
   rval = _bitsToBase64Map[bits];

   if(rval == _S)
   {
      // know its 62 || 63, so just mask low bit and lookup in 2 element map
      rval = symMap[bits & 0x1];
   }

   return rval;
}

/**
 * Encodes 3 bytes into 4 characters.
 *
 * @param data the byte array with the 3 bytes.
 * @param length the length of the byte array.
 * @param group the group to populate with 4 characters.
 * @param symMap array of 2 symbol characters.
 */
static void _encodeGroup(
   const char* data, unsigned int length, char* group, const char symMap[])
{
   unsigned char b0 = data[0] & 0xff;
   unsigned char b1 = (length > 1) ? data[1] & 0xff : 0;
   unsigned char b2 = (length > 2) ? data[2] & 0xff : 0;

   group[0] = _bitsToBase64(b0 >> 2, symMap);
   group[1] = _bitsToBase64((b0 << 4 | b1 >> 4) & 0x3f, symMap);
   group[2] =
      (length > 1) ? _bitsToBase64((b1 << 2 | b2 >> 6) & 0x3f, symMap) : '=';
   group[3] = (length > 2) ? _bitsToBase64(b2 & 0x3f, symMap) : '=';
}

/**
 * Decodes ONE group of Base64 characters into bytes.
 *
 * @param str the characters to decode into bytes.
 * @param bytes the decoded byte array to populate (of size 3).
 * @param length to store the number of decoded bytes (1 to 3).
 * @param symMap array of 2 symbol characters.
 */
static void _decodeGroup(
   const char* str, char* bytes, unsigned int& length, const char symMap[])
{
   // get 6-bit integer values
   char index[4];
   for(int i = 0; i < 4; ++i)
   {
      index[i] = _base64ToBits(str[i], symMap);
   }

   // this byte will definitely be used, padding or not
   char b0 = (char)(index[0] << 2 | index[1] >> 4);

   // determine how many bytes we'll be decoding to and decode
   // _P maps is the padding character '='
   if(index[2] == _P)
   {
      // 1 decoded byte
      length = 1;
      bytes[0] = b0;
   }
   else if(index[3] == _P)
   {
      // 2 decoded bytes
      length = 2;
      bytes[0] = b0;
      bytes[1] = (char)(index[1] << 4 | index[2] >> 2);
   }
   else
   {
      // 3 decoded bytes
      length = 3;
      bytes[0] = b0;
      bytes[1] = (char)(index[1] << 4 | index[2] >> 2);
      bytes[2] = (char)(index[2] << 6 | index[3]);
   }
}

string Base64Codec::encode(
   const char* data, unsigned int length, size_t maxLineLength,
   const char symChars[])
{
   string rval;

   if(data != NULL && length > 0)
   {
      // Base64 encoding requires 24 bit groups, and each
      // byte is 8 bits, so the data should be broken into groups
      // of 3 bytes each
      unsigned int groups = length / 3;

      // see if there is an incomplete group
      if(groups * 3 != length)
      {
         ++groups;
      }

      // encode all the groups
      char group[4];
      unsigned int offset = 0;
      unsigned int lineLength = 0;
      for(unsigned int i = 0; i < groups; ++i, offset += 3)
      {
         // encode the group
         _encodeGroup(data + offset, length - offset, group, symChars);

         // Insert a line break if line will be too long.
         if(maxLineLength > 0 && lineLength + 4 > maxLineLength)
         {
            rval.push_back('\r');
            rval.push_back('\n');
            lineLength = 0;
         }

         // update line length
         lineLength += 4;

         // add the group to the buffer
         rval.append(group, 4);
      }
   }

   return rval;
}

string Base64Codec::urlSafeEncode(const char* data, unsigned int length)
{
   return encode(data, length, 0, sUrlSymChars);
}

void Base64Codec::decode(
   const char* str, char** data, unsigned int& length, const char symChars[])
{
   // point data at NULL and set length to 0
   *data = NULL;
   length = 0;

   // remove all white space
   unsigned int oldLength = strlen(str);
   char temp[oldLength];
   unsigned int len = 0;
   for(unsigned int i = 0; i < oldLength; ++i)
   {
      if(!isspace(str[i]))
      {
         temp[len++] = str[i];
      }
   }
   temp[len] = 0;

   // make sure the string is at least 4 characters
   if(len > 3)
   {
      // get and check the number of groups, must be a multiple of 4
      unsigned int groups = len / 4;
      if(groups * 4 == len)
      {
         // get the number of pad characters
         unsigned int padChars = 0;
         if(temp[len - 2] == '=')
         {
            padChars = 2;
         }
         else if(temp[len - 1] == '=')
         {
            padChars = 1;
         }

         // calculate the decoded length, it should be the number of
         // groups * 3 - padBytes
         length = groups * 3 - padChars;

         // allocate space for the byte array
         *data = (char*)malloc(length);

         // decode all the groups
         char bytes[3];
         unsigned int dataIndex = 0;
         unsigned int strIndex = 0;
         for(unsigned int i = 0; i < groups; ++i, strIndex += 4)
         {
            // copy the decoded bytes into the decoded buffer
            _decodeGroup(temp + strIndex, bytes, len, symChars);
            memcpy((*data) + dataIndex, bytes, len);
            dataIndex += len;
         }
      }
   }
}

void Base64Codec::urlSafeDecode(
   const char* str, char** data, unsigned int& length)
{
   decode(str, data, length, sUrlSymChars);
}
