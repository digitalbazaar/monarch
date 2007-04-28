#include "Base64Coder.h"

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
   std::string& str, std::string find, std::string replace)
{
	unsigned int found = str.find(find);
	while(found != std::string::npos)
	{
		str.replace(found, find.length(), replace);
		found = str.find(find);
	}
}

int Base64Coder::charToInt(char c)
{
   int rval = -1;
   
   int index = c - 43;
   rval = BASE64_TO_INDEX[index];
   
   return rval;
}

void Base64Coder::encodeGroup(char* data, int offset, int length, char* group)
{
   int len = length - offset;
   
   int b0 = data[offset++] & 0xFF;
   int b1 = (len > 1) ? data[offset++] & 0xFF : 0;
   int b2 = (len > 2) ? data[offset++] & 0xFF : 0;
   
   group[0] = INDEX_TO_BASE64[b0 >> 2];
   group[1] = INDEX_TO_BASE64[(b0 << 4 | b1 >> 4) & 0x3F];
   group[2] = (len > 1) ? INDEX_TO_BASE64[(b1 << 2 | b2 >> 6) & 0x3F] : '=';
   group[3] = (len > 2) ? INDEX_TO_BASE64[b2 & 0x3F] : '='; 
}

int Base64Coder::decodeGroup(std::string str, int offset, char* bytes)
{
	int rval = 0;
	
   // get 6-bit integer values
   int index[4];
   for(int i = 0; i < 4; i++)
   {
   	index[i] = charToInt(str[offset++]);
   }
   
   // this byte will definitely be used, padding or not
   char b0 = (char)(index[0] << 2 | index[1] >> 4);
   
   // determine how many bytes we'll be decoding to and decode
   // -2 maps is the padding character '='
   if(index[2] == -2)
   {
   	// 1 decoded byte
   	rval = 1;
      bytes[0] = b0;
   }
   else if(index[3] == -2)
   {
   	// 2 decoded bytes 
      rval = 2;
      bytes[0] = b0;
      bytes[1] = (char)(index[1] << 4 | index[2] >> 2);
   }
   else
   {
   	// 3 decoded bytes
      rval = 3;
      bytes[0] = b0;
      bytes[1] = (char)(index[1] << 4 | index[2] >> 2);
      bytes[2] = (char)(index[2] << 6 | index[3]);
   }
   
   return rval;
}

std::string Base64Coder::encode(char* data, int offset, int length)
{
   std::string rval = "";
   
   if(data != NULL && length > 0)
   {
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
      
      // encode all the groups
      int lineLength = 0;
      for(int i = 0; i < groups; i++, offset += 3)
      {
         // encode the group
         char group[4];
         encodeGroup(data, offset, length, group);
         
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

char* Base64Coder::decode(std::string str)
{
   char* rval = NULL;
   
   // remove all white space
   replaceAll(str, " ", "");
   replaceAll(str, "\n", "");
   replaceAll(str, "\r", "");
   replaceAll(str, "\t", "");
   
   // make sure the string has length
   int length = str.size();
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
         rval = new char[decodedLength];
         
         int dataIndex = 0;
         int strIndex = 0;
         
         // decode all the groups
         for(int i = 0; i < groups; i++, strIndex += 4)
         {
            char bytes[3];
            int len = decodeGroup(str, strIndex, bytes);
            
            // copy the decoded bytes into the decoded buffer
            memcpy(rval + dataIndex, bytes, len);
            dataIndex += len;
         }
      }
   }
   
   return rval;
}
