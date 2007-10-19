/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/net/http/HttpHeader.h"
#include "db/util/StringTools.h"
#include "db/util/Convert.h"

using namespace std;
using namespace db::net::http;
using namespace db::util;

// define CRLF
const char* HttpHeader::CRLF = "\r\n";

HttpHeader::HttpHeader()
{
}

HttpHeader::~HttpHeader()
{
   for(map<const char*, string, FieldComparator>::iterator i =
       mFields.begin(); i != mFields.end(); i++)
   {
      delete [] i->first;
   }
}

bool HttpHeader::parseStartLine(const char* str, unsigned int length)
{
   // no start line
   return true;
}

void HttpHeader::getStartLine(std::string& line)
{
   // no start line
   line.erase();
}

void HttpHeader::setField(const char* name, long long value)
{
   setField(name, Convert::integerToString(value));
}

void HttpHeader::setField(const char* name, const std::string& value)
{
   // clear old field
   map<const char*, std::string, FieldComparator>::iterator i =
      mFields.find(name);
   if(i != mFields.end())
   {
      delete [] i->first;
      mFields.erase(i);
   }
   
   // set new field
   mFields.insert(make_pair(strdup(name), value));
}

void HttpHeader::addField(const char* name, const std::string& value)
{
   // get existing value
   string existing;
   getField(name, existing);
   
   // append new value
   existing.append(", ");
   existing.append(value);
   setField(name, existing);
}

void HttpHeader::removeField(const char* name)
{
   // erase field
   mFields.erase(name);
}

void HttpHeader::clearFields()
{
   for(map<const char*, string, FieldComparator>::iterator i =
       mFields.begin(); i != mFields.end(); i++)
   {
      delete [] i->first;
   }
   
   mFields.clear();
}

bool HttpHeader::getField(const char* name, long long& value)
{
   bool rval = false;
   
   string str;
   if(getField(name, str))
   {
      rval = Convert::stringToInteger(str.c_str(), value);
   }
   
   return rval;
}

bool HttpHeader::getField(const char* name, string& value)
{
   bool rval = false;
   
   // find field entry
   map<const char*, string, FieldComparator>::iterator i =
      mFields.find(name);
   if(i != mFields.end())
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
   
   // clear fields
   clearFields();
   
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
               
               // clear old field
               map<const char*, std::string, FieldComparator>::iterator i =
                  mFields.find(name);
               if(i != mFields.end())
               {
                  delete [] i->first;
                  mFields.erase(i);
               }
               
               // set field
               mFields.insert(make_pair(name, value));
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
   
   // append the start line, if one exists, and CRLF
   getStartLine(str);
   if(str.length() > 0)
   {
      str.append(CRLF);
   }
   
   // append all fields
   for(map<const char*, string, FieldComparator>::iterator i =
       mFields.begin(); i != mFields.end(); i++)
   {
      // get field name and bicapitalize it
      char name[strlen(i->first) + 1];
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
   
   // set date field
   setField("Date", str);
}

bool HttpHeader::getDate(Date& date)
{
   bool rval = false;
   
   string str;
   if(getField("Date", str))
   {
      // get GMT time zone
      TimeZone gmt = TimeZone::getTimeZone("GMT");
      rval = date.parse(str, "%a, %d %b %Y %H:%M:%S", "c", &gmt);
   }
   
   return rval;
}

void HttpHeader::biCapitalize(char* name)
{
   // 97 = 'a', 122 = 'z'
   if(*name > 96 && *name < 123)
   {
      // capitalize letter
      *name -= 32;
   }
   
   // capitalize letters after hyphens, decapitalize other letters
   name++;
   for(; *name != 0; name++)
   {
      if(*(name - 1) == '-')
      {
         if(*name > 96 && *name < 123)
         {
            // capitalize
            *name -= 32;
         }
      }
      else
      {
         // 65 = 'A', 90 = 'Z'
         if(*name > 64 && *name < 90)
         {
            // decapitalize
            *name += 32;
         }
      }
   }
}
