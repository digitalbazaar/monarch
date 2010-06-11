/*
 * Copyright (c) 2008-2010 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/http/CookieJar.h"

#include "monarch/util/StringTokenizer.h"

using namespace std;
using namespace monarch::http;
using namespace monarch::rt;
using namespace monarch::util;

CookieJar::CookieJar()
{
   mCookies->setType(Map);
}

CookieJar::~CookieJar()
{
}

void CookieJar::readCookies(HttpHeader* header, CookieOrigin origin)
{
   // parse cookies if appropriate cookie field exists
   const char* field = (origin == Server ? "Set-Cookie" : "Cookie");
   int count = header->getFieldCount(field);
   if(count > 0)
   {
      // Set-Cookie: cookie1_name=cookie1_value; max-age=0; path=/
      // Set-Cookie: c2=v2; expires=Thu, 21-Aug-2008 23:47:25 GMT; path=/
      // Cookie: cookie1_name=cookie1_value; cookie2_name=cookie2_value

      // parse cookies by semi-colons (cannot parse by commas because
      // the "expires" value may contain a comma and no one follows the
      // standard)
      string cookies;
      Cookie cookie(NULL);
      TimeZone gmt = TimeZone::getTimeZone("GMT");
      Date d;
      Date now;
      bool name;
      StringTokenizer pairs;
      for(int i = 0; i < count; ++i)
      {
         name = true;
         header->getField(field, cookies, i);
         pairs.tokenize(cookies.c_str(), ';');
         while(pairs.hasNextToken())
         {
            // get next token (name=value)
            const char* token = pairs.nextToken();

            // get name part of token
            size_t nameLength = strcspn(token, "=");
            char tmpName[nameLength + 1];
            strncpy(tmpName, token, nameLength);
            tmpName[nameLength] = 0;

            // trim whitespace from name
            char* namePtr = (tmpName + nameLength);
            for(; namePtr != tmpName && *namePtr == ' '; --namePtr)
            {
               *namePtr = 0;
            }
            for(namePtr = tmpName; *namePtr != 0 && *namePtr == ' ';
                ++namePtr);

            // get value part of token (ensure value length is at least 0 in
            // case bad parsing because of no equals sign after name)
            size_t valueLength = strlen(token);
            valueLength = (nameLength == valueLength ?
               0 : valueLength - nameLength - 1);
            char tmpValue[valueLength + 1];
            if(valueLength > 0)
            {
               strncpy(tmpValue, token + nameLength + 1, valueLength);
            }
            tmpValue[valueLength] = 0;

            if(origin == Client)
            {
               // set cookie
               setCookie(namePtr, tmpValue, 0, false, false);
            }
            else
            {
               if(name)
               {
                  // first token *must* be cookie name = cookie value
                  cookie = Cookie();
                  cookie["name"] = namePtr;
                  cookie["value"] = tmpValue;
                  name = false;
               }
               else
               {
                  if(strcmp(namePtr, "expires") == 0)
                  {
                     // parse expiration time
                     if(d.parse(tmpValue, HttpHeader::sDateFormat, &gmt))
                     {
                        int64_t age = d.getSeconds() - now.getSeconds();
                        cookie["maxAge"] = (age <= 0 ? 0 : age);
                     }
                     else
                     {
                        // bad date format, use maxAge of 0
                        cookie["maxAge"] = 0;
                     }
                  }
                  else if(strcasecmp(namePtr, "secure") == 0)
                  {
                     // cookie is secure
                     cookie["secure"] = true;
                  }
                  else if(strcasecmp(namePtr, "HttpOnly") == 0)
                  {
                     // cookie is http-only (non-javascript)
                     cookie["httpOnly"] = true;
                  }
                  else
                  {
                     // not first token or expires, set other cookie parameter
                     cookie[namePtr] = tmpValue;
                  }
               }

               // add cookie if last token
               if(!pairs.hasNextToken())
               {
                  // cookie not secure if no secure value found
                  if(!cookie->hasMember("secure"))
                  {
                     cookie["secure"] = false;
                  }

                  // cookie is not http-only if no HttpOnly value found
                  if(!cookie->hasMember("httpOnly"))
                  {
                     cookie["httpOnly"] = false;
                  }

                  mCookies[cookie["name"]->getString()] = cookie;
                  cookie.setNull();
               }
            }
         }
      }
   }
}

bool CookieJar::writeCookies(
   HttpHeader* header, CookieOrigin origin, bool overwrite)
{
   bool rval = true;

   // get header field name
   const char* field = (origin == Server ? "Set-Cookie" : "Cookie");

   // if overwriting, remove existing field
   if(overwrite)
   {
      header->removeField(field);
   }

   // if cookies exist, convert all cookies to string
   if(mCookies->length() > 0)
   {
      TimeZone gmt = TimeZone::getTimeZone("GMT");
      Date d;
      int age;
      string str;
      CookieIterator i = mCookies.getIterator();
      while(i->hasNext())
      {
         Cookie& cookie = i->next();

         // only write out cookie if not expired or if origin is Server
         age = cookie["maxAge"]->getInt32();
         if(origin == Server || age != 0)
         {
            if(origin == Client)
            {
               // append semi-colon and space if not first cookie
               if(str.length() > 0)
               {
                  str.append("; ");
               }
            }

            // output cookie name and value
            str.append(cookie["name"]->getString());
            str.push_back('=');
            str.append(cookie["value"]->getString());

            // only send out other cookie data if writing out Set-Cookie
            if(origin == Server)
            {
               // output comment if appropriate
               if(cookie->hasMember("comment"))
               {
                  str.append("; comment=");
                  str.append(cookie["comment"]->getString());
               }

               // output "expires" value if appropriate
               if(age > 0)
               {
                  str.append("; expires=");
                  d.addSeconds(age);
                  str.append(d.toString(HttpHeader::sDateFormat, &gmt));
                  d.addSeconds(-age);
               }
               else if(age == 0)
               {
                  // expire cookie
                  str.append("; max-age=0");
               }

               // output path
               str.append("; path=");
               str.append(cookie["path"]->getString());

               // output secure if appropriate
               if(cookie["secure"]->getBoolean())
               {
                  str.append("; secure");
               }

               // output http only if appropriate
               if(cookie["httpOnly"]->getBoolean())
               {
                  str.append("; HttpOnly");
               }

               // output domain if appropriate
               if(cookie->hasMember("domain"))
               {
                  str.append("; domain=");
                  str.append(cookie["domain"]->getString());
               }

               // output version if appropriate
               if(cookie->hasMember("version"))
               {
                  str.append("; version=");
                  str.append(cookie["version"]->getString());
               }

               // add header field and clear string
               header->addField(field, str.c_str());
               str.erase();
            }
         }
      }

      if(origin == Client)
      {
         // add single header field
         header->addField(field, str.c_str());
      }
   }

   return rval;
}

void CookieJar::setCookie(Cookie& cookie)
{
   mCookies[cookie["name"]->getString()] = cookie;
}

void CookieJar::setCookie(
   const char* name, const char* value, int maxAge, bool secure, bool httpOnly,
   const char* path, const char* domain, int version)
{
   Cookie cookie;
   cookie["name"] = name;
   cookie["value"] = value;
   cookie["maxAge"] = maxAge;
   cookie["path"] = path;
   cookie["secure"] = secure;
   cookie["httpOnly"] = httpOnly;

   if(domain != NULL)
   {
      cookie["domain"] = domain;
   }

   if(version != 0)
   {
      cookie["version"] = version;
   }

   setCookie(cookie);
}

Cookie CookieJar::getCookie(const char* name)
{
   Cookie rval(NULL);

   if(mCookies->hasMember(name))
   {
      rval = mCookies[name];
   }

   return rval;
}

void CookieJar::deleteCookie(const char* name, bool secure)
{
   Cookie cookie = getCookie(name);
   if(cookie.isNull())
   {
      setCookie(name, "", 0, secure, false);
   }
   else
   {
      cookie["value"] = "";
      cookie["maxAge"] = 0;
      cookie["httpOnly"] = true;
   }
}

bool CookieJar::removeCookie(const char* name)
{
   bool rval = mCookies->hasMember(name);
   mCookies->removeMember(name);
   return rval;
}

void CookieJar::clearCookies()
{
   mCookies->clear();
}
