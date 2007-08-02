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

HttpHeader::HttpHeader()
{
}

HttpHeader::~HttpHeader()
{
}

void HttpHeader::setHeader(const string& header, long long value)
{
   setHeader(header, Convert::integerToString(value));
}

void HttpHeader::setHeader(const string& header, const string& value)
{
   // bicapitalize and trim header
   string bic = header;
   biCapitalize(StringTools::trim(bic));
   
   // trim and set value
   string v = value;
   mHeaders[bic] = StringTools::trim(v);
}

void HttpHeader::addHeader(const string& header, const string& value)
{
   // bicapitalize and trim header
   string bic = header;
   biCapitalize(StringTools::trim(bic));
   
   // get existing value
   string existing = "";
   getHeader(bic, existing);
   
   // trim and append new value
   string v = value;
   setHeader(bic, existing + ", " + StringTools::trim(v));
}

void HttpHeader::removeHeader(const string& header)
{
   // bicapitalize and trim header
   string bic = header;
   biCapitalize(StringTools::trim(bic));
   
   // erase it
   mHeaders.erase(bic);
}

void HttpHeader::clearHeaders()
{
   mHeaders.clear();
}

bool HttpHeader::getHeader(const string& header, long long& value)
{
   bool rval = false;
   
   string str;
   if(getHeader(header, str))
   {
      rval = Convert::stringToInteger(str.c_str(), value);
   }
   
   return rval;
}

bool HttpHeader::getHeader(const string& header, string& value)
{
   bool rval = false;
   
   // bicapitalize and trim header
   string bic = header;
   biCapitalize(StringTools::trim(bic));
   
   // find header entry
   map<string, string>::iterator i = mHeaders.find(bic);
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
            rval = parseStartLine(str.substr(0, cr - start));
            startLine = false;
         }
         else
         {
            // found a CRLF, now find colon
            if((colon = strchr(start, ':')) != NULL && colon < cr)
            {
               setHeader(
                  str.substr(start - str.c_str(), colon - start),
                  str.substr(colon + 1 - str.c_str(), cr - colon));
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
   for(map<string, string>::iterator i = mHeaders.begin();
      i != mHeaders.end(); i++)
   {
      str.append(i->first);
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

void HttpHeader::biCapitalize(string& field)
{
   // capitalize the first letter
   transform(field.begin(), field.begin() + 1, field.begin(), toupper);
   
   // decapitalize all other letters
   transform(field.begin() + 1, field.end(), field.begin() + 1, tolower);
   
   // capitalize all other letters that occur after hyphens
   for(string::iterator i = field.begin() + 1; i != field.end(); i++)
   {
      if(*(i - 1) == '-')
      {
         // capitalize the letter
         transform(i, i + 1, i, toupper);
      }
   }
}
