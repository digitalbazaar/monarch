/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "db/data/json/JsonWriter.h"

#include "db/rt/DynamicObjectIterator.h"
#include "db/rt/Exception.h"
#include "db/io/BufferedOutputStream.h"
#include "db/io/OStreamOutputStream.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace db::data;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;

JsonWriter::JsonWriter(bool strict)
{
   mStrict = strict;
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

bool JsonWriter::write(DynamicObject& dyno, OutputStream* os, int level)
{
   bool rval = true;

   if(level < 0)
   {
      level = mIndentLevel;
   }

   if(dyno.isNull())
   {
      rval = os->write("null", 4);
   }
   else
   {
      switch(dyno->getType())
      {
         case String:
         {
            const char* temp = dyno->getString();
            size_t length = strlen(temp);
            
            // Note: UTF-8 has a maximum of 6-bytes per character when
            // encoded in json format ("/u1234")
            string encoded;
            encoded.reserve(length);
            encoded.push_back('"');
            char unicode[6];
            for(size_t i = 0; i < length; i++)
            {
               unsigned char c = temp[i];
               if((c >= 0x5d /* && c <= 0x10FFFF */) ||
                  (c >= 0x23 && c <= 0x5B) ||
                  (c == 0x21) ||
                  (c == 0x20))
               {
                  // TODO: check this handles UTF-* properly
                  encoded.push_back(c);
               }
               else
               {
                  encoded.push_back('\\');
                  switch(c)
                  {
                     case '"': /* 0x22 */
                     case '\\': /* 0x5C */
                     // '/' is in the RFC but not required to be escaped
                     //case '/': /* 0x2F */
                        encoded.push_back(c);
                        break;
                     case '\b': /* 0x08 */
                        encoded.push_back('b');
                        break;
                     case '\f': /* 0x0C */
                        encoded.push_back('f');
                        break;
                     case '\n': /* 0x0A */
                        encoded.push_back('n');
                        break;
                     case '\r': /* 0x0D */
                        encoded.push_back('r');
                        break;
                     case '\t': /* 0x09 */
                        encoded.push_back('t');
                        break;
                     default:
                        // produces "u01af" (4 digits of 0-filled hex)
                        snprintf(unicode, 6, "u%04x", c);
                        encoded.append(unicode, 5);
                        break;
                  }
               }
            }
            
            // end string serialization and write encoded string
            encoded.push_back('"');
            rval = os->write(encoded.c_str(), encoded.length());
            break;
         }
         case Boolean:
         case Int32:
         case UInt32:
         case Int64:
         case UInt64:
         case Double:
         {
            const char* temp = dyno->getString();
            rval = os->write(temp, strlen(temp));
            break;
         }
         case Map:
         {
            // start map serialization
            rval = (mCompact) ? os->write("{", 1) : os->write("{\n", 2);
            
            // serialize each map member
            DynamicObjectIterator i = dyno.getIterator();
            while(rval && i->hasNext())
            {
               DynamicObject& next = i->next();
               
               // serialize indentation and start serializing member name
               if((rval =
                  writeIndentation(os, level + 1) &&
                  os->write("\"", 1) &&
                  os->write(i->getName(), strlen(i->getName()))))
               {
                  // end serializing member name, serialize member value
                  rval = ((mCompact) ?
                     os->write("\":", 2) : os->write("\" : ", 4)) &&
                     write(next, os, level + 1);
                  
                  // serialize delimiter if appropriate
                  if(rval && i->hasNext())
                  {
                     rval = os->write(",", 1);
                  }
                  
                  // add formatting if appropriate
                  if(rval && !mCompact)
                  {
                     rval = os->write("\n", 1);
                  }
               }
            }
            
            // end map serialization
            rval = writeIndentation(os, level) && os->write("}", 1);
            break;
         }
         case Array:
         {
            // start array serialization
            rval = (mCompact) ? os->write("[", 1) : os->write("[\n", 2);
            
            // serialize each array element
            DynamicObjectIterator i = dyno.getIterator();
            while(rval && i->hasNext())
            {
               // serialize indentation and array value
               rval =
                  writeIndentation(os, level + 1) &&
                  write(i->next(), os, level + 1);
               
               // serialize delimiter if appropriate
               if(rval && i->hasNext())
               {
                  rval = os->write(",", 1);
               }
               
               // add formatting if appropriate
               if(rval && !mCompact)
               {
                  rval = os->write("\n", 1);
               }
            }
            
            // end array serialization
            rval = writeIndentation(os, level) && os->write("]", 1);
            break;
         }
      }
   }
   
   return rval;
}

bool JsonWriter::write(DynamicObject& dyno, OutputStream* os)
{
   bool rval = true;
   
   if(mStrict)
   {
      rval = !dyno.isNull();
      
      DynamicObjectType type;
      if(rval)
      {
         type = dyno->getType();
      }
   
      if(!rval || !(type == Map || type == Array))
      {
         ExceptionRef e = new Exception(
            "No JSON top-level Map or Array found.",
            "db.data.json.JsonWriter.InvalidJson");
         Exception::setLast(e, false);
         rval = false;
      }
   }
   
   if(rval)
   {
      ByteBuffer b(1024);
      BufferedOutputStream bos(&b, os);
      rval = write(dyno, &bos, mIndentLevel) && bos.flush();
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

bool JsonWriter::writeToOStream(
   DynamicObject& dyno, ostream& stream, bool compact, bool strict)
{
   OStreamOutputStream os(&stream);
   JsonWriter jw(strict);
   jw.setCompact(compact);
   if(!compact)
   {
      jw.setIndentation(0, 3);
   }
   return jw.write(dyno, &os);
}

std::string JsonWriter::writeToString(
   DynamicObject& dyno, bool compact, bool strict)
{
   string rval;
   
   ostringstream oss;
   if(writeToOStream(dyno, oss, compact, strict))
   {
      rval = oss.str();
   }
   
   return rval;
}

bool JsonWriter::writeToStdOut(
   DynamicObject& dyno, bool compact, bool strict)
{
   bool rval;
   if((rval = writeToOStream(dyno, cout, compact, strict)))
   {
      cout << endl;
      cout.flush();
   }
   return rval;
}
