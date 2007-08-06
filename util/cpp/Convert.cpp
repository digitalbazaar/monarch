/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Convert.h"
#include "StringTools.h"

#include <sstream>

using namespace std;
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
   string rval;
   
   // convert bytes to unsigned char to ensure data integrity
   unsigned char* ubytes = (unsigned char*)bytes;
   for(unsigned int i = 0; i < length; i++)
   {
      // hexadecimal uses 2 digits, each with 16 values (or 4 bits):
      
      // convert the top 4 bits
      rval += HEX_CHARS[(ubytes[i] >> 4)];
      
      // convert the bottom 4 bits
      rval += HEX_CHARS[(ubytes[i] & 0x0f)];
   }
   
   return rval;
}

string Convert::bytesToUpperHex(const char* bytes, unsigned int length)
{
   string rval;
   
   // convert bytes to unsigned char to ensure data integrity
   unsigned char* ubytes = (unsigned char*)bytes;
   for(unsigned int i = 0; i < length; i++)
   {
      // hexadecimal uses 2 digits, each with 16 values (or 4 bits):
      
      // convert the top 4 bits
      rval += UPPER_HEX_CHARS[(ubytes[i] >> 4)];
      
      // convert the bottom 4 bits
      rval += UPPER_HEX_CHARS[(ubytes[i] & 0x0f)];
   }
   
   return rval;
}

void Convert::hexToBytes(
   const char* hex, unsigned int hexLength, char* bytes, unsigned int& length)
{
   length = 0;
   
   // convert bytes to unsigned
   unsigned char* ubytes = (unsigned char*)bytes;
   
   unsigned char c1;
   unsigned char c2;
   for(unsigned int i = 0; i < hexLength; i += 2)
   {
      // hexadecimal uses 2 digits, each with 16 values
      if(i + 1 < hexLength)
      {
         c1 = hex[i];
         c2 = hex[i + 1];
         
         // ASCII '0' = 48, ASCII 'A' = 65, ASCII 'a' = 97
         // so subtract 48, so subtract 55, so subtract 87
         
         // convert first hex digit
         if(c1 > 47 && c1 < 58)
         {
            c1 = 16 * (c1 - 48);
         }
         else if(c1 > 64 && c1 < 91)
         {
            c1 = 16 * (c1 - 55);
         }
         else if(c1 > 96 && c1 < 123)
         {
            c1 = 16 * (c1 - 87);
         }
         
         // convert second hex digit and set byte
         if(c2 > 47 && c2 < 58)
         {
            ubytes[length++] = c1 + c2 - 48;
         }
         else if(c2 > 64 && c2 < 91)
         {
            ubytes[length++] = c1 + c2 - 55;
         }
         else if(c2 > 96 && c2 < 123)
         {
            ubytes[length++] = c1 + c2 - 87;
         }
      }
   }
}

int Convert::hexToInt(const char* hex, unsigned int hexLength)
{
   int rval = 0;
   
   unsigned int base = 1;
   if(hexLength > 1)
   {
      for(int i = 0; i < (hexLength - 2); i++, base *= 16);
   }
   
   unsigned char c1;
   unsigned char c2;
   for(unsigned int i = 0; i < hexLength; i += 2)
   {
      // hexadecimal uses 2 digits, each with 16 values
      if(i + 1 < hexLength)
      {
         c1 = hex[i];
         c2 = hex[i + 1];
         
         // ASCII '0' = 48, ASCII 'A' = 65, ASCII 'a' = 97
         // so subtract 48, so subtract 55, so subtract 87
         
         // convert first hex digit
         if(c1 > 47 && c1 < 58)
         {
            rval += base * 16 * (c1 - 48);
         }
         else if(c1 > 64 && c1 < 91)
         {
            rval += base * 16 * (c1 - 55);
         }
         else if(c1 > 96 && c1 < 123)
         {
            rval += base * 16 * (c1 - 87);
         }
         
         // convert second hex digit and add
         if(c2 > 47 && c2 < 58)
         {
            rval += base * (c2 - 48);
         }
         else if(c2 > 64 && c2 < 91)
         {
            rval += base * (c2 - 55);
         }
         else if(c2 > 96 && c2 < 123)
         {
            rval += base * (c2 - 87);
         }
      }
      
      // decrease base
      base /= 256;
   }
   
   return rval;
}

string Convert::intToHex(int n)
{
   string rval;
   
   char ch;
   for(int i = 2 * sizeof(int) - 1; i >= 0; i--)
   {
      ch = HEX_CHARS[(n >> (i * 4)) & 0x0F];
      if(ch != '0' || rval.length() > 0)
      {
         rval.append(1, ch);
      }
   }
   
   // insert single leading zero as necessary
   if(rval.length() % 2 != 0)
   {
      rval.insert(rval.begin(), '0');
   }
   
   return rval;
}

string Convert::intToUpperHex(int n)
{
   string rval;
   
   char ch;
   for(int i = 2 * sizeof(int) - 1; i >= 0; i--)
   {
      ch = UPPER_HEX_CHARS[(n >> (i * 4)) & 0x0F];
      if(ch != '0' || rval.length() > 0)
      {
         rval.append(1, ch);
      }
   }
   
   // insert single leading zero as necessary
   if(rval.length() % 2 != 0)
   {
      rval.insert(rval.begin(), '0');
   }
   
   return rval;
}

string Convert::integerToString(long long n)
{
   ostringstream oss;
   oss << n;
   return oss.str();
}

bool Convert::stringToInteger(const char* s, long long& n, int base)
{
   char* endptr = NULL;
   n = strtoll(s, &endptr, base);
   return endptr != s && *endptr == '\0';
}
