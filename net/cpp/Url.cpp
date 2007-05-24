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
   string::size_type index = url.find(':');
   if(index == string::npos)
   {
      // no colon found
      throw MalformedUrlException("Url is missing a colon!");
   }
   
   // split string into the scheme and scheme-specific-part
   mScheme = url.substr(0, index);
   
   // make scheme lower case
   transform(mScheme.begin(), mScheme.end(), mScheme.begin(), tolower);
   
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
      // get scheme specific part
      mSchemeSpecificPart = url.substr(index + 1);
      
      // get authority, path, and query:
      
      // authority is preceeded by double slash "//" and
      // is terminated by single slash "/", a question mark "?", or
      // the end of the url
      index = mSchemeSpecificPart.find("//");
      if(index == 0 && mSchemeSpecificPart.length() > 2)
      {
         string::size_type slash = mSchemeSpecificPart.find('/', 2);
         string::size_type qMark = mSchemeSpecificPart.find('?', 2);
         
         // see if a query exists
         if(qMark != string::npos)
         {
            // a query exists
            
            // get the path
            if(slash != string::npos && slash < qMark)
            {
               // get authority
               mAuthority = mSchemeSpecificPart.substr(2, slash - 2);
               
               // get path
               mPath = mSchemeSpecificPart.substr(slash, qMark - slash);
            }
            else
            {
               // get authority
               mAuthority = mSchemeSpecificPart.substr(2, qMark - 2);
               
               // use base path
               mPath = "/";
            }
            
            // get query
            if(qMark != mSchemeSpecificPart.length() - 1) 
            {
               mQuery = mSchemeSpecificPart.substr(qMark + 1);
            }
         }
         else if(slash != string::npos)
         {
            // no query -- just authority and path
            
            // get authority
            mAuthority = mSchemeSpecificPart.substr(2, slash - 2);
            
            // get path
            mPath = mSchemeSpecificPart.substr(slash);
         }
         else
         {
            // no path or query, just authority
            
            // get authority
            mAuthority = mSchemeSpecificPart.substr(2);
         }
      }
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

const string& Url::getAuthority()
{
   return mAuthority;
}

const string& Url::getPath()
{
   return mPath;
}

const string& Url::getQuery()
{
   return mQuery;
}

void Url::toString(string& str)
{
   str.append(mScheme + ":" + mSchemeSpecificPart);
}

unsigned int Url::getDefaultPort()
{
   unsigned int rval = 0;
   
   if(getScheme() == "http")
   {
      rval = 80;
   }
   else if(getScheme() == "https")
   {
      rval = 443;
   }
   else if(getScheme() == "ftp")
   {
      rval = 21;
   }
   else if(getScheme() == "sftp" || getScheme() == "ssh")
   {
      rval = 22;
   }
   else if(getScheme() == "telnet")
   {
      rval = 23;
   }
   else if(getScheme() == "smtp")
   {
      rval = 25;
   }
   
   return rval;
}

string Url::encode(const char* str, unsigned int length)
{
   string rval;
   
   char c;
   for(unsigned int i = 0; i < length; i++)
   {
      c = str[i];
      
      // see if the character is "safe" (0-9, A-Z, or a-z)
      if((unsigned int)(c - '0') < 10u)
      {
         rval += c;
      }
      else if((unsigned int)(c - 'A') < 26u)
      {
         rval += c;
      }
      else if((unsigned int)(c - 'a') < 26u)
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
      if((unsigned int)(c - '0') < 10u)
      {
         rval += c;
      }
      else if((unsigned int)(c - 'A') < 26u)
      {
         rval += c;
      }
      else if((unsigned int)(c - 'a') < 26u)
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
