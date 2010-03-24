/*
 * Copyright (c) 2007-2010 Digital Bazaar, Inc. All rights reserved.
 */
#define __STDC_FORMAT_MACROS

#include "monarch/http/HttpHeader.h"

#include "monarch/util/StringTools.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>

using namespace std;
using namespace monarch::io;
using namespace monarch::http;
using namespace monarch::util;

// define CRLF
const char* HttpHeader::CRLF = "\r\n";

// define date format
const char* HttpHeader::sDateFormat = "%a, %d %b %Y %H:%M:%S GMT";

HttpHeader::HttpHeader()
{
   mVersion = NULL;
   mFieldsSize = 0;
}

HttpHeader::~HttpHeader()
{
   if(mVersion != NULL)
   {
      free(mVersion);
   }

   HttpHeader::clearFields();
}

bool HttpHeader::parseStartLine(const char* str, unsigned int length)
{
   // no start line
   return true;
}

void HttpHeader::getStartLine(string& line)
{
   // no start line
   line.erase();
}

bool HttpHeader::hasStartLine()
{
   // no start line
   return false;
}

void HttpHeader::setVersion(const char* version)
{
   if(mVersion != NULL)
   {
      free(mVersion);
   }
   mVersion = strdup(version);
}

const char* HttpHeader::getVersion()
{
   if(mVersion == NULL)
   {
      // default to HTTP/1.1
      mVersion = strdup("HTTP/1.1");
   }

   return mVersion;
}

void HttpHeader::setField(const char* name, int64_t value)
{
   char temp[25];
   snprintf(temp, 22, "%" PRIi64, value);
   setField(name, temp);
}

void HttpHeader::setField(const char* name, const string& value)
{
   // update old field if possible
   FieldMap::iterator start = mFields.find(name);
   if(start != mFields.end())
   {
      // save name
      char* storedName = (char*)start->first;

      // clean up old fields
      FieldMap::iterator end = mFields.upper_bound(storedName);
      for(FieldMap::iterator i = start; i != end; i++)
      {
         mFieldsSize -= i->second.length();
      }
      mFields.erase(start, end);

      // set new field
      mFields.insert(make_pair(storedName, value));
      mFieldsSize += value.length();
   }
   else
   {
      // set new field
      mFieldsSize += strlen(name);
      mFieldsSize += value.length();
      char* fieldName = strdup(name);
      biCapitalize(fieldName);
      mFields.insert(make_pair(fieldName, value));
   }
}

void HttpHeader::addField(const char* name, const string& value)
{
   FieldMap::iterator i = mFields.find(name);
   if(i != mFields.end())
   {
      // add a new field
      mFieldsSize += strlen(name);
      mFieldsSize += value.length();
      mFields.insert(make_pair(i->first, value));
   }
   else
   {
      // set the field
      setField(name, value);
   }
}

void HttpHeader::appendFieldValue(
   const char* name, const string& value, const char* delimiter)
{
   string old = getFieldValue(name);
   if(old.length() > 0)
   {
      old.append(delimiter);
   }
   old.append(value);
   setField(name, old);
}

void HttpHeader::removeField(const char* name)
{
   // erase field
   FieldMap::iterator start = mFields.find(name);
   if(start != mFields.end())
   {
      // store name for freeing
      char* storedName = (char*)start->first;

      // remove all field entries (may be more than one)
      int length = strlen(name);
      FieldMap::iterator end = mFields.upper_bound(name);
      for(FieldMap::iterator i = start; i != end; i++)
      {
         mFieldsSize -= length;
         mFieldsSize -= i->second.length();
      }

      // erase entire range
      mFields.erase(start, end);

      // free name
      free(storedName);
   }
}

void HttpHeader::clearFields()
{
   // save unique names to free (one char* per field name)
   size_t length = mFields.size();
   char* nameArray[length];
   size_t next = 0;
   for(FieldMap::iterator i = mFields.begin();
       i != mFields.end(); i = mFields.upper_bound(i->first))
   {
      // save the name to be freed
      nameArray[next] = (char*)i->first;
      next++;
   }

   // free names
   for(size_t n = 0; n < next; n++)
   {
      free(nameArray[n]);
   }

   mFieldsSize = 0;
   mFields.clear();
}

int HttpHeader::getFieldCount(const char* name)
{
   return mFields.count(name);
}

int HttpHeader::getFieldCount()
{
   return mFields.size();
}

bool HttpHeader::getField(const char* name, int64_t& value, int index)
{
   bool rval = false;

   string str;
   if(getField(name, str, index))
   {
      char* endptr = NULL;
      value = strtoll(str.c_str(), &endptr, 10);
      rval = endptr != str.c_str() && *endptr == '\0';
   }

   return rval;
}

bool HttpHeader::getField(const char* name, string& value, int index)
{
   bool rval = false;

   // find field entry
   FieldMap::iterator i = mFields.find(name);
   if(i != mFields.end())
   {
      // count to correct value
      FieldMap::iterator end = mFields.upper_bound(i->first);
      for(int n = 0; i != end && n < index; n++, i++);

      if(i != end)
      {
         // get value
         value = i->second;
         rval = true;
      }
   }

   return rval;
}

