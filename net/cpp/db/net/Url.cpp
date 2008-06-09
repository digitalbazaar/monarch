/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/Url.h"
#include "db/util/Convert.h"
#include "db/util/StringTokenizer.h"

#include <cstdlib>
#include <cctype>
#include <algorithm>

using namespace std;
using namespace db::net;
using namespace db::rt;
using namespace db::util;

Url::Url(const string& url, bool relative)
{
   setUrl(url, relative);
}

Url::Url(const char* url, bool relative)
{
   setUrl(url, relative);
}

Url::Url(const Url& copy)
{
   *this = copy;
}

Url::~Url()
{
}

Url& Url::operator=(const Url& rhs)
{
   setUrl(rhs.toString(), rhs.mRelative);
   return *this;
}

bool Url::setUrl(const string& url, bool relative)
{
   bool rval = true;
   
   mRelative = relative;
   
   // find the first colon, if not relative
   string::size_type index = 0;
   if(!relative)
   {
      index = url.find(':');
   }
   
   if(!relative && index == string::npos)
   {
      // no colon found
      ExceptionRef e = new Exception(
         "Url is missing a colon!", "db.net.MalformedUrl");
      Exception::setLast(e, false);
      rval = false;
   }
   else
   {
      // handle scheme for absolute urls only
      if(!relative)
      {
         // find double slashes
         index = url.find("//", index);
         if(index == string::npos)
         {
            index = url.rfind(':');
         }
         else
         {
            index--;
         }
         
         // split string into the scheme and scheme-specific-part
         mScheme = url.substr(0, index);
         
         // make scheme lower case
         transform(mScheme.begin(), mScheme.end(), mScheme.begin(), ::tolower);
         
         // check scheme for validity
         // FIXME scheme should be case-insensitive
         char c;
         c = mScheme.c_str()[0];
         if(c < 'a' || c > 'z')
         {
            ExceptionRef e = new Exception(
               "Url scheme contains invalid start character!",
               "db.net.MalformedUrl");
            Exception::setLast(e, false);
            rval = false;
         }
         else
         {
            for(string::iterator i = mScheme.begin();
                rval && i != mScheme.end(); i++)
            {
               // non-start characters must be in [a-z0-9+.-]
               c = *i;
               if(!((c > 'a' && c < 'z') || (c > '0' && c < '9') ||
                  c == '+' || c == '.' || c != '-'))
               {
                  ExceptionRef e = new Exception(
                     "Url scheme contains invalid characters!",
                     "db.net.MalformedUrl");
                  Exception::setLast(e, false);
                  rval = false;
               }
            }
         }
      }
      
      if(rval && index < url.length() - 1)
      {
         // get scheme specific part
         if(relative)
         {
            mSchemeSpecificPart = "//";
            mSchemeSpecificPart.append(url);
         }
         else
         {
            mSchemeSpecificPart = url.substr(index + 1);
         }
         
         // get authority, path, and query:
         
         // authority is preceeded by double slash "//" (current index + 1) and
         // is terminated by single slash "/", a question mark "?", or
         // the end of the url
         if(mSchemeSpecificPart.length() > 2)
         {
            string::size_type slash = mSchemeSpecificPart.find('/', 2);
            string::size_type qMark = mSchemeSpecificPart.find('?', 2);
            
            // see if a query exists
            if(qMark != string::npos)
            {
               // a query exists
               
               // get authority & path
               if(slash != string::npos && slash < qMark)
               {
                  mAuthority = mSchemeSpecificPart.substr(2, slash - 2);
                  mPath = mSchemeSpecificPart.substr(slash, qMark - slash);
               }
               else
               {
                  mAuthority = mSchemeSpecificPart.substr(2, qMark - 2);
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
               mPath = '/';
               
               if(mSchemeSpecificPart[1] == '/')
               {
                  // get authority after slash
                  mAuthority = mSchemeSpecificPart.substr(2);
               }
               else
               {
                  // set authority equal to scheme specific part
                  mAuthority = mSchemeSpecificPart;
               }
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

bool Url::isRelative() const
{
   return mRelative;
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

bool Url::getTokenizedPath(DynamicObject& result, const char* basePath)
{
   bool rval = false;
   
   const char* start = strstr(mPath.c_str(), basePath);
   if(start != NULL)
   {
      rval = true;
      
      // split path up by forward slashes
      const char* tok;
      StringTokenizer st(start + strlen(basePath), '/');
      for(int i = 0; st.hasNextToken(); i++)
      {
         tok = st.nextToken();
         
         // url-decode token, set dynamic object value and type
         result[i] = decode(tok, strlen(tok)).c_str();
      }
   }
   
   return rval;
}

const string& Url::getQuery()
{
   return mQuery;
}

bool Url::getQueryVariables(DynamicObject& vars)
{
   bool rval = false;
   
   if(mQuery.length() > 0)
   {
      rval = true;
      
      // split query up by ampersands
      const char* tok;
      const char* eq;
      StringTokenizer st(mQuery.c_str(), '&');
      while(st.hasNextToken())
      {
         tok = st.nextToken();
         
         // split on equals
         eq = strchr(tok, '=');
         if(eq != NULL)
         {
            // get variable name and set value
            char name[eq - tok];
            memcpy(name, tok, eq - tok);
            
            // url-decode name and value
            vars[decode(name, eq - tok).c_str()] =
               decode(eq + 1, strlen(eq + 1)).c_str();
         }
      }
   }
   
   return rval;
}

string Url::getPathAndQuery()
{
   string str = mPath;
   if(mQuery.length() > 0)
   {
      str.push_back('?');
      str.append(mQuery);
   }
   return str;
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
   else if(strcmp(getScheme().c_str(), "mysql") == 0)
   {
      rval = 3306;
   }
   
   return rval;
}

string Url::toString() const
{
   string str;
   
   if(!isRelative())
   {
      str.append(mScheme);
      str.push_back(':');
      str.append(mSchemeSpecificPart);
   }
   else
   {
      str.append(mSchemeSpecificPart.substr(2));
   }
   
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
         rval.push_back(c);
      }
      else if((unsigned int)(c - 'A') < 26u)
      {
         rval.push_back(c);
      }
      else if((unsigned int)(c - 'a') < 26u)
      {
         rval.push_back(c);
      }
      else if(c == '.' || c == '_' || c == '-')
      {
         rval.push_back(c);
      }
      else if(c == ' ')
      {
         // substitute pluses for spaces
         rval.push_back('+');
      }
      else
      {
         // character is unsafe, so add a '%' and convert to hex
         rval.push_back('%');
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
         rval.push_back(c);
      }
      else if((unsigned int)(c - 'A') < 26u)
      {
         rval.push_back(c);
      }
      else if((unsigned int)(c - 'a') < 26u)
      {
         rval.push_back(c);
      }
      else if(c == '.' || c == '_' || c == '-')
      {
         rval.push_back(c);
      }
      else if(c == '+')
      {
         // substitute spaces for pluses
         rval.push_back(' ');
      }
      else if(c == '%')
      {
         // looking for 3 characters
         if(i + 2 < length)
         {
            Convert::hexToBytes(str + i + 1, 2, &c, cLength);
            rval.push_back(c);
         }
         
         // skip two characters
         i += 2;
      }
   }
   
   return rval;
}
