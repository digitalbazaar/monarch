/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/CookieJar.h"

#include "db/util/StringTokenizer.h"

using namespace std;
using namespace db::net::http;
using namespace db::rt;
using namespace db::util;

CookieJar::CookieJar()
{
   mCookies->setType(Map);
}

CookieJar::~CookieJar()
{
}

void CookieJar::readCookies(HttpHeader* header, CookieOrigin origin)
{
   const char* field = (origin == Server ? "Set-Cookie" : "Cookie");
   
   // get the cookies field
   string cookies;
   if(header->getField(field, cookies))
   {
      if(origin == Client)
      {
         // Cookie: cookie1_name=cookie1_value; cookie2_name=cookie2_value
         
         // parse cookies by semi-colons
         StringTokenizer pairs(cookies.c_str(), ';');
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
            for(; namePtr != tmpName && *namePtr == ' '; namePtr--)
            {
               *namePtr = 0;
            }
            for(namePtr = tmpName; *namePtr != 0 && *namePtr == ' '; namePtr++);
            
            // get value part of token
            size_t valueLength = strlen(token) - nameLength - 1;
            char tmpValue[valueLength + 1];
            strncpy(tmpValue, token + nameLength + 1, valueLength);
            tmpValue[valueLength] = 0;
            
            // determine if value ends in a comma
            if(valueLength > 0 && tmpValue[valueLength - 1] == ',')
            {
               // next token is a new cookie
               tmpValue[valueLength - 1] = 0;
            }
            
            // set cookie
            setCookie(namePtr, tmpValue, 0, false);
         }
      }
      else
      {
         // FIXME: Even this parsing code isn't even complex enough to handle
         // the mush that servers send for cookies. For instance, php sends
         // cookies separated by a *lack* of a semi-colon.
#if 0         
         // parse cookies by semi-colons (cannot parse by commas because
         // the "expires" value may contain a comma and no one follows the
         // standard)
         TimeZone gmt = TimeZone::getTimeZone("GMT");
         Date d;
         Date now;
         Cookie cookie(NULL);
         bool name = true;
         bool next = false;
         StringTokenizer pairs(cookies.c_str(), ';');
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
            for(; namePtr != tmpName && *namePtr == ' '; namePtr--)
            {
               *namePtr = 0;
            }
            for(namePtr = tmpName; *namePtr != 0 && *namePtr == ' '; namePtr++);
            
            // get value part of token
            size_t valueLength = strlen(token) - nameLength - 1;
            char tmpValue[valueLength + 1];
            strncpy(tmpValue, token + nameLength + 1, valueLength);
            tmpValue[valueLength] = 0;
            
            // determine if value ends in a comma
            if(valueLength > 0 && tmpValue[valueLength - 1] == ',')
            {
               // next token is a new cookie
               tmpValue[valueLength - 1] = 0;
               next = true;
            }
            
            if(name)
            {
               // add previous cookie if not NULL
               if(!cookie.isNull())
               {
                  mCookies[cookie["name"]->getString()] = cookie;
               }
               
               // first token *must* be cookie name = cookie value
               cookie = Cookie();
               cookie["name"] = namePtr;
               cookie["value"] = tmpValue;
            }
            else
            {
               if(strcmp(namePtr, "expires") == 0)
               {
                  // parse expiration time
                  if(d.parse(tmpValue, "%a, %d-%b-%Y %H:%M:%S GMT", &gmt))
                  {
                     int age = d.second() - now.second();
                     if(age <= 0)
                     {
                        cookie["maxAge"] = 0;
                     }
                     else
                     {
                        cookie["maxAge"] = age;
                     }
                  }
                  else
                  {
                     // bad date format, use maxAge of 0
                     cookie["maxAge"] = 0;
                  }
               }
               else
               {
                  // not first token or expires, set other cookie parameter
                  cookie[namePtr] = tmpValue;
               }
            }
            
            // set whether or not the next token is a cookie name
            name = next;
            
            // add cookie if last token
            if(!pairs.hasNextToken())
            {
               mCookies[cookie["name"]->getString()] = cookie;
            }
         }
#endif
      }
   }
}

bool CookieJar::writeCookies(
   HttpHeader* header, CookieOrigin origin, bool overwrite)
{
   bool rval = true;
   
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
            if(str.length() > 0)
            {
               // output a comma and space if this is not the first cookie
               if(origin == Server)
               {
                  str.append(", ");
               }
               // output a semi-colon and space if this is not the first cookie
               else
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
                  str.append(cookie["comment"]->getString());
               }
               
               // output "expires" value if appropriate
               if(age > 0)
               {
                  str.append("; expires=");
                  d.addSeconds(age);
                  str.append(d.toString("%a, %d-%b-%Y %H:%M:%S GMT", &gmt));
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
            }
         }
      }
      
      // get header field name
      const char* field = (origin == Server ? "Set-Cookie" : "Cookie");
      
      // set field or add field based on overwrite value
      if(overwrite)
      {
         header->setField(field, str.c_str());
      }
      else if(origin == Server)
      {
         header->addField(field, str.c_str());
      }
      else
      {
         // must append client-sent cookies without commas
         string old;
         header->getField(field, old);
         old.append("; ");
         old.append(str);
         header->setField(field, old.c_str());
      }
   }
   
   return rval;
}

void CookieJar::setCookie(Cookie& cookie)
{
   mCookies[cookie["name"]->getString()] = cookie;
}

void CookieJar::setCookie(
   const char* name, const char* value, int maxAge, bool secure,
   const char* path, const char* domain, int version)
{
   Cookie cookie;
   cookie["name"] = name;
   cookie["value"] = value;
   cookie["maxAge"] = maxAge;
   cookie["path"] = path;
   cookie["secure"] = secure;
   
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
      setCookie(name, "", 0, secure);
   }
   else
   {
      cookie["value"] = "";
      cookie["maxAge"] = 0;
   }
}

bool CookieJar::removeCookie(const char* name)
{
   bool rval = mCookies->hasMember(name);
   mCookies->removeMember(name);
   return rval;
}
