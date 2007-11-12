/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/json/JsonWriter.h"
#include "db/util/DynamicObjectIterator.h"

using namespace std;
using namespace db::data;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;
using namespace db::util;

JsonWriter::JsonWriter()
{
   // Initialize to compact representation
   setCompact(true);
   setIndentation(0, 3);
}

JsonWriter::~JsonWriter()
{
}

bool JsonWriter::writeIndentation(OutputStream* os, int level)
{
   bool rval = true;
   int indent = mCompact ? 0 : (level * mIndentSpaces);
   
   // write out indentation
   if(indent > 0)
   {
      char temp[indent];
      memset(temp, ' ', indent);
      rval = os->write(temp, indent);
   }
   
   return rval;
}

bool JsonWriter::write(DynamicObject dyno, OutputStream* os, int level)
{
   bool rval = true;
   DynamicObjectIterator i;

   if(level < 0)
   {
      level = mIndentLevel;
   }
   
   switch(dyno->getType())
   {
      case String:
         rval = os->write("\"", 1);
         if(rval)
         {
            string encoded;
            const char* temp = dyno->getString();
            for(size_t i=0; i<strlen(temp); i++)
            {
               char c = temp[i];
               if((c >= 0x5d /* && c <= 0x10FFFF */) ||
                  (c >= 0x23 && c <= 0x5B) ||
                  (c == 0x21) ||
                  (c == 0x20))
               {
                  encoded.push_back(c);
               }
               else
               {
                  const char* csub;
                  char ucsub[7];
                  switch(c)
                  {
                     case '"': /* 0x22 */
                        csub = "\\\"";
                        break;
                     case '\\': /* 0x5C */
                        csub = "\\\\";
                        break;
                     // '/' is in the RFC but not required to be escaped
                     //case '/': /* 0x2F */
                     //   csub = "\\\\";
                     //   break;
                     case '\b': /* 0x08 */
                        csub = "\\b";
                        break;
                     case '\f': /* 0x0C */
                        csub = "\\f";
                        break;
                     case '\n': /* 0x0A */
                        csub = "\\n";
                        break;
                     case '\r': /* 0x0D */
                        csub = "\\r";
                        break;
                     case '\t': /* 0x09 */
                        csub = "\\t";
                        break;
                     default:
                        snprintf(ucsub, 6, "\\u%4x", c);
                        csub = ucsub;
                        // FIXME other utf-8/16/32, surrugate pairs, etc
                        break;
                  }
                  encoded.append(csub);
               }
            }
            rval = os->write(encoded.c_str(), encoded.length());
            if(rval)
            {
               rval = os->write("\"", 1);
            }
         }
         break;
      case Boolean:
      case Int32:
      case UInt32:
      case Int64:
      case UInt64:
      case Double:
         {
            string temp;
            const char* ctemp;
            dyno->toString(temp);
            ctemp = temp.c_str();
            rval = os->write(ctemp, strlen(ctemp));
         }
         break;
      case Map:
         i = dyno.getIterator();
         if(mCompact)
         {
            rval = os->write("{", 1);
         }
         else
         {
            rval = os->write("{\n", 2);
         }
         while(rval && i->hasNext())
         {
            rval = writeIndentation(os, level + 1);
            if(rval)
            {
               DynamicObject next = i->next();
               rval = os->write("\"", 1);
               if(rval)
               {
                  rval = os->write(i->getName(), strlen(i->getName()));
                  if(rval)
                  {
                     if(mCompact)
                     {
                        rval = os->write("\":", 2);
                     }
                     else
                     {
                        rval = os->write("\" : ", 4);
                     }
                     if(rval)
                     {
                        rval = write(next, os, level + 1);
                        if(rval && i->hasNext())
                        {
                           rval = os->write(",", 1);
                        }
                        if(rval && !mCompact)
                        {
                           rval = os->write("\n", 1);
                        }
                     }
                  }
               }
            }
         }
         rval = writeIndentation(os, level);
         if(rval)
         {
            rval = os->write("}", 1);
         }
         break;
      case Array:
         i = dyno.getIterator();
         if(mCompact)
         {
            rval = os->write("[", 1);
         }
         else
         {
            rval = os->write("[\n", 2);
         }
         while(rval && i->hasNext())
         {
            rval = writeIndentation(os, level + 1);
            if(rval)
            {
               rval = write(i->next(), os, level + 1);
               if(rval)
               {
                  if(rval && i->hasNext())
                  {
                     rval = os->write(",", 1);
                  }
                  if(rval && !mCompact)
                  {
                     rval = os->write("\n", 1);
                  }
               }
            }
         }
         rval = writeIndentation(os, level);
         if(rval)
         {
            rval = os->write("]", 1);
         }
         break;
   }
   
   return rval;
}

void JsonWriter::setCompact(bool compact)
{
   mCompact = compact;
}

void JsonWriter::setIndentation(int level, int spaces)
{
   mIndentLevel = level;
   mIndentSpaces = spaces;
}
