/*
 * Copyright (c) 2007-2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/json/JsonWriter.h"

#include "db/rt/DynamicObjectIterator.h"
#include "db/io/BufferedOutputStream.h"
#include "db/io/OStreamOutputStream.h"

#include <iostream>
#include <sstream>

using namespace std;
using namespace db::data;
using namespace db::data::json;
using namespace db::io;
using namespace db::rt;

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
               
               // UTF-8 has a maximum of 7-bytes per character when
               // encoded in json format
               char encoded[length * 7 + 2];
               encoded[0] = '"';
               size_t n = 1;
               for(size_t i = 0; i < length; i++)
               {
                  unsigned char c = temp[i];
                  if((c >= 0x5d /* && c <= 0x10FFFF */) ||
                     (c >= 0x23 && c <= 0x5B) ||
                     (c == 0x21) ||
                     (c == 0x20))
                  {
                     // TODO: check this handles UTF-* properly
                     encoded[n++] = c;
                  }
                  else
                  {
                     encoded[n++] = '\\';
                     switch(c)
                     {
                        case '"': /* 0x22 */
                        case '\\': /* 0x5C */
                        // '/' is in the RFC but not required to be escaped
                        //case '/': /* 0x2F */
                           encoded[n++] = c;
                           break;
                        case '\b': /* 0x08 */
                           encoded[n++] = 'b';
                           break;
                        case '\f': /* 0x0C */
                           encoded[n++] = 'f';
                           break;
                        case '\n': /* 0x0A */
                           encoded[n++] = 'n';
                           break;
                        case '\r': /* 0x0D */
                           encoded[n++] = 'r';
                           break;
                        case '\t': /* 0x09 */
                           encoded[n++] = 't';
                           break;
                        default:
                           snprintf(encoded + n, 6, "u%04x", c);
                           n += 5;
                           break;
                     }
                  }
               }
               
               // end string serialization and write encoded string
               encoded[n++] = '"';
               rval = os->write(encoded, n);
            }
            break;
         case Boolean:
         case Int32:
         case UInt32:
         case Int64:
         case UInt64:
         case Double:
            {
               const char* temp = dyno->getString();
               rval = os->write(temp, strlen(temp));
            }
            break;
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
            }
            break;
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
            }
            break;
      }
   }
   
   return rval;
}

bool JsonWriter::write(DynamicObject& dyno, OutputStream* os)
{
   bool rval = true;
   DynamicObjectType type = dyno->getType();
   
   if(!(type == Map || type == Array))
   {
      ExceptionRef e = new IOException(
         "No JSON top-level Map or Array found");
      Exception::setLast(e, false);
      rval = false;
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

bool JsonWriter::writeDynamicObjectToStream(
   DynamicObject& dyno, ostream& stream, bool compact)
{
   OStreamOutputStream os(&stream);
   JsonWriter jw;
   jw.setCompact(compact);
   if(!compact)
   {
      jw.setIndentation(0, 3);
   }
   return jw.write(dyno, &os);
}

std::string JsonWriter::writeDynamicObjectToString(
   DynamicObject& dyno, bool compact)
{
   string rval;
   
   ostringstream oss;
   if(writeDynamicObjectToStream(dyno, oss, compact))
   {
      rval = oss.str();
   }
   
   return rval;
}

bool JsonWriter::writeDynamicObjectToStdOut(DynamicObject& dyno, bool compact)
{
   bool rval = writeDynamicObjectToStream(dyno, cout, compact);
   cout << endl;
   cout.flush();
   return rval;
}
