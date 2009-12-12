/*
 * Copyright (c) 2007-2009 Digital Bazaar, Inc. All rights reserved.
 */
#include "monarch/data/xml/XmlWriter.h"

#include "monarch/rt/DynamicObjectIterator.h"
#include "monarch/io/BufferedOutputStream.h"

#include <cstdio>

using namespace std;
using namespace db::data;
using namespace db::data::xml;
using namespace db::io;
using namespace db::rt;

XmlWriter::XmlWriter()
{
   // Initialize to compact representation
   setCompact(true);
   setIndentation(0, 3);
}

XmlWriter::~XmlWriter()
{
}

string XmlWriter::encode(const char* data)
{
   string rval;

   for(int i = 0; data[i] != 0; i++)
   {
      switch(data[i])
      {
         case '<':
            rval.append("&lt;");
            break;
         case '>':
            rval.append("&gt;");
            break;
         case '&':
            rval.append("&amp;");
            break;
         case '\'':
            rval.append("&apos;");
            break;
         case '"':
            rval.append("&quot;");
            break;
         default:
            rval.push_back(data[i]);
            break;
      }
   }

   return rval;
}

bool XmlWriter::writeIndentation(OutputStream* os, int level)
{
   bool rval = true;

   // write out indentation
   int indent = mCompact ? 0 : (level * mIndentSpaces);
   if(indent > 0)
   {
      char temp[indent];
      memset(temp, ' ', indent);
      rval = os->write(temp, indent);
   }

   return rval;
}

bool XmlWriter::write(DynamicObject& dyno, OutputStream* os, int level)
{
   bool rval = true;

   if(level < 0)
   {
      level = mIndentLevel;
   }

   if(dyno.isNull())
   {
      rval = writeIndentation(os, level + 1) && os->write("<null/>", 7);
   }
   else
   {
      // get element tag name
      const char* tagName = "";
      switch(dyno->getType())
      {
         case String:
            tagName = "string";
            break;
         case Boolean:
            tagName = "boolean";
            break;
         case Int32:
         case UInt32:
         case Int64:
         case UInt64:
         case Double:
            tagName = "number";
            break;
         case Map:
            tagName = "object";
            break;
         case Array:
            tagName = "array";
            break;
      }

      // write opening tag, if element is empty, use compact element
      int tagLength = strlen(tagName);
      rval =
         ((mCompact || level == 0) ? true : os->write("\n", 1)) &&
         writeIndentation(os, level) &&
         os->write("<", 1) &&
         os->write(tagName, tagLength) &&
         ((dyno->length() == 0) ? os->write("/>", 2) : os->write(">", 1));

      // write element data/contents
      if(rval && dyno->length() > 0)
      {
         switch(dyno->getType())
         {
            case String:
            {
               // xml-encode string
               string encoded = encode(dyno->getString());
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
               // serialize number to string
               const char* temp = dyno->getString();
               rval = os->write(temp, strlen(temp));
               break;
            }
            case Map:
            {
               // serialize each map member
               DynamicObjectIterator i = dyno.getIterator();
               while(rval && i->hasNext())
               {
                  DynamicObject next = i->next();

                  // serialize member name and value
                  rval =
                     (mCompact ? true : os->write("\n", 1)) &&
                     writeIndentation(os, level + 1) &&
                     os->write("<member name=\"", 14) &&
                     os->write(i->getName(), strlen(i->getName())) &&
                     os->write("\">", 2) &&
                     write(next, os, level + 2) &&
                     (mCompact ? true : os->write("\n", 1)) &&
                     writeIndentation(os, level + 1) &&
                     os->write("</member>", 9);
               }
               break;
            }
            case Array:
            {
               // serialize each array element
               char temp[22];
               DynamicObjectIterator i = dyno.getIterator();
               while(rval && i->hasNext())
               {
                  DynamicObject next = i->next();

                  // serialize element index and value
                  sprintf(temp, "%i", i->getIndex());
                  rval =
                     (mCompact ? true : os->write("\n", 1)) &&
                     writeIndentation(os, level + 1) &&
                     os->write("<element index=\"", 16) &&
                     os->write(temp, strlen(temp)) &&
                     os->write("\">", 2) &&
                     write(next, os, level + 2) &&
                     (mCompact ? true : os->write("\n", 1)) &&
                     writeIndentation(os, level + 1) &&
                     os->write("</element>", 10);
               }
               break;
            }
         }

         // write end element
         // (only write indentation for map or array)
         if(dyno->getType() == Map || dyno->getType() == Array)
         {
            rval = rval &&
               (mCompact ? true : os->write("\n", 1)) &&
               writeIndentation(os, level);
         }

         rval =
            rval &&
            os->write("</", 2) &&
            os->write(tagName, tagLength) &&
            os->write(">", 1);
      }
   }

   return rval;
}

bool XmlWriter::write(DynamicObject& dyno, OutputStream* os)
{
   bool rval;

   ByteBuffer b(1024);
   BufferedOutputStream bos(&b, os);
   rval = write(dyno, &bos, mIndentLevel) && bos.flush();

   return rval;
}

void XmlWriter::setCompact(bool compact)
{
   mCompact = compact;
}

void XmlWriter::setIndentation(int level, int spaces)
{
   mIndentLevel = level;
   mIndentSpaces = spaces;
}
