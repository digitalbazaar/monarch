/*
 * Copyright (c) 2008 Digital Bazaar, Inc.  All rights reserved.
 */
#include "db/data/xml/DomWriter.h"

#include "db/rt/DynamicObjectIterator.h"
#include "db/io/BufferedOutputStream.h"

using namespace std;
using namespace db::data;
using namespace db::data::xml;
using namespace db::io;
using namespace db::rt;

DomWriter::DomWriter()
{
   // Initialize to compact representation
   setCompact(true);
   setIndentation(0, 3);
}

DomWriter::~DomWriter()
{
}

bool DomWriter::write(Element& e, OutputStream* os, int level)
{
   bool rval = true;
   
   if(level < 0)
   {
      level = mIndentLevel;
   }
   
   // open start element
   rval =
      ((mCompact || level == 0) ? true : os->write("\n", 1)) &&
      writeIndentation(os, level) &&
      os->write("<", 1) &&
      os->write(e["name"]->getString(), e["name"]->length());
   
   // write attributes
   AttributeIterator attrs = e["attributes"].getIterator();
   while(rval && attrs->hasNext()) 
   {
      DynamicObject& attr = attrs->next();
      rval =
         os->write(" ", 1) &&
         os->write(attrs->getName(), strlen(attrs->getName())) &&
         os->write("=\"", 2) &&
         os->write(attr->getString(), attr->length()) &&
         os->write("\"", 1);
   }
   
   // close start element, if element is empty, use compact element
   int dataLength = e["data"]->length();
   bool empty = (dataLength == 0 && e["children"]->length() == 0);
   rval = rval && (empty ? os->write("/>", 2) : os->write(">", 1));   
   
   // write element data and children
   if(rval && !empty)
   {
      // write element children
      if(rval && e["children"]->length() > 0)
      {
         // serialize each child
         DynamicObjectIterator lists = e["children"].getIterator();
         while(rval && lists->hasNext())
         {
            DynamicObject& list = lists->next();
            ElementIterator children = list.getIterator();
            while(rval && children->hasNext())
            {
               // serialize child
               Element& child = children->next();
               rval = write(child, os, level + 1);
            }
         }
      }
      
      // write element data
      if(dataLength > 0)
      {
         // xml-encode data
         string encoded = encode(e["data"]->getString());
         rval = os->write(encoded.c_str(), encoded.length());
      }
      
      // write end element
      // (only write indentation when data is blank and children are present)
      if(dataLength == 0 && !empty)
      {
         rval = rval &&
            (mCompact ? true : os->write("\n", 1)) &&
            writeIndentation(os, level);
      }
      
      rval = rval &&
         os->write("</", 2) &&
         os->write(e["name"]->getString(), e["name"]->length()) &&
         os->write(">", 1);
   }
   
   return rval;
}

bool DomWriter::write(DynamicObject& root, OutputStream* os)
{
   return XmlWriter::write(root, os);
}
