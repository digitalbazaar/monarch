/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Convert.h"

using namespace std;
using namespace db::util;

// initialize hexadecimal characters string
const char Convert::HEX_CHARS[16] =
{
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
   'a', 'b', 'c', 'd', 'e', 'f'
};

string Convert::bytesToHex(const char* bytes, unsigned int length)
{
   string rval = "";
   
   // convert bytes to unsigned char to ensure data integrity
   unsigned char* input = (unsigned char*)bytes;
   for(unsigned int i = 0; i < length; i++)
   {
      // hexadecimal uses 2 digits, each with 16 values (or 4 bits):
      
      // convert the top 4 bits
      rval.append(&HEX_CHARS[(input[i] >> 4)], 1);
      
      // convert the bottom 4 bits
      rval.append(&HEX_CHARS[(input[i] & 0x0f)], 1);
   }
   
   return rval;
}
