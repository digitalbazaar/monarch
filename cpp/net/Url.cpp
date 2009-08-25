/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/net/Url.h"

#include "db/rt/DynamicObjectIterator.h"
#include "db/rt/Exception.h"
#include "db/util/Convert.h"
#include "db/util/StringTokenizer.h"

#include <cstdlib>
#include <cctype>
#include <cstdarg>
#include <algorithm>

using namespace std;
using namespace db::net;
using namespace db::rt;
using namespace db::util;

Url::Url()
{
}

Url::Url(const char* url)
{
   setUrl(url);
}

Url::Url(const string& url)
{
   setUrl(url);
}

Url::Url(const Url& copy)
{
   *this = copy;
}

Url::~Url()
{
}

bool Url::setUrl(const char* format, va_list varargs)
{
   bool rval;

   // Note: this code is adapted from the glibc sprintf documentation

   // estimate 256 bytes to start with
   int n, size = 256;
   char *p;
   char *np;

   bool mallocFailed = ((p = (char*)malloc(size)) == NULL);
   bool success = false;
   while(!success && !mallocFailed)
   {
      // try to print in the allocated space
      n = vsnprintf(p, size, format, varargs);

      // if that worked, return the string
      if(n > -1 && n < size)
      {
         success = true;
      }
      else
      {
         // try again with more space
         if(n > -1)
         {
            // glibc 2.1 says (n + 1) is exactly what is needed
            size = n + 1;
         }
         else
         {
            // glibc 2.0 doesn't know the exact size, so guess
            size *= 2;
         }

         if((np = (char*)realloc(p, size)) == NULL)
         {
            // bad malloc
            free(p);
            mallocFailed = true;
         }
         else
         {
            p = np;
         }
      }
   }

   if(success)
   {
      rval = setUrl(p);
      free(p);
   }
   else
   {
      ExceptionRef e = new Exception(
         "Could not set url. Formatted string could not be malloc'd.",
         "db.net.Url.BadMalloc");
      e->getDetails()["format"] = format;
      Exception::set(e);
      rval = false;
   }

   return rval;
}

Url& Url::operator=(const Url& rhs)
{
   setUrl(rhs.toString());
   return *this;
}

bool Url::setUrl(const string& url)
{
   bool rval = true;

   // find the first colon
   string::size_type index = 0;
   index = url.find(':');

   // if no colon found, assume relative
   mRelative = (index == string::npos);

   // handle scheme for absolute urls only
   if(!mRelative)
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
            "Url scheme contains invalid start character.",
            "db.net.MalformedUrl");
         e->getDetails()["url"] = url.c_str();
         e->getDetails()["relative"] = mRelative;
         Exception::set(e);
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
                  "Url scheme contains invalid characters.",
                  "db.net.MalformedUrl");
               e->getDetails()["url"] = url.c_str();
               e->getDetails()["relative"] = mRelative;
               Exception::set(e);
               rval = false;
            }
         }
      }
   }
   else
   {
      // url is relative, start at beginning
      index = 0;
   }

   if(rval && index < url.length() - 1)
   {
      // get scheme specific part
      if(mRelative)
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

   return rval;
}

bool Url::setUrl(const char* url)
{
   return setUrl(string(url));
}

bool Url::format(const char* format, ...)
{
   bool rval;

   va_list varargs;
   va_start(varargs, format);
   rval = setUrl(format, varargs);
   va_end(varargs);

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

void Url::addQueryVariables(DynamicObject& vars)
{
   if(vars->getType() == Map && vars->length() > 0)
   {
      string query;
      DynamicObjectIterator i = vars.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();

         if(query.length() > 0 || mQuery.length() > 0)
         {
            query.push_back('&');
         }
         query.append(encode(i->getName()));
         query.push_back('=');
         query.append(encode(next->getString()));
      }

      // update scheme specific part and query
      if(mQuery.length() == 0)
      {
         mSchemeSpecificPart.push_back('?');
      }
      mSchemeSpecificPart.append(query);
      mQuery.append(query);
   }
}

bool Url::getQueryVariables(DynamicObject& vars, bool asArrays)
{
   // url-form decode query
   return formDecode(vars, mQuery.c_str(), asArrays);
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

string Url::encode(const char* str)
{
   return Url::encode(str, strlen(str));
}

string Url::decode(const char* str, unsigned int length)
{
   string rval;

   char c;
   unsigned int cLength;
   for(unsigned int i = 0; i < length; i++)
   {
      c = str[i];

      // FIXME: optimize with a char[128] LUT
      // see if the character is "safe" (0-9, A-Z, or a-z)
      if((c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z') ||
         (c == '-') ||
         (c == '_') ||
         (c == '.') ||
         (c == '!') ||
         (c == '~') ||
         (c == '*') ||
         (c == '\'') ||
         (c == '(') ||
         (c == ')') ||
         (c == ':') ||
         (c == '/'))
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
      else
      {
         // FIXME: handle other characters
      }
   }

   return rval;
}

string Url::decode(const char* str)
{
   return Url::decode(str, strlen(str));
}

string Url::formEncode(DynamicObject& form)
{
   string rval;

   // ensure type is map
   form->setType(Map);
   DynamicObjectIterator i = form.getIterator();
   while(i->hasNext())
   {
      DynamicObject& next = i->next();

      if(rval.length() > 0)
      {
         rval.push_back('&');
      }

      // url-encode and append form field
      rval.append(encode(i->getName(), strlen(i->getName())));
      rval.push_back('=');
      rval.append(encode(next->getString(), strlen(next->getString())));
   }

   return rval;
}

bool Url::formDecode(DynamicObject& form, const char* str, bool asArrays)
{
   bool rval = false;

   // force form to be a map
   form->setType(Map);

   // split string up by ampersands
   const char* tok;
   const char* eq;
   StringTokenizer st(str, '&');
   while(st.hasNextToken())
   {
      tok = st.nextToken();

      // split on equals
      eq = strchr(tok, '=');
      if(eq != NULL)
      {
         size_t namelen = eq - tok;

         if(namelen > 0)
         {
            // valid var found
            rval = true;

            // get variable name and set value
            char name[namelen];
            memcpy(name, tok, namelen);

            // url-decode name and value
            string key = decode(name, namelen);
            DynamicObject value;
            value = decode(eq + 1, strlen(eq + 1)).c_str();
            // add value for key or add value to key array
            if(asArrays)
            {
               form[key.c_str()]->setType(Array);
               form[key.c_str()]->append(value);
            }
            else
            {
               form[key.c_str()] = value;
            }
         }
      }
      else
      {
         size_t namelen = strlen(tok);

         // ignore empty names
         if(namelen > 0)
         {
            // valid var found
            rval = true;

            // url-decode name and value
            string key = decode(tok, namelen).c_str();
            DynamicObject value;
            value = "";
            if(asArrays)
            {
               form[key.c_str()]->setType(Array);
               form[key.c_str()]->append(value);
            }
            else
            {
               form[key.c_str()] = value;
            }
         }
      }
   }

   return rval;
}
