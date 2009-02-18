/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/util/Convert.h"

#include "db/util/StringTools.h"
#include "db/rt/Exception.h"

//#include <sstream>

using namespace std;
using namespace db::rt;
using namespace db::util;

// initialize hexadecimal characters strings for fast lookups
const char Convert::HEX_CHARS[16] =
{
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
   'a', 'b', 'c', 'd', 'e', 'f'
};

const char Convert::UPPER_HEX_CHARS[16] =
{
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
   'A', 'B', 'C', 'D', 'E', 'F'
};

string Convert::bytesToHex(const char* bytes, unsigned int length)
{
   // convert bytes to unsigned char to ensure data integrity
   char hex[length * 2 + 1];
   char* ptr = hex;
   unsigned char* ubytes = (unsigned char*)bytes;
   for(unsigned int i = 0; i < length; i++, ptr += 2)
   {
      // hexadecimal uses 2 digits, each with 16 values (or 4 bits):
      
      // convert the top 4 bits
      ptr[0] = HEX_CHARS[(ubytes[i] >> 4)];
      
      // convert the bottom 4 bits
      ptr[1] = HEX_CHARS[(ubytes[i] & 0x0f)];
   }
   ptr[0] = 0;
   
   return hex;
}

string Convert::bytesToUpperHex(const char* bytes, unsigned int length)
{
   // convert bytes to unsigned char to ensure data integrity
   char hex[length * 2 + 1];
   char* ptr = hex;
   unsigned char* ubytes = (unsigned char*)bytes;
   for(unsigned int i = 0; i < length; i++, ptr += 2)
   {
      // hexadecimal uses 2 digits, each with 16 values (or 4 bits):
      
      // convert the top 4 bits
      ptr[0] = UPPER_HEX_CHARS[(ubytes[i] >> 4)];
      
      // convert the bottom 4 bits
      ptr[1] = UPPER_HEX_CHARS[(ubytes[i] & 0x0f)];
   }
   ptr[0] = 0;
   
   return hex;
}

static inline bool nibbleToByte(const char hex, unsigned char& value)
{
   bool rval = true;
   
   if(hex >= '0' && hex <= '9')
   {
      value = hex - '0';
   }
   else if(hex >= 'A' && hex <= 'F')
   {
      value = hex - 'A' + 0xa;
   }
   else if(hex >= 'a' && hex <= 'f')
   {
      value = hex - 'a' + 0xa;
   }
   else
   {
      rval = false;
   }
   
   return rval;
}
   
static inline bool hexToByte(
   const char high, const char low, unsigned char& value)
{
   bool rval;
   unsigned char temp;
   rval = nibbleToByte(high, temp) && nibbleToByte(low, value);
   if(rval)
   {
      value += (temp << 4);
   }
   return rval;
}

bool Convert::hexToBytes(
   const char* hex, unsigned int hexLength, char* bytes, unsigned int& length)
{
   bool rval = true;

   length = 0;
   
   // convert bytes to unsigned
   unsigned char* ubytes = (unsigned char*)bytes;
   
   unsigned char c1;
   unsigned char c2;
   unsigned int i;

   if(hexLength % 2 == 0)
   {
      // even # of characters
      i = 0;
   }
   else
   {
      // odd # of characters, prepend initial '0' and convert first char
      i = 1;
      c1 = '0';
      c2 = hex[0];
      rval = hexToByte(c1, c2, ubytes[length++]);
   }

   // convert the rest
   for(; rval && i < hexLength; i += 2)
   {
      c1 = hex[i];
      c2 = hex[i + 1];
      rval = hexToByte(c1, c2, ubytes[length++]);
   }
   
   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Invalid hex value!", "db.util.ConversionError");
      Exception::setLast(e, false);
   }
   
   return rval;
}

bool Convert::hexToInt(
   const char* hex, unsigned int hexLength, unsigned int& value)
{
   bool rval = true;
   
   if(hexLength > (sizeof(unsigned int) * 2))
   {
      rval = false;
      ExceptionRef e = new Exception(
         "Hex value too large!", "db.util.ConversionError");
      Exception::setLast(e, false);
   }
   else
   {
      // convert backwards and shift to proper position
      value = 0;
      for(int i = hexLength - 1; rval && i >= 0; i--)
      {
         unsigned char temp;
         rval = nibbleToByte(hex[i], temp);
         if(rval)
         {
            value |= (temp << (4 * (hexLength - i - 1)));
         }
      }
   }
   
   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Invalid hex value!", "db.util.ConversionError");
      Exception::setLast(e, false);
   }
   
   return rval;
}

string Convert::intToHex(unsigned int n)
{
   string rval;
   
   char ch;
   for(int i = 2 * sizeof(int) - 1; i >= 0; i--)
   {
      ch = HEX_CHARS[(n >> (i * 4)) & 0x0F];
      if(ch != '0' || rval.length() > 0)
      {
         rval.push_back(ch);
      }
   }
   
   // insert single leading zero as necessary
   if(rval.length() % 2 != 0)
   {
      rval.insert(rval.begin(), '0');
   }
   
   return rval;
}

string Convert::intToUpperHex(unsigned int n)
{
   string rval;
   
   char ch;
   for(int i = 2 * sizeof(int) - 1; i >= 0; i--)
   {
      ch = UPPER_HEX_CHARS[(n >> (i * 4)) & 0x0F];
      if(ch != '0' || rval.length() > 0)
      {
         rval.push_back(ch);
      }
   }
   
   // insert single leading zero as necessary
   if(rval.length() % 2 != 0)
   {
      rval.insert(rval.begin(), '0');
   }
   
   return rval;
}

// Note: Commented out because the C functions are simple enough --
// just use them and they are faster.
//string Convert::integerToString(long long n)
//{
//   ostringstream oss;
//   oss << n;
//   return oss.str();
//}
//
//bool Convert::stringToInteger(const char* s, long long& n, int base)
//{
//   char* endptr = NULL;
//   n = strtoll(s, &endptr, base);
//   return endptr != s && *endptr == '\0';
//}
