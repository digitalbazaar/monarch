/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Base64Coder.h"

using namespace std;
using namespace db::util;

const char Base64Coder::INDEX_TO_BASE64[] =
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

const int Base64Coder::BASE64_TO_INDEX[] =
{
   /* 43 -43 = 0*/  
   /* '+',  1,  2,  3,'/' */  
       62, -1, -1, -1, 63,
       
   /* '0','1','2','3','4','5','6','7','8','9' */
       52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
      
   /* 15, 16, 17,'=', 19, 20, 21 */
      -1, -1, -1, -2, -1, -1, -1,
   
   /* 65 - 43 = 22*/
   /*'A','B','C','D','E','F','G','H','I','J','K','L','M', */
       0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
      
   /* 'N','O','P','Q','R','S','T','U','V','W','X','Y','Z' */
       13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
   
   /* 91 - 43 = 48 */
   /* 48, 49, 50, 51, 52, 53 */
      -1, -1, -1, -1, -1, -1,

   /* 97 - 43 = 54*/
   /* 'a','b','c','d','e','f','g','h','i','j','k','l','m' */   
       26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
       
   /* 'n','o','p','q','r','s','t','u','v','w','x','y','z' */
       39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};

void Base64Coder::replaceAll(
   string& str, string find, string replace)
{
	string::size_type found = str.find(find);
	while(found != string::npos)
	{
		str.replace(found, find.length(), replace);
		found = str.find(find);
	}
}

int Base64Coder::charToInt(const char& c)
{
   int rval = -1;
   
   int index = c - 43;
   rval = BASE64_TO_INDEX[index];
   
   return rval;
}

void Base64Coder::encodeGroup(
   const char* data, unsigned int length, char* group)
{
   unsigned int b0 = data[0] & 0xff;
   unsigned int b1 = (length > 1) ? data[1] & 0xff : 0;
   unsigned int b2 = (length > 2) ? data[2] & 0xff : 0;
   
   group[0] = INDEX_TO_BASE64[b0 >> 2];
   group[1] = INDEX_TO_BASE64[(b0 << 4 | b1 >> 4) & 0x3f];
   group[2] = (length > 1) ? INDEX_TO_BASE64[(b1 << 2 | b2 >> 6) & 0x3f] : '=';
   group[3] = (length > 2) ? INDEX_TO_BASE64[b2 & 0x3f] : '='; 
}

void Base64Coder::decodeGroup(
   const char* str, char* bytes, unsigned int& length)
{
   // get 6-bit integer values
   int index[4];
   for(int i = 0; i < 4; i++)
   {
   	index[i] = charToInt(str[i]);
   }
   
   // this byte will definitely be used, padding or not
   char b0 = (char)(index[0] << 2 | index[1] >> 4);
   
   // determine how many bytes we'll be decoding to and decode
   // -2 maps is the padding character '='
   if(index[2] == -2)
   {
   	// 1 decoded byte
   	length = 1;
      bytes[0] = b0;
   }
   else if(index[3] == -2)
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

string Base64Coder::encode(const char* data, unsigned int length)
{
   string rval = "";
   
   if(data != NULL && length > 0)
   {
      // Base64 encoding requires 24 bit groups, and each
      // byte is 8 bits, so the data should be broken into groups
      // of 3 bytes each
      unsigned int groups = length / 3;
      
      // see if there is an incomplete group
      if(groups * 3 != length)
      {
         groups++;
      }
      
      // the string buffer for string the encoded data
      // Base64 encoding turns 3 bytes into 4 characters, so the
      // length of the encoded data will be:
      unsigned int encodedLength = groups * 4;
      
      // add end of line characters padding
      encodedLength += (encodedLength / 76);
      
      // encode all the groups
      char group[4];
      unsigned int offset = 0;
      unsigned int lineLength = 0;
      for(unsigned int i = 0; i < groups; i++, offset += 3)
      {
         // encode the group
         encodeGroup(data + offset, length - offset, group);
         
         // Base64 allows no more than 76 characters per line
         // if the line length is greater 76, then insert a line break
         if(lineLength + 4 > 76)
         {
            rval.append("\r\n");
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

void Base64Coder::decode(const string& str, char** data, unsigned int& length)
{
   // point data at NULL and set length to 0
   *data = NULL;
   length = 0;
   
   // remove all white space
   string input = str;
   replaceAll(input, " ", "");
   replaceAll(input, "\n", "");
   replaceAll(input, "\r", "");
   replaceAll(input, "\t", "");
   
   // make sure the string has length
   unsigned int len = input.length();
   if(len != 0)
   {
      // get and check the number of groups, must be a multiple of 4
      unsigned int groups = len / 4;
      if(groups * 4 == len)
      {
         // get the number of pad characters
         unsigned int padChars = 0;
         if(input[len - 2] == '=')
         {
            padChars = 2;
         }
         else if(input[len - 1] == '=')
         {
            padChars = 1;
         }
         
         // calculate the decoded length, it should be the number of
         // groups * 3 - padBytes
         length = groups * 3 - padChars;
         
         // allocate space for the byte array
         *data = new char[length];
         
         // decode all the groups
         char bytes[3];
         unsigned int dataIndex = 0;
         unsigned int strIndex = 0;
         for(unsigned int i = 0; i < groups; i++, strIndex += 4)
         {
            // copy the decoded bytes into the decoded buffer
            decodeGroup(input.c_str() + strIndex, bytes, len);
            memcpy((*data) + dataIndex, bytes, len);
            dataIndex += len;
         }
      }
   }
}
