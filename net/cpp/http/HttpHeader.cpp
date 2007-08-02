/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "HttpHeader.h"
#include "StringTools.h"
#include "Convert.h"

using namespace std;
using namespace db::net::http;
using namespace db::util;

// define CRLF
const char HttpHeader::CRLF[] = "\r\n";

bool StringComparator::operator()(const char* s1, const char* s2) const
{
   bool rval = false;
   
   int count1 = 0;
   int count2 = 0;
   char ch1 = *s1;
   char ch2 = *s2;
   bool equal = true;
   for(; equal && ch1 != 0 && ch2 != 0; count1++, count2++)
   {
      ch1 = *(s1 + count1);
      ch2 = *(s2 + count2);
      
      if(ch1 != ch2)
      {
         // do case-insensitive compare, convert both chars to upper-case
         // 97 = 'a', 122 = 'z'
         // 65 = 'A', 90 = 'Z'
         if(ch1 > 96 && ch1 < 123)
         {
            ch1 -= 32;
         }
         
         if(ch2 > 96 && ch2 < 123)
         {
            ch2 -= 32;
         }
         
         if(ch1 < ch2)
         {
            rval = true;
            equal = false;
         }
         else if(ch1 > ch2)
         {
            equal = false;
         }
      }
   }
   
   if(equal && count1 < count2)
   {
      // string one is shorter than string two, so it is less
      rval = true;
   }
   
   return rval;
}

HttpHeader::HttpHeader()
{
}

HttpHeader::~HttpHeader()
{
   for(map<const char*, string, StringComparator>::iterator i =
       mHeaders.begin(); i != mHeaders.end(); i++)
   {
      delete [] i->first;
   }
}

void HttpHeader::setHeader(const char* header, long long value)
{
   setHeader(header, Convert::integerToString(value));
}

void HttpHeader::setHeader(const char* header, const std::string& value)
{
   // clear old header
   map<const char*, std::string, StringComparator>::iterator i =
      mHeaders.find(header);
   if(i != mHeaders.end())
   {
      delete [] i->first;
      mHeaders.erase(i);
   }
   
   // set new header
   char* str = new char[strlen(header) + 1];
   strcpy(str, header);
   mHeaders.insert(make_pair(str, value));
}

void HttpHeader::addHeader(const char* header, const std::string& value)
{
   // get existing value
   string existing;
   getHeader(header, existing);
   
   // append new value
   existing.append(", ");
   existing.append(value);
   setHeader(header, existing);
}

void HttpHeader::removeHeader(const char* header)
{
   // erase header
   mHeaders.erase(header);
}

void HttpHeader::clearHeaders()
{
   mHeaders.clear();
}

bool HttpHeader::getHeader(const char* header, long long& value)
{
   bool rval = false;
   
   string str;
   if(getHeader(header, str))
   {
      rval = Convert::stringToInteger(str.c_str(), value);
   }
   
   return rval;
}

bool HttpHeader::getHeader(const char* header, string& value)
{
   bool rval = false;
   
   // find header entry
   map<const char*, string, StringComparator>::iterator i =
      mHeaders.find(header);
   if(i != mHeaders.end())
   {
      // get value
      value = i->second;
      rval = true;
   }
   
   return rval;
}

bool HttpHeader::parse(const string& str)
{
   bool rval = false;
   
   // clear headers
   clearHeaders();
   
   bool startLine = true;
   const char* start = str.c_str();
   char* cr;
   char* colon;
   while((cr = strchr(start, '\r')) != NULL)
   {
      // look for CRLF
      if(strchr(cr, '\n') == cr + 1)
      {
         if(startLine)
         {
            rval = parseStartLine(start, cr - start);
            startLine = false;
         }
         else
         {
            // found a CRLF, now find colon
            if((colon = strchr(start, ':')) != NULL && colon < cr)
            {
               // get field name
               char* name = new char[colon - start + 1];
               strncpy(name, start, colon - start);
               memset(name + (colon - start), 0, 1);
               
               // skip whitespace
               colon++;
               for(; *colon == ' ' && colon < cr; colon++);
               
               // get field value
               char value[cr - colon + 1];
               strncpy(value, colon, cr - colon);
               memset(value + (cr - colon), 0, 1);
               
               // clear old header
               map<const char*, std::string, StringComparator>::iterator i =
                  mHeaders.find(name);
               if(i != mHeaders.end())
               {
                  delete [] i->first;
                  mHeaders.erase(i);
               }
               
               // set header
               mHeaders.insert(make_pair(name, value));
            }
         }
         
         start = cr + 2;
      }
      else
      {
         start = cr + 1;
      }
   }
   
   return rval;
}

string& HttpHeader::toString(string& str)
{
   str.erase();
   
   // append the start line and CRLF
   getStartLine(str);
   str.append(CRLF);
   
   // append all headers
   for(map<const char*, string, StringComparator>::iterator i =
       mHeaders.begin(); i != mHeaders.end(); i++)
   {
      // get field name and bicapitalize it
      char name[strlen(i->first)];
      strcpy(name, i->first);
      biCapitalize(name);
      
      // append name and value
      str.append(name);
      str.append(": ");
      str.append(i->second);
      str.append(CRLF);
   }
   
   // add CRLF
   str.append(CRLF);
   
   return str;
}

void HttpHeader::setDate(Date* date)
{
   // get GMT time zone
   TimeZone gmt = TimeZone::getTimeZone("GMT");
   string str;
   string format = "%a, %d %b %Y %H:%M:%S GMT";
   
   if(date == NULL)
   {
      // get current date
      Date now;
      now.format(str, format, "c", &gmt);
   }
   else
   {
      date->format(str, format, "c", &gmt);
   }
   
   // set date header
   setHeader("Date", str);
}

bool HttpHeader::getDate(Date& date)
{
   bool rval = false;
   
   string str;
   if(getHeader("Date", str))
   {
      // get GMT time zone
      TimeZone gmt = TimeZone::getTimeZone("GMT");
      rval = date.parse(str, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   }
   
   return rval;
}

void HttpHeader::biCapitalize(char* header)
{
   // 97 = 'a', 122 = 'z'
   if(*header > 96 && *header < 123)
   {
      // capitalize letter
      *header -= 32;
   }
   
   // capitalize letters after hyphens, decapitalize other letters
   header++;
   for(; *header != 0; header++)
   {
      if(*(header - 1) == '-')
      {
         if(*header > 96 && *header < 123)
         {
            // capitalize
            *header -= 32;
         }
      }
      else
      {
         // 65 = 'A', 90 = 'Z'
         if(*header > 64 && *header < 90)
         {
            // decapitalize
            *header += 32;
         }
      }
   }
}
