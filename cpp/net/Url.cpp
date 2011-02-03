/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/net/Url.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/rt/Exception.h"
#include "monarch/util/Convert.h"
#include "monarch/util/StringTokenizer.h"
#include "monarch/util/StringTools.h"

#include <cstdlib>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <algorithm>

using namespace std;
using namespace monarch::net;
using namespace monarch::rt;
using namespace monarch::util;

Url::Url() :
   mRelative(true),
   mPort(0)
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

   string url;
   rval =
      StringTools::vsformat(url, format, varargs) &&
      setUrl(url);

   if(!rval)
   {
      ExceptionRef e = new Exception(
         "Could not set URL.",
         "monarch.net.Url.FormatError");
      e->getDetails()["format"] = format;
      Exception::push(e);
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

   // initialize
   mPort = 0;

   // find the first colon
   string::size_type index = 0;
   index = url.find(':');

   // if no colon found, assume relative
   mRelative = (index == string::npos);

   // handle scheme for absolute urls only
   if(!mRelative)
   {
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
            "URL scheme contains invalid start character.",
            "monarch.net.MalformedUrl");
         e->getDetails()["url"] = url.c_str();
         e->getDetails()["relative"] = mRelative;
         Exception::set(e);
         rval = false;
      }
      else
      {
         for(string::iterator i = mScheme.begin();
             rval && i != mScheme.end(); ++i)
         {
            // non-start characters must be in [a-z0-9+.-]
            c = *i;
            if(!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
               c == '+' || c == '.' || c == '-'))
            {
               ExceptionRef e = new Exception(
                  "URL scheme contains invalid characters.",
                  "monarch.net.MalformedUrl");
               e->getDetails()["scheme"] = mScheme.c_str();
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

   // parse the scheme specific part if the URL is relative (the entire URL
   // is the scheme specific part) or if the URL is absolute and has a scheme
   // specific part (the colon is not at the end of the url)
   if(rval && (mRelative || (index < url.length() - 1)))
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

   // if port not set, use default port
   if(mPort == 0)
   {
      mPort = getDefaultPort();
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

void Url::setScheme(const char* scheme)
{
   mScheme = scheme;
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

   // initialize result to an array
   result->setType(Array);

   // start tokenizing after the base path
   const char* start = strstr(mPath.c_str(), basePath);
   if(start != NULL)
   {
      rval = true;

      // there are no tokens if the URL path is the same as the base path
      start += strlen(basePath);
      if(start[0] != '\0')
      {
         // split path up by forward slashes
         const char* tok;
         StringTokenizer st(start, '/');
         for(int i = 0; st.hasNextToken(); ++i)
         {
            tok = st.nextToken();

            // url-decode token, set dynamic object value and type
            result[i] = decode(tok, strlen(tok)).c_str();
         }
      }
   }

   return rval;
}

const string& Url::getQuery()
{
   return mQuery;
}

void Url::addQueryVariable(const char* key, const char* value)
{
   string kv;
   kv.append(encode(key));
   kv.push_back('=');
   kv.append(encode(value));

   // update scheme specific part and query
   if(mQuery.length() == 0)
   {
      mSchemeSpecificPart.push_back('?');
   }
   else
   {
      mSchemeSpecificPart.push_back('&');
      mQuery.push_back('&');
   }
   mSchemeSpecificPart.append(kv);
   mQuery.append(kv);
}

void Url::addQueryVariables(DynamicObject& vars)
{
   if(vars->getType() == Map && vars->length() > 0)
   {
      DynamicObjectIterator i = vars.getIterator();
      while(i->hasNext())
      {
         DynamicObject& next = i->next();

         if(next->getType() == Array)
         {
            // multiple values
            DynamicObjectIterator ni = next.getIterator();
            while(ni->hasNext())
            {
               DynamicObject& value = ni->next();
               addQueryVariable(i->getName(), value->getString());
            }
         }
         else
         {
            // single value
            addQueryVariable(i->getName(), next->getString());
         }
      }
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

int Url::getPort()
{
   // set default port
   if(mPort == 0)
   {
      mPort = getDefaultPort();
   }
   return mPort;
}

string Url::getHostAndPort()
{
   string rval = mHost;
   char tmp[10];
   snprintf(tmp, 10, "%u", mPort);
   rval.push_back(':');
   rval.append(tmp);
   return rval;
}

string Url::getSchemeHostAndPort()
{
   string rval = mScheme;
   rval.append("://");
   rval.append(getHostAndPort());
   return rval;
}

int Url::getDefaultPort()
{
   int rval = 0;

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

void Url::normalize()
{
   // FIXME generalize and handle other schemes
   if(strcmp(getScheme().c_str(), "http") == 0 ||
      strcmp(getScheme().c_str(), "https") == 0)
   {
      bool success = true;

      // save old url to restore on error
      string oldUrl = toString();

      // lowercase host, scheme is already lowercased
      string nHost = mHost;
      transform(nHost.begin(), nHost.end(), nHost.begin(), ::tolower);

      // setup port if not the default port
      string port;
      if(getPort() != getDefaultPort())
      {
         success = StringTools::sformat(port, ":%d", getPort());
      }

      // FIXME:
      // uppercase % encoded entities (ie %3a => %3A)
      // normalize path segments "." and ".."
      // normalize special chars "/", "+", etc to % encoded

      if(success)
      {
         // reformat with normalized data
         success = format("%s://%s%s%s%s",
            getScheme().c_str(),
            nHost.c_str(),
            port.c_str(),
            getPath().c_str(),
            getQuery().c_str());
      }

      if(!success)
      {
         // clear and restore
         Exception::clear();
         setUrl(oldUrl);
      }
   }
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

string Url::getParentPath(const char* path)
{
   string rval = path;

   // erase sub-path from the end of path
   path = rval.c_str();
   int len = rval.length();
   const char* end = strrchr(path, '/');

   // if end is at the end of path, we need to get a new end in order
   // to get to the parent (ie: input: "/foo/bar/", parent we want: "/foo")
   if(end != NULL && end[1] == 0)
   {
      rval.erase(--len);
      path = rval.c_str();
      end = strrchr(path, '/');
   }

   if(end == NULL || end == path)
   {
      // return root path if not found or at beginning of path
      rval = "/";
   }
   else
   {
      // erase starting at end
      rval.erase(end - path);
   }

   return rval;
}

void Url::parseHostAndPort(const char* input, string& host, string& port)
{
   const char* colon = strchr(input, ':');
   if(colon != NULL)
   {
      host.assign(input, (colon - input));
      port.assign(colon + 1);
   }
   else
   {
      host = input;
      port.clear();
   }
}

string Url::encode(const char* str, unsigned int length, bool spaceToPlus)
{
   string rval;

   char c;
   for(unsigned int i = 0; i < length; ++i)
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
      else if(c == ' ' && spaceToPlus)
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

string Url::encode(const char* str, bool spaceToPlus)
{
   return Url::encode(str, strlen(str), spaceToPlus);
}

string Url::decode(const char* str, unsigned int length)
{
   string rval;

   char c;
   unsigned int cLength;
   for(unsigned int i = 0; i < length; ++i)
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
      if(next->getType() == Array)
      {
         // handle array of values
         DynamicObjectIterator ai = next.getIterator();
         while(ai->hasNext())
         {
            DynamicObject& item = ai->next();
            rval.append(encode(i->getName(), strlen(i->getName())));
            rval.push_back('=');
            rval.append(encode(item->getString(), strlen(item->getString())));
         }
      }
      else
      {
         // handle single value
         rval.append(encode(i->getName(), strlen(i->getName())));
         rval.push_back('=');
         rval.append(encode(next->getString(), strlen(next->getString())));
      }
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
      size_t namelen = (eq != NULL) ? (eq - tok) : strlen(tok);
      if(namelen > 0)
      {
         // valid var found
         rval = true;

         // get and url-decode name and value
         string keyStr = decode(tok, namelen);
         const char* key = keyStr.c_str();
         DynamicObject value;
         if(eq != NULL)
         {
            value = decode(eq + 1, strlen(eq + 1)).c_str();
         }

         // always add value to an array
         if(asArrays)
         {
            form[key]->append(value);
         }
         else
         {
            // check if form already has key
            bool exists = form->hasMember(key);
            DynamicObject& v = form[key];

            // add value to an array if there are duplicates
            if(exists)
            {
               // convert existing value to an array
               if(v->getType() != Array)
               {
                  DynamicObject tmp;
                  tmp = v->getString();
                  v->append(tmp);
               }
               v->append(value);
            }
            // set value to string
            else
            {
               v = value;
            }
         }
      }
   }

   return rval;
}
