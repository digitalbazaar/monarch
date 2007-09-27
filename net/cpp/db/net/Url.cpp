/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/Url.h"
#include "db/util/Convert.h"

using namespace std;
using namespace db::net;
using namespace db::rt;
using namespace db::util;

Url::Url(const string& url)
{
   setUrl(url);
}

Url::Url(const char* url)
{
   setUrl(url);
}

Url::~Url()
{
}

Url& Url::operator=(const Url& rhs)
{
   string s;
   setUrl(rhs.toString(s));
   return *this;
}

MalformedUrlException* Url::setUrl(const string& url)
{
   MalformedUrlException* rval = NULL;
   
   // find the first colon
   string::size_type index = url.find(':');
   if(index == string::npos)
   {
      // no colon found
      rval = new MalformedUrlException("Url is missing a colon!");
      Exception::setLast(rval);
   }
   else
   {
      // split string into the scheme and scheme-specific-part
      mScheme = url.substr(0, index);
      
      // make scheme lower case
      transform(mScheme.begin(), mScheme.end(), mScheme.begin(), tolower);
      
      // check scheme for validity
      // FIXME scheme should be case-insensitive
      char c;
      c = mScheme.c_str()[0];
      if((c < 'a') || (c > 'z'))
      {
         rval = new MalformedUrlException(
            "Url scheme contains invalid start character!");
         Exception::setLast(rval);
      }
      else
      {
         for(string::iterator i = mScheme.begin(); i != mScheme.end(); i++)
         {
            // non-start characters must be in [a-z0-9+.-]
            c = *i;
            if(!(((c > 'a') && (c < 'z')) ||
               ((c > '0') && (c < '9')) ||
               (c == '+') || (c == '.') || (c != '-')))
            {
               rval = new MalformedUrlException(
                  "Url scheme contains invalid characters!");
               Exception::setLast(rval);
               break;
            }
         }
      }

      if(rval == NULL && index != url.length() - 1)
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
                  mPath = '/';
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
               
               // get authority, use base path
               mAuthority = mSchemeSpecificPart.substr(2);
               mPath = '/';
            }
         }
      }
      
      if(mAuthority.length() > 0)
      {
         string hostAndPort;

         const char* at = strchr(mAuthority.c_str(), '@');
         if(at != NULL)
         {
            mUserInfo = mAuthority.substr(0, at - mAuthority.c_str());
            hostAndPort = mAuthority.substr(at - mAuthority.c_str() + 1);
         }
         else
         {
            hostAndPort = mAuthority;
         }

         const char* colon = strchr(hostAndPort.c_str(), ':');
         if(colon != NULL)
         {
            mHost = hostAndPort.substr(0, colon - hostAndPort.c_str());
            mPort = strtoll(colon + 1, NULL, 10);
         }
         else
         {
            const char* slash = strchr(hostAndPort.c_str(), '/');
            if(slash == NULL)
            {
               mHost = hostAndPort;
            }
            else
            {
               mHost = hostAndPort.substr(0, slash - hostAndPort.c_str());
            }
            
            // try to get default port
            mPort = getDefaultPort();
         }
      }

      if(mUserInfo.length() > 0)
      {
         const char* colon = strchr(mUserInfo.c_str(), ':');
         if(colon != NULL)
         {
            mUser = mUserInfo.substr(0, colon - mUserInfo.c_str());
            mPassword = mUserInfo.substr(colon - mUserInfo.c_str() + 1);
         }
      }
   }
   
   return rval;
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

const string& Url::getUserInfo()
{
   return mUserInfo;
}
   
const string& Url::getUser()
{
   return mUser;
}
   
const string& Url::getPassword()
{
   return mPassword;
}
   
const string& Url::getPath()
{
   return mPath;
}

const string& Url::getQuery()
{
   return mQuery;
}

const string& Url::getHost()
{
   return mHost;
}

unsigned int Url::getPort()
{
   return mPort;
}

unsigned int Url::getDefaultPort()
{
   unsigned int rval = 0;
   
   if(strcmp(getScheme().c_str(), "http") == 0)
   {
      rval = 80;
   }
   else if(strcmp(getScheme().c_str(), "https") == 0)
   {
      rval = 443;
   }
   else if(strcmp(getScheme().c_str(), "ftp") == 0)
   {
      rval = 21;
   }
   else if(strcmp(getScheme().c_str(),"sftp") == 0 ||
           strcmp(getScheme().c_str(), "ssh") == 0)
   {
      rval = 22;
   }
   else if(strcmp(getScheme().c_str(), "telnet") == 0)
   {
      rval = 23;
   }
   else if(strcmp(getScheme().c_str(), "smtp") == 0)
   {
      rval = 25;
   }
   
   return rval;
}

string& Url::toString(string& str) const
{
   str.append(mScheme);
   str.append(1, ':');
   str.append(mSchemeSpecificPart);
   return str;
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
         rval.append(1, c);
      }
      else if((unsigned int)(c - 'A') < 26u)
      {
         rval.append(1, c);
      }
      else if((unsigned int)(c - 'a') < 26u)
      {
         rval.append(1, c);
      }
      else if(c == ' ')
      {
         // substitute pluses for spaces
         rval.append(1, '+');
      }
      else
      {
         // character is unsafe, so add a '%' and convert to hex
         rval.append(1, '%');
         rval.append(Convert::bytesToUpperHex(&c, 1));
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
         rval.append(1, c);
      }
      else if((unsigned int)(c - 'A') < 26u)
      {
         rval.append(1, c);
      }
      else if((unsigned int)(c - 'a') < 26u)
      {
         rval.append(1, c);
      }
      else if(c == '+')
      {
         // substitute spaces for pluses
         rval.append(1, ' ');
      }
      else if(c == '%')
      {
         // looking for 3 characters
         if(i + 2 < length)
         {
            Convert::hexToBytes(str + i + 1, 2, &c, cLength);
            rval.append(1, c);
         }
         
         // skip two characters
         i += 2;
      }
   }
   
   return rval;
}