std::string HttpHeader::getFieldValue(const char* name, int index)
{
   string rval;
   getField(name, rval, index);
   return rval;
}

bool HttpHeader::hasField(const char* name)
{
   bool rval = false;

   // find field entry
   FieldMap::iterator i = mFields.find(name);
   if(i != mFields.end())
   {
      rval = true;
   }

   return rval;
}

bool HttpHeader::parse(const string& str)
{
   bool rval = false;

   // clear fields
   clearFields();

   bool startLine = hasStartLine();
   const char* start = str.c_str();
   const char* cr;
   const char* colon;
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
               char name[(colon - start) + 1];
               memcpy(name, start, colon - start);
               name[colon - start] = 0;

               // skip whitespace
               colon++;
               for(; *colon == ' ' && colon < cr; colon++);

               // get field value
               char value[cr - colon + 1];
               strncpy(value, colon, cr - colon);
               value[(cr - colon)] = 0;

               // add field
               addField(name, value);
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

string HttpHeader::toString()
{
   string str;
   str.reserve(512);

   // get the start line
   getStartLine(str);

   // determine total fields size:
   // (CRLF + fields size + fields * (": " + CRLF))
   char fields[2 + mFieldsSize + mFields.size() * 4];
   char* s = fields;

   // append CRLF if there is a start line
   if(str.length() > 0)
   {
      s[0] = '\r';
      s[1] = '\n';
      s += 2;
   }

   // append all fields
   int length;
   for(FieldMap::iterator i = mFields.begin(); i != mFields.end(); i++)
   {
      // get field name length
      length = strlen(i->first);

      // append name
      memcpy(s, i->first, length);
      s += length;

      // append delimiter
      s[0] = ':';
      s[1] = ' ';
      s += 2;

      // append value
      memcpy(s, i->second.c_str(), i->second.length());
      s += i->second.length();

      // append CRLF
      s[0] = '\r';
      s[1] = '\n';
      s += 2;
   }

   // add CRLF
   s[0] = '\r';
   s[1] = '\n';
   s += 2;

   // append fields
   str.append(fields, s - fields);

   return str;
}

bool HttpHeader::write(OutputStream* os)
{
   bool rval = true;

   // get the start line
   string line;
   getStartLine(line);

   // write the start line and CRLF if one exists
   if(line.length() > 0)
   {
      rval =
         os->write(line.c_str(), line.length()) &&
         os->write(CRLF, 2);
   }

   // write all fields
   for(FieldMap::iterator i = mFields.begin(); rval && i != mFields.end(); i++)
   {
      // write name, delimiter, value, and CRLF
      rval =
         os->write(i->first, strlen(i->first)) &&
         os->write(": ", 2) &&
         os->write(i->second.c_str(), i->second.length()) &&
         os->write(CRLF, 2);
   }

   // write ending CRLF
   rval = rval && os->write(CRLF, 2);

   return rval;
}

void HttpHeader::setDate(Date* date)
{
   // get GMT time zone
   TimeZone gmt = TimeZone::getTimeZone("GMT");
   string str;

   if(date == NULL)
   {
      // get current date
      Date now;
      now.format(str, sDateFormat, &gmt);
   }
   else
   {
      date->format(str, sDateFormat, &gmt);
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
      rval = date.parse(str.c_str(), "%a, %d %b %Y %H:%M:%S", &gmt);
   }

   return rval;
}

bool HttpHeader::hasContent()
{
   int64_t length = 0;
   getField("Content-Length", length);
   return (length != 0 || hasField("Transfer-Encoding"));
}

void HttpHeader::writeTo(HttpHeader* header)
{
   // set version
   header->setVersion(getVersion());

   // add all fields
   for(FieldMap::iterator i = mFields.begin(); i != mFields.end(); i++)
   {
      header->addField(i->first, i->second);
   }
}

inline HttpHeader::Type HttpHeader::getType()
{
   return HttpHeader::Header;
}

void HttpHeader::biCapitalize(char* name)
{
   // capitalize first letter and letters after hyphens
   // decapitalize other letters
   // NOTE: hardcoded version is faster than using toupper/tolower
   int length = 0;
   if(name != NULL && *name != '\0')
   {
      length++;
      char* ptr = name;
      // cap first
      if(*ptr >= 'a' && *ptr <= 'z')
      {
         *ptr -= 'a' - 'A';
      }
      for(ptr++; *ptr != '\0'; ptr++, length++)
      {
         // cap after '-'
         if(*(ptr - 1) == '-')
         {
            if(*ptr >= 'a' && *ptr <= 'z')
            {
               *ptr -= 'a' - 'A';
            }
         }
         // decap rest
         else if(*ptr >= 'A' && *ptr <= 'Z')
         {
            *ptr += 'a' - 'A';
         }
      }

      // special case TE header
      if(length == 2 && name[0] == 'T' && name[1] == 'e')
      {
         name[1] = 'E';
      }
   }
}
