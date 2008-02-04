/*
 * Copyright (c) 2007 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/json/JsonWriter.h"
#include "db/util/DynamicObjectIterator.h"
#include "db/io/BufferedOutputStream.h"

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
               string encoded;
               encoded.push_back('"');
               const char* temp = dyno->getString();
               size_t length = strlen(temp);
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
                           snprintf(ucsub, 7, "\\u%04x", c);
                           csub = ucsub;
                           break;
                     }
                     encoded.append(csub);
                  }
               }
               
               // write encoded string and end string serialization
               encoded.push_back('"');
               rval = os->write(encoded.c_str(), encoded.length());
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
                  DynamicObject next = i->next();
                  
                  // serialize indentation and start serializing member name
                  if(rval =
                     writeIndentation(os, level + 1) &&
                     os->write("\"", 1) &&
                     os->write(i->getName(), strlen(i->getName())))
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
   bool rval;
   
   ByteBuffer b(1024);
   BufferedOutputStream bos(&b, os);
   rval = write(dyno, &bos, mIndentLevel) && bos.flush();
   
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
