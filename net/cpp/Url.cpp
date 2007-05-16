/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "Url.h"
#include "Convert.h"

using namespace std;
using namespace db::net;
using namespace db::util;

Url::Url(const string& url) throw(MalformedUrlException)
{
   // find the first colon
   string::size_type index = url.find_first_of(':', 0);
   if(index == string::npos)
   {
      // no color found
      throw MalformedUrlException("Url is missing a colon!");
   }
   
   // split string into the scheme and scheme-specific-part
   mScheme = url.substr(0, index);
   
   // check scheme for validity
   char c;
   for(string::iterator i = mScheme.begin(); i != mScheme.end(); i++)
   {
      // character must be 'a'-'z', '+', '.', or '-'
      c = *i;
      if(c > 'z')
      {
         throw MalformedUrlException(
            "Url scheme contains invalid characters!");
      }
      
      if(c < 'a' && (c != '+' || c != '.' || c != '-'))
      {
         throw MalformedUrlException(
            "Url scheme contains invalid characters!");
      }
   }
   
   if(index != url.length() - 1)
   {
      mSchemeSpecificPart = url.substr(index + 1);
   }
}

Url::~Url()
{
}

const string& Url::getScheme()
{
   return mScheme;
}

const string& Url::getSchemeSpecificPart() 
{
   return mSchemeSpecificPart;
}

string Url::encode(const char* str, unsigned int length)
{
   string rval;
   
   char c;
   for(unsigned int i = 0; i < length; i++)
   {
      c = str[i];
      
      // see if the character is "safe" (0-9, A-Z, or a-z)
      if(c >= '0' && c <= '9') 
      {
         rval += c;
      }
      else if(c >= 'A' && c <= 'Z')
      {
         rval += c;
      }
      else if(c >= 'a' && c <= 'z')
      {
         rval += c;
      }
      else if(c == ' ')
      {
         // substitute pluses for spaces
         rval += '+';
      }
      else
      {
         // character is unsafe, so add a '%' and convert to hex
         rval += '%';
         rval += Convert::bytesToUpperHex(&c, 1);
      }
   }
   
   return rval;
}

string Url::decode(const char* str, unsigned int length)
{
   string rval;
   
   char c;
   unsigned int cLength;
   for(unsigned int i = 0; i < length; i++)
   {
      c = str[i];
      
      // see if the character is "safe" (0-9, A-Z, or a-z)
      if(c >= '0' && c <= '9') 
      {
         rval += c;
      }
      else if(c >= 'A' && c <= 'Z')
      {
         rval += c;
      }
      else if(c >= 'a' && c <= 'z')
      {
         rval += c;
      }
      else if(c == '+')
      {
         // substitute spaces for pluses
         rval += ' ';
      }
      else if(c == '%')
      {
         // looking for 3 characters
         if(i + 2 < length)
         {
            Convert::hexToBytes(str + i + 1, 2, &c, cLength);
            rval += c;
         }
         
         // skip two characters
         i += 2;
      }
   }
   
   return rval;
}
